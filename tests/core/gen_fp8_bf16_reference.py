#!/usr/bin/env python3
"""Generate tests/core/fp8_bf16_reference.inc, the oracle for FP8 E4M3FN / BF16 conversion.

Independent of any repository code. The formats are implemented from their specifications:

* FP8 E4M3FN: OCP 8-bit Floating Point Specification (OFP8) rev 1.0, E4M3 -- 1 sign, 4 exponent
  (bias 7), 3 mantissa bits; no infinities; only S.1111.111 is NaN; max finite 448;
  subnormals m * 2^-9.  The float -> FP8 direction is "round to nearest even, saturate to
  finite" (CUDA __NV_SATFINITE): +-inf and any magnitude above 448 give +-448, NaN gives NaN.
* BF16: IEEE-754 binary32 with the low 16 mantissa bits dropped, round to nearest even; the
  exponent range is that of binary32, so overflow rounds to +-inf.

Rounding is done exactly: every representable magnitude is tabulated as a Fraction and the
nearest entry is picked by exact comparison, ties going to the even code.  The BF16 table also
carries the "virtual" next value 2^128 (code 0x7F80, inf) so overflow follows IEEE rounding.

Usage:  python gen_fp8_bf16_reference.py          (re)write the .inc
        python gen_fp8_bf16_reference.py --check  self-checks + compare with the committed .inc
"""
import argparse
import bisect
import sys
from fractions import Fraction
from pathlib import Path

INC = Path(__file__).with_name("fp8_bf16_reference.inc")
F32_NAN_Q = 0x7FC00000


# ----------------------------------------------------------------------------- binary32
def f32_decode(bits):
    """-> (kind, sign, magnitude) with kind in {'nan','inf','fin'}; magnitude is an exact Fraction."""
    sign, exp, frac = bits >> 31, (bits >> 23) & 0xFF, bits & 0x7FFFFF
    if exp == 0xFF:
        return ("inf" if frac == 0 else "nan"), sign, None
    if exp == 0:
        return "fin", sign, Fraction(frac) * Fraction(1, 2**149)
    return "fin", sign, Fraction(2**23 + frac) * (Fraction(2) ** (exp - 150))


def f32_encode_exact(sign, mag):
    """Bits of the binary32 whose value is exactly +-mag (mag must be representable)."""
    if mag == 0:
        return sign << 31
    e = mag.numerator.bit_length() - mag.denominator.bit_length()   # floor(log2 mag) or +1
    if Fraction(2) ** e > mag:
        e -= 1
    if e < -126:                                                    # subnormal
        frac = mag * 2**149
        assert frac.denominator == 1 and frac < 2**23, mag
        return sign << 31 | int(frac)
    sig = mag / Fraction(2) ** (e - 23)
    assert sig.denominator == 1 and 2**23 <= sig < 2**24 and e <= 127, mag
    return sign << 31 | (e + 127) << 23 | (int(sig) - 2**23)


# --------------------------------------------------------------------------------- E4M3FN
def e4m3fn_decode(code):
    """-> (sign, magnitude Fraction) or (sign, None) for NaN."""
    sign, exp, man = code >> 7, (code >> 3) & 0xF, code & 7
    if exp == 0xF and man == 7:
        return sign, None
    if exp == 0:
        return sign, Fraction(man, 8) * Fraction(1, 2**6)
    return sign, (1 + Fraction(man, 8)) * Fraction(2) ** (exp - 7)


E4M3_VALUES = [e4m3fn_decode(c)[1] for c in range(0x7F)]    # finite magnitudes, codes 0x00..0x7E
assert E4M3_VALUES[0x7E] == 448


def nearest_even_code(mag, values):
    """Index of the value nearest to mag; exact ties go to the even index (= even code LSB)."""
    i = bisect.bisect_left(values, mag)
    if i == 0:
        return 0
    if i == len(values):
        return len(values) - 1
    lo, hi = values[i - 1], values[i]
    if mag - lo < hi - mag:
        return i - 1
    if mag - lo > hi - mag:
        return i
    return i - 1 if (i - 1) % 2 == 0 else i


def fp8_encode_satfinite(bits):
    """-> (code, is_nan)."""
    kind, sign, mag = f32_decode(bits)
    if kind == "nan":
        return 0x7F | sign << 7, True
    if kind == "inf" or mag > 448:
        mag = Fraction(448)
    return sign << 7 | nearest_even_code(mag, E4M3_VALUES), False


# ------------------------------------------------------------------------------------ BF16
def bf16_decode(code):
    """-> (sign, magnitude Fraction | None for NaN | 'inf')."""
    sign, exp, man = code >> 15, (code >> 7) & 0xFF, code & 0x7F
    if exp == 0xFF:
        return sign, ("inf" if man == 0 else None)
    if exp == 0:
        return sign, Fraction(man) * Fraction(1, 2**133)
    return sign, Fraction(128 + man) * Fraction(2) ** (exp - 134)


# Magnitudes of codes 0x0000..0x7F7F plus the virtual next value 2^128 as index 0x7F80 (inf).
BF16_VALUES = [bf16_decode(c)[1] for c in range(0x7F80)] + [Fraction(2) ** 128]


