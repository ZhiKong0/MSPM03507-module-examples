#!/usr/bin/env python3
"""Resolve line-trace SWD symbols and optionally read them with pyOCD."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path


DEFAULT_SYMBOLS = [
    "g_line_tuning_block",
    "g_line_telemetry",
    "g_line_smoke_counter",
    "g_line_smoke_cookie",
]


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def find_tool(name: str) -> str:
    resolved = shutil.which(name)
    if resolved is None:
        raise SystemExit(f"tool not found: {name}")
    return resolved


def resolve_symbols(nm: str, elf: Path, names: list[str]) -> dict[str, int]:
    if not elf.exists():
        raise SystemExit(f"ELF not found: {elf}")

    proc = subprocess.run(
        [nm, "-g", "--defined-only", str(elf)],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    wanted = set(names)
    found: dict[str, int] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) < 3:
            continue
        addr_text, _, name = parts[:3]
        if name in wanted:
            found[name] = int(addr_text, 16)

    missing = sorted(wanted - set(found))
    if missing:
        raise SystemExit(f"missing symbols in {elf}: {', '.join(missing)}")
    return found


def build_pyocd_command(args: argparse.Namespace, symbols: dict[str, int]) -> list[str]:
    cmd = [args.pyocd, "commander", "--no-config"]
    if args.pack:
        cmd.extend(["--pack", args.pack])
    if args.probe_uid:
        cmd.extend(["-u", args.probe_uid])
    cmd.extend(["-t", args.target, "-f", str(args.frequency)])
    cmd.extend(["-c", "status"])
    for name in args.symbol:
        addr = symbols[name]
        if (addr % 4) != 0:
            raise SystemExit(f"symbol is not 4-byte aligned: {name}=0x{addr:08x}")
        cmd.extend(["-c", f"read32 0x{addr:08x} 4"])
    cmd.extend(["-c", "exit"])
    return cmd


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
    parser.add_argument("--target", default="mspm0g3507", help="pyOCD target name")
    parser.add_argument("--frequency", type=int, default=500000, help="SWD frequency")
    parser.add_argument("--probe-uid", default="", help="required for --run on shared benches")
    parser.add_argument("--pack", default="", help="optional TI DFP pack path")
    parser.add_argument("--symbol", action="append", default=list(DEFAULT_SYMBOLS))
    parser.add_argument("--run", action="store_true", help="actually invoke pyOCD readback")
    args = parser.parse_args()

    args.elf = str(Path(args.elf).resolve())
    args.nm = find_tool(args.nm)
    if args.run:
        args.pyocd = find_tool(args.pyocd)
        if not args.probe_uid:
            raise SystemExit("--probe-uid is required with --run")

    symbols = resolve_symbols(args.nm, Path(args.elf), args.symbol)
    pyocd_command = build_pyocd_command(args, symbols)
    report = {
        "elf": args.elf,
        "target": args.target,
        "frequency": args.frequency,
        "symbols": {name: f"0x{symbols[name]:08x}" for name in args.symbol},
        "pyocd_command": pyocd_command,
        "mode": "run" if args.run else "dry-run",
    }

    if not args.run:
        print(json.dumps(report, indent=2, ensure_ascii=False))
        return 0

    proc = subprocess.run(pyocd_command, text=True)
    report["pyocd_exit_code"] = proc.returncode
    print(json.dumps(report, indent=2, ensure_ascii=False))
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
