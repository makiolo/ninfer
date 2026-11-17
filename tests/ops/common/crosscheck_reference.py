"""Independent cross-check of tests/ops/common/primitives_reference.h (R97-09 oracle).

Usage: crosscheck_reference.py DUMP.txt [LOG]   (DUMP from reference_dump.cpp)
Recomputes sums (exact Fractions, plus numpy for max), max (numpy nanmax, NaN ignored), argmax (lexicographic
key) and Welford moments (exact rationals, sum x^2 - (sum x)^2/n) from the dumped inputs and compares
with the C++ expectations. Standard library + numpy only. Exit status 1 on any discrepancy.
"""
import math
import struct
import sys
from fractions import Fraction

import numpy as np


def f32(h):
    return struct.unpack("<f", struct.pack("<I", int(h, 16)))[0]


def bits(f):
    return struct.unpack("<I", struct.pack("<f", f))[0]


def main(argv):
    dump, log = argv[1], (argv[2] if len(argv) > 2 else None)
    count = {"S": 0, "M": 0, "A": 0, "W": 0}
    bad, notes = [], []
    order_sensitive = 0
    u = Fraction(1, 2**24)
    for line in open(dump):
        left, right = line.split(" | ")
        tag, name, *rest = left.split()
        out = right.split()
        count[tag] += 1
        if tag in "SM":
            exact_flag = int(rest[0]) if tag == "S" else 0
            n = int(rest[1] if tag == "S" else rest[0])
            xs = [f32(h) for h in rest[2 if tag == "S" else 1:]]
            assert len(xs) == n
            if tag == "S":
                nan = any(math.isnan(x) for x in xs)
                pinf = any(x == math.inf for x in xs)
                ninf = any(x == -math.inf for x in xs)
                kind = 1 if (nan or (pinf and ninf)) else 2 if pinf else 3 if ninf else 0
                fin = [x for x in xs if math.isfinite(x)]
                ex = sum((Fraction(x) for x in fin), Fraction(0))
                absum = sum((abs(Fraction(x)) for x in fin), Fraction(0))
                nf = len(fin)
                bound = Fraction(0) if exact_flag else (nf - 1) * u / (1 - (nf - 1) * u) * absum if nf > 1 else Fraction(0)
                ck, cex, cb, cneg = int(out[0]), float.fromhex(out[1]), float.fromhex(out[2]), int(out[3])
                if ck != kind:
                    bad.append(f"{name}: sum kind C++ {ck} py {kind}")
                if abs(Fraction(cex) - ex) > abs(ex) * Fraction(1, 2**50):
                    bad.append(f"{name}: exact sum C++ {cex!r} py {float(ex)!r}")
                if abs(Fraction(cb) - bound) > bound * Fraction(1, 2**40):
                    bad.append(f"{name}: bound C++ {cb!r} py {float(bound)!r}")
                if exact_flag and Fraction(float(np.float32(float(ex)))) != ex:
                    bad.append(f"{name}: exact flag set but sum not representable in float32")
                if ck == 0 and len(xs) > 0 and cneg != int(all(bits(x) == 0x80000000 for x in xs)):
                    bad.append(f"{name}: negative-zero flag")
                if not exact_flag and kind == 0 and n > 2:
                    seq = np.float32(0)
                    for x in xs:
                        seq = np.float32(seq + np.float32(x))
                    if abs(Fraction(float(seq)) - ex) > 0:
                        order_sensitive += 1
            else:
                with np.errstate(all="ignore"):
                    arr = np.array(xs, dtype=np.float32)
                    py = np.float32(np.nan) if np.all(np.isnan(arr)) else np.nanmax(arr)
                cm = f32(out[0])
                same = (math.isnan(cm) and math.isnan(py)) or cm == float(py)
                if not same:
                    bad.append(f"{name}: max C++ {cm!r} numpy {float(py)!r}")
        elif tag == "A":
            n = int(rest[0])
            pairs = [(f32(p.split(":")[0]), int(p.split(":")[1])) for p in rest[1:]]
            assert len(pairs) == n
            best = max(range(n), key=lambda i: (pairs[i][0], -pairs[i][1]))
            if best != int(out[0]):
                bad.append(f"{name}: argmax C++ {out[0]} py {best}")
        else:
            n = int(rest[0])
            xs = [f32(h) for h in rest[1:]]
            cn, cmean, cm2, special = int(out[0]), float.fromhex(out[1]), float.fromhex(out[2]), int(out[3])
            py_special = 1 if any(math.isnan(x) for x in xs) else 2 if any(math.isinf(x) for x in xs) else 0
            if special != py_special or cn != n:
                bad.append(f"{name}: welford class/count C++ {special}/{cn} py {py_special}/{n}")
            elif special == 0:
                s1 = sum((Fraction(x) for x in xs), Fraction(0))
                s2 = sum((Fraction(x) ** 2 for x in xs), Fraction(0))
                mean = s1 / n
                m2 = s2 - s1 * s1 / n
                if abs(Fraction(cmean) - mean) > max(abs(mean), Fraction(1)) * Fraction(1, 2**40):
                    bad.append(f"{name}: mean C++ {cmean!r} py {float(mean)!r}")
                if abs(Fraction(cm2) - m2) > max(abs(m2), Fraction(1)) * Fraction(1, 2**40):
                    bad.append(f"{name}: m2 C++ {cm2!r} py {float(m2)!r}")
    lines = [
        "R97-09 crosscheck: C++ reference (primitives_reference.h) vs Python (Fraction/math.fsum/numpy)",
        f"cases: sum={count['S']} max={count['M']} argmax={count['A']} welford={count['W']}",
        f"discrepancies = {len(bad)}",
        "NaN convention: sum -> NaN if any NaN or (+Inf and -Inf); max -> maxNum (NaN ignored, all-NaN -> NaN);",
        "                argmax NaN cases are excluded here (outside the finite score/id contract, PINNED only);",
        "                welford -> NaN input gives NaN mean and m2.",
        f"order-sensitive cases (sequential float32 sum != exact): {order_sensitive}",
        "numpy " + np.__version__,
    ] + bad
    text = "\n".join(lines) + "\n"
    print(text, end="")
    if log:
        open(log, "w").write(text)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