def bf16_encode_rne(bits):
    """-> (code, is_nan)."""
    kind, sign, mag = f32_decode(bits)
    if kind == "nan":
        return 0x7FC0 | sign << 15, True
    if kind == "inf":
        return sign << 15 | 0x7F80, False
    return sign << 15 | nearest_even_code(mag, BF16_VALUES), False


def bf16_encode_bits_trick(bits):
    """Second, independent method (integer RNE on the dropped half-word) used only in self-checks."""
    if (bits & 0x7FFFFFFF) > 0x7F800000:
        return None
    hi, lo = bits >> 16, bits & 0xFFFF
    return hi + (1 if lo > 0x8000 or (lo == 0x8000 and hi & 1) else 0)


# ------------------------------------------------------------------------------ case lists
def xorshift(seed):
    x = seed
    while True:
        x ^= (x << 13) & (2**64 - 1)
        x ^= x >> 7
        x ^= (x << 17) & (2**64 - 1)
        yield x


def dedupe(seq):
    return list(dict.fromkeys(seq))


def fp8_decode_cases():
    out = []
    for code in range(256):
        sign, mag = e4m3fn_decode(code)
        out.append((code, F32_NAN_Q | sign << 31, True) if mag is None
                   else (code, f32_encode_exact(sign, mag), False))
    return out


def fp8_encode_inputs():
    """-> list of (group, input float32 bits)."""
    groups = []
    exact, ties, tiny, big, nans, rnd = [], [], [], [], [], []
    for sign in (0, 1):
        s = sign << 31
        for c in range(0x7F):
            exact.append(f32_encode_exact(sign, E4M3_VALUES[c]))
        for c in range(0x7E):
            mid = f32_encode_exact(sign, (E4M3_VALUES[c] + E4M3_VALUES[c + 1]) / 2)
            ties += [mid, mid - 1, mid + 1]          # tie, just below, just above in magnitude
        t = f32_encode_exact(0, Fraction(1, 2**10))   # tie between 0 and the smallest subnormal
        tiny += [s | 0, s | 1, s | 0x007FFFFF, s | 0x00800000, s | t - 1, s | t, s | t + 1,
                 s | f32_encode_exact(0, Fraction(1, 2**11)),
                 s | f32_encode_exact(0, Fraction(1, 2**20))]
        for v in (448, 464, 480, 512, 1000, 2**33):
            b = f32_encode_exact(0, Fraction(v))
            big += [s | b - 1, s | b, s | b + 1]     # 448 and 464 are the interesting seams
        big += [s | 0x7F7FFFFF, s | 0x7F800000]      # FLT_MAX, inf
        nans += [s | 0x7FC00000, s | 0x7F800001, s | 0x7FFFFFFF]
    gen = xorshift(0x9E3779B97F4A7C15)
    for _ in range(2000):
        r = next(gen)
        exp = 127 - 12 + (r >> 8) % 23               # |x| in [2^-12, 2^11)
        rnd.append((r & 1) << 31 | exp << 23 | (r >> 20) & 0x7FFFFF)
    for name, lst in (("exact", exact), ("tie", ties), ("underflow", tiny), ("overflow", big),
                      ("nan", nans), ("random", rnd)):
        groups += [(name, b) for b in dedupe(lst)]
    return groups


def bf16_encode_inputs():
    exact_tie, special, rnd = [], [], []
    for sign in (0, 1):
        for exp in range(255):                       # finite exponents, subnormal..max
            for man in (0, 1, 0x7F):                 # even, odd, odd + carry into the exponent
                hi = sign << 15 | exp << 7 | man
                base = hi << 16
                exact_tie += [base, base | 0x8000, base | 0x7FFF, base | 0x8001]
        s = sign << 31
        special += [s, s | 0x7F800000, s | 0x7F7FFFFF, s | 0x7F7F8000, s | 0x7F7F7FFF,
                    s | 0x00000001, s | 0x007FFFFF, s | 0x00008000, s | 0x00018000,
                    s | 0x7FC00000, s | 0x7F800001, s | 0x7F807FFF, s | 0x7F808000, s | 0x7FFFFFFF]
    gen = xorshift(0xD1B54A32D192ED03)
    for _ in range(1500):
        rnd.append((next(gen) >> 16) & 0xFFFFFFFF)
    return ([("tie", b) for b in dedupe(exact_tie)] + [("special", b) for b in dedupe(special)]
            + [("random", b) for b in dedupe(rnd)])


def bf16_decode_cases():
    out = []
    for sign in (0, 1):
        for exp in range(256):
            for man in (0, 1, 2, 0x2A, 0x40, 0x55, 0x7E, 0x7F):
                code = sign << 15 | exp << 7 | man
                s, mag = bf16_decode(code)
                if mag is None:
                    out.append((code, F32_NAN_Q | s << 31, True))
                elif mag == "inf":
                    out.append((code, s << 31 | 0x7F800000, False))
                else:
                    out.append((code, f32_encode_exact(s, mag), False))
    return out


