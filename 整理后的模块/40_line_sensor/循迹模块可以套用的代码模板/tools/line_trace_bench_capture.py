#!/usr/bin/env python3
"""Capture E-003 line-sensor bench snapshots from an MSPM0G3507 ELF/SWD target."""

from __future__ import annotations

import argparse
import csv
import json
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path


SNAPSHOT_SYMBOL = "g_line_bench_snapshot"
SNAPSHOT_WORDS = 22
SNAPSHOT_FIELDS = [
    ("magic", "u32"),
    ("version", "u32"),
    ("size_words", "u32"),
    ("loop_counter", "u32"),
    ("raw_bits", "hex8"),
    ("active_bits", "hex8"),
    ("active_count", "u32"),
    ("position", "i32"),
    ("error", "i32"),
    ("confidence", "u32"),
    ("detected", "u32"),
    ("pattern", "u32"),
    ("lost_dir", "i32"),
    ("target_left", "i32"),
    ("target_right", "i32"),
    ("correction", "i32"),
    ("searching", "u32"),
    ("threshold_status", "u32"),
    ("sensor_status", "u32"),
    ("tuning_status", "u32"),
    ("error_code", "u32"),
    ("safety_state", "u32"),
]


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def find_tool(name: str) -> str:
    resolved = shutil.which(name)
    if resolved is None:
        raise SystemExit(f"tool not found: {name}")
    return resolved


def resolve_symbol(nm: str, elf: Path, name: str) -> int:
    if not elf.exists():
        raise SystemExit(f"ELF not found: {elf}")

    proc = subprocess.run(
        [nm, "-g", "--defined-only", str(elf)],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[2] == name:
            return int(parts[0], 16)
    raise SystemExit(f"missing symbol in {elf}: {name}")


def build_pyocd_command(args: argparse.Namespace, addr: int) -> list[str]:
    if (addr % 4) != 0:
        raise SystemExit(f"{SNAPSHOT_SYMBOL} is not 4-byte aligned: 0x{addr:08x}")

    cmd = [args.pyocd, "commander", "--no-config"]
    if args.pack:
        cmd.extend(["--pack", args.pack])
    if args.probe_uid:
        cmd.extend(["-u", args.probe_uid])
    cmd.extend(["-t", args.target, "-f", str(args.frequency)])
    cmd.extend(["-c", "status"])
    cmd.extend(["-c", f"read32 0x{addr:08x} {SNAPSHOT_WORDS}"])
    cmd.extend(["-c", "exit"])
    return cmd


def as_i32(value: int) -> int:
    return value - 0x100000000 if (value & 0x80000000) else value


def decode_words(words: list[int]) -> dict[str, object]:
    if len(words) < SNAPSHOT_WORDS:
        raise ValueError(f"need {SNAPSHOT_WORDS} words, got {len(words)}")

    decoded: dict[str, object] = {}
    for index, (name, kind) in enumerate(SNAPSHOT_FIELDS):
        value = words[index]
        if kind == "i32":
            decoded[name] = as_i32(value)
        elif kind == "hex8":
            decoded[name] = f"0x{value & 0xff:02x}"
        else:
            decoded[name] = value
    return decoded


def parse_read32_words(output: str) -> list[int]:
    words: list[int] = []
    for line in output.splitlines():
        if not re.match(r"^\s*[0-9a-fA-F]{8}:", line):
            continue
        _, rest = line.split(":", 1)
        for token in re.findall(r"\b[0-9a-fA-F]{8}\b", rest):
            words.append(int(token, 16))
    return words


def write_csv(path: Path, samples: list[dict[str, object]]) -> None:
    if not samples:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    keys = list(samples[0].keys())
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=keys)
        writer.writeheader()
        writer.writerows(samples)


def main() -> int:
    root = repo_root()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--elf",
        default=str(root / "build/mspm0g3507-line-trace-smoke/line_trace_smoke.elf"),
        help="fresh MSPM0G3507 ELF to inspect",
    )
    parser.add_argument("--nm", default="arm-none-eabi-nm", help="nm executable")
    parser.add_argument("--pyocd", default="pyocd", help="pyOCD executable")
    parser.add_argument("--target", default="mspm0g3507", help="pyOCD target")
    parser.add_argument("--frequency", type=int, default=500000, help="SWD frequency")
    parser.add_argument("--probe-uid", default="", help="required with --run")
    parser.add_argument("--pack", default="", help="optional TI DFP pack path")
    parser.add_argument("--case", default="bench_sample", help="sample label")
    parser.add_argument("--samples", type=int, default=1, help="number of samples")
    parser.add_argument("--interval-ms", type=int, default=200, help="delay between samples")
    parser.add_argument("--out-json", default="", help="write decoded report JSON")
    parser.add_argument("--out-csv", default="", help="write decoded samples CSV")
    parser.add_argument("--run", action="store_true", help="actually invoke pyOCD")
    args = parser.parse_args()

    if args.samples <= 0:
        raise SystemExit("--samples must be positive")

    args.elf = str(Path(args.elf).resolve())
    args.nm = find_tool(args.nm)
    if args.run:
        args.pyocd = find_tool(args.pyocd)
        if not args.probe_uid:
            raise SystemExit("--probe-uid is required with --run")

    addr = resolve_symbol(args.nm, Path(args.elf), SNAPSHOT_SYMBOL)
    pyocd_command = build_pyocd_command(args, addr)
    report: dict[str, object] = {
        "mode": "run" if args.run else "dry-run",
        "case": args.case,
        "elf": args.elf,
        "symbol": SNAPSHOT_SYMBOL,
        "address": f"0x{addr:08x}",
        "word_count": SNAPSHOT_WORDS,
        "pyocd_command": pyocd_command,
        "samples": [],
    }

    if not args.run:
        print(json.dumps(report, indent=2, ensure_ascii=False))
        return 0

    decoded_samples: list[dict[str, object]] = []
    for index in range(args.samples):
        proc = subprocess.run(
            pyocd_command,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        words = parse_read32_words(proc.stdout)
        sample = decode_words(words)
        sample["case"] = args.case
        sample["sample_index"] = index
        sample["pyocd_exit_code"] = proc.returncode
        decoded_samples.append(sample)
        if proc.returncode != 0:
            break
        if index + 1 < args.samples:
            time.sleep(args.interval_ms / 1000.0)

    report["samples"] = decoded_samples
    if args.out_json:
        out_json = Path(args.out_json)
        out_json.parent.mkdir(parents=True, exist_ok=True)
        out_json.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")
    if args.out_csv:
        write_csv(Path(args.out_csv), decoded_samples)

    print(json.dumps(report, indent=2, ensure_ascii=False))
    return 0 if all(sample["pyocd_exit_code"] == 0 for sample in decoded_samples) else 1


if __name__ == "__main__":
    sys.exit(main())
