"""Opt-in Windows >4 GiB queue check; no model or GPU, standard library only."""

import argparse
import hashlib
import json
from pathlib import Path
import random
import re
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reader", type=Path, required=True)
    parser.add_argument("--directory", type=Path,
                        default=Path(tempfile.gettempdir()) / "ninfer-io")
    args = parser.parse_args()
    args.directory.mkdir(parents=True, exist_ok=True)
    free = shutil.disk_usage(args.directory).free
    if free < 10 * 1024**3:
        raise RuntimeError(f"need 10 GiB free in {args.directory}; available={free}")
    path = args.directory / "deterministic-4.5gib.bin"
    if path.exists():
        raise RuntimeError(f"refusing to replace existing fixture: {path}")
    length = 9 * 512 * 1024**2 + 123
    random_source = random.Random(9705)
    generated = hashlib.sha256()
    try:
        with path.open("xb") as output:
            remaining = length
            while remaining:
                block = random_source.randbytes(min(4 * 1024**2, remaining))
                output.write(block)
                generated.update(block)
                remaining -= len(block)
        # Reopen and reread with Python buffered I/O: independent of the production queue
        # and of the generation-time hash. BCrypt is used on the C++ side.
        reference = hashlib.sha256()
        with path.open("rb") as source:
            while block := source.read(8 * 1024**2):
                reference.update(block)
        result = subprocess.run([str(args.reader.resolve()), str(path.resolve())],
                                check=True, capture_output=True, text=True, timeout=600)
        match = re.fullmatch(r"bytes=(\d+) crossed_4gib=1 sha256=([0-9a-f]{64})\s*", result.stdout)
        if not match or int(match[1]) != length:
            raise RuntimeError(f"invalid queue result: {result.stdout!r} {result.stderr!r}")
        if not generated.hexdigest() == reference.hexdigest() == match[2]:
            raise RuntimeError("SHA-256 mismatch")
        print(json.dumps({"bytes": length, "free_before": free, "seed": 9705,
                          "generated_sha256": generated.hexdigest(),
                          "reference_sha256": reference.hexdigest(), "queue_sha256": match[2],
                          "crossed_4gib": True}, indent=2), flush=True)
    finally:
        path.unlink(missing_ok=True)
        print(f"large fixture removed: {not path.exists()}", flush=True)


if __name__ == "__main__":
    main()