# --------------------------------------------------------------------------------- output
def render():
    fp8_dec = fp8_decode_cases()
    fp8_enc = [(g, b) + fp8_encode_satfinite(b) for g, b in fp8_encode_inputs()]
    bf_enc = [(g, b) + bf16_encode_rne(b) for g, b in bf16_encode_inputs()]
    bf_dec = bf16_decode_cases()

    def counts(rows):
        c = {}
        for r in rows:
            c[r[0]] = c.get(r[0], 0) + 1
        return ", ".join(f"{k}={v}" for k, v in c.items())

    L = ["// GENERATED by tests/core/gen_fp8_bf16_reference.py -- do not edit.",
         "// Regenerate: python tests/core/gen_fp8_bf16_reference.py ; verify: ... --check",
         "// Reference: OCP OFP8 rev 1.0 (E4M3) and IEEE-754 binary32 -> bfloat16 RNE; see the script.",
         "// Each section is enabled by defining its NINFER_REF_* macro before including this file.",
         "// Rows ending in `true` are NaN results/inputs: any NaN of the right type is accepted.",
         f"// counts: FP8_DECODE={len(fp8_dec)}, FP8_ENCODE={len(fp8_enc)} ({counts(fp8_enc)}),",
         f"//         BF16_ENCODE={len(bf_enc)} ({counts(bf_enc)}), BF16_DECODE={len(bf_dec)}",
         "", "#ifdef NINFER_REF_FP8_DECODE", "// {fp8 code, float bits, is_nan}"]
    L += [f"{{0x{c:02X}, 0x{b:08X}u, {str(n).lower()}}}," for c, b, n in fp8_dec]
    L += ["#endif", "", "#ifdef NINFER_REF_FP8_ENCODE_SATFINITE",
          "// {float bits in, fp8 code expected, is_nan}   RNE, saturate to +-448"]
    L += [f"{{0x{b:08X}u, 0x{c:02X}, {str(n).lower()}}},  // {g}" for g, b, c, n in fp8_enc]
    L += ["#endif", "", "#ifdef NINFER_REF_BF16_ENCODE_RNE",
          "// {float bits in, bf16 bits expected, is_nan}   RNE, overflow -> inf"]
    L += [f"{{0x{b:08X}u, 0x{c:04X}, {str(n).lower()}}},  // {g}" for g, b, c, n in bf_enc]
    L += ["#endif", "", "#ifdef NINFER_REF_BF16_DECODE",
          "// {bf16 bits, float bits expected, is_nan}"]
    L += [f"{{0x{c:04X}, 0x{b:08X}u, {str(n).lower()}}}," for c, b, n in bf_dec]
    L += ["#endif", ""]
    return "\n".join(L), fp8_dec, fp8_enc, bf_enc, bf_dec


def selfcheck(fp8_dec, fp8_enc, bf_enc, bf_dec):
    assert len(fp8_dec) == 256
    assert sum(1 for r in fp8_dec if r[2]) == 2 and {r[0] for r in fp8_dec if r[2]} == {0x7F, 0xFF}
    bits = {c: b for c, b, _ in fp8_dec}
    one = lambda v: f32_encode_exact(0, Fraction(v))
    assert bits[0x00] == 0 and bits[0x80] == 0x80000000
    assert bits[0x01] == one(Fraction(1, 2**9)) and bits[0x08] == one(Fraction(1, 2**6))
    assert bits[0x7E] == one(448) and bits[0xFE] == one(448) | 0x80000000 and bits[0x38] == one(1)
    assert max(f32_decode(b)[2] for c, b, n in fp8_dec if not n) == 448
    for c, b, n in fp8_dec:                                   # decode/encode round trip
        if not n:
            assert fp8_encode_satfinite(b) == (c, False), hex(c)
    for g, b, c, n in fp8_enc:                                # saturation, sign and monotonic sanity
        k, s, m = f32_decode(b)
        if n:
            assert k == "nan"
        elif k == "inf" or m > 448:
            assert c & 0x7F == 0x7E and c >> 7 == s
        assert c >> 7 == s or n
    for g, b, c, n in bf_enc:
        t = bf16_encode_bits_trick(b)
        assert (t is None) == n and (n or t == c), (hex(b), hex(c), t)
    for c, b, n in bf_dec:
        assert n or b == c << 16, hex(c)                      # bf16 -> float is a pure shift
    assert len(bf_dec) == 4096 and len({r[0] for r in bf_dec}) == 4096
    return True


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--check", action="store_true",
                    help="run self-checks and compare with the committed .inc instead of writing")
    args = ap.parse_args()
    text, *tables = render()
    selfcheck(*tables)
    if args.check:
        have = INC.read_text(encoding="utf-8").replace("\r\n", "\n") if INC.exists() else None
        if have != text:
            print(f"DIFFERS: {INC} does not match the regenerated tables", file=sys.stderr)
            return 1
        print(f"OK: self-checks passed, {INC.name} matches (0 differences)")
        return 0
    INC.write_text(text, encoding="utf-8", newline="\n")
    print(f"wrote {INC} ({len(text.splitlines())} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
