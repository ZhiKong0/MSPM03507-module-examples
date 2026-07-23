#!/usr/bin/env python3
"""Generate the G-017 hardware-gate execution plan without touching hardware."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any


EXPECTED_PROBE_UID = "031305620164"
EXPECTED_PACK_SHA256 = "071bd317fc0f152ded6b2ae594d79c6fc5eb9952370526b4c14ef5b3b9807860"
SAFETY_TOKEN = "G017-HARDWARE-SAFE"
DEFAULT_PACK = (
    r"C:\Users\DZ\AppData\Local\Temp"
    r"\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack"
)
REQUIRED_SYMBOLS = [
    "g_line_tuning_block",
    "g_line_telemetry",
    "g_line_bench_snapshot",
    "g_line_smoke_counter",
    "g_line_smoke_cookie",
]
BENCH_CASES = [
    "white_idle",
    "ch0_black",
    "ch1_black",
    "ch2_black",
    "ch3_black",
    "ch4_black",
    "ch5_black",
    "ch6_black",
    "ch7_black",
    "left_to_right",
    "no_line_after_seen",
]
HOT_TUNE_CASES = [
    ("hot_kp_step", ["--seq", "1", "--kp", "42"]),
    ("hot_kd_step", ["--seq", "2", "--kd", "12"]),
    ("hot_base_speed_step", ["--seq", "3", "--base-speed", "320"]),
    ("hot_max_correction_step", ["--seq", "4", "--max-correction", "650"]),
    ("reject_bad_kp", ["--seq", "90", "--kp", "3000", "--expect", "rejected"]),
    (
        "rollback_cold_param",
        ["--seq", "91", "--active-level", "active-high", "--expect", "rolled_back"],
    ),
]


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def find_tool(name: str) -> str:
    resolved = shutil.which(name)
    if resolved is None:
        raise SystemExit(f"tool not found: {name}")
    return resolved


def resolve_symbols(nm: str, elf: Path) -> dict[str, str]:
    if not elf.exists():
        raise SystemExit(f"ELF not found: {elf}")
    proc = subprocess.run(
        [nm, "-g", "--defined-only", str(elf)],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    wanted = set(REQUIRED_SYMBOLS)
    found: dict[str, str] = {}
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[2] in wanted:
            found[parts[2]] = f"0x{int(parts[0], 16):08x}"
    missing = sorted(wanted - set(found))
    if missing:
        raise SystemExit(f"missing required symbols: {', '.join(missing)}")
    return found


def file_sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cmd(*parts: str) -> list[str]:
    return list(parts)


def bench_capture_command(args: argparse.Namespace, case: str, run: bool) -> list[str]:
    out_base = f"evidence/e003/e003-{case}"
    command = cmd(
        "python",
        "tools\\line_trace_bench_capture.py",
        "--elf",
        args.elf,
        "--target",
        args.target,
        "--probe-uid",
        args.probe_uid,
        "--case",
        case,
    )
    if args.pack:
        command.extend(["--pack", args.pack])
    if run:
        command.extend([
            "--samples",
            str(args.samples),
            "--interval-ms",
            str(args.interval_ms),
            "--out-json",
            f"{out_base}.json",
            "--out-csv",
            f"{out_base}.csv",
            "--run",
        ])
    return command


def tune_plan_command(args: argparse.Namespace, case: str, extra: list[str], run: bool) -> list[str]:
    command = cmd(
        "python",
        "tools\\line_trace_swd_tune_plan.py",
        "--elf",
        args.elf,
        "--target",
        args.target,
        "--probe-uid",
        args.probe_uid,
        "--case",
        case,
        "--out-json",
        f"evidence/e005/e005-{case}.json",
    )
    if args.pack:
        command.extend(["--pack", args.pack])
    command.extend(extra)
    if run:
        command.extend(["--run", "--allow-ram-write"])
    return command


def command_text(command: list[str] | None) -> str:
    if command is None:
        return "(omitted until G-017 is confirmed)"
    return " ".join(command)


def build_plan(args: argparse.Namespace, symbols: dict[str, str]) -> dict[str, Any]:
    include_run = args.include_run_commands
    pack_path = Path(args.pack) if args.pack else None
    pack_exists = bool(pack_path and pack_path.exists())
    pack_sha = file_sha256(pack_path) if pack_exists and pack_path is not None else ""
    findings: list[str] = []
    if args.probe_uid != EXPECTED_PROBE_UID:
        findings.append(
            f"probe_uid must be exact UID {EXPECTED_PROBE_UID}, got {args.probe_uid}"
        )
    if pack_exists and pack_sha != EXPECTED_PACK_SHA256:
        findings.append(f"unexpected TI DFP pack SHA-256: {pack_sha}")
    if include_run and args.safety_token != SAFETY_TOKEN:
        findings.append(f"--include-run-commands requires --safety-token {SAFETY_TOKEN}")

    run_commands_enabled = include_run and not findings
    bench_steps = []
    for case in BENCH_CASES:
        step = {
            "case": case,
            "dry_run_command": bench_capture_command(args, case, False),
            "pass_hint": (
                "active_bits maps to the named channel; left channels negative error, "
                "right channels positive error"
            ),
        }
        if run_commands_enabled:
            step["run_command"] = bench_capture_command(args, case, True)
        else:
            step["run_command_omitted_until"] = (
                f"--include-run-commands --safety-token {SAFETY_TOKEN}"
            )
        bench_steps.append(step)

    tuning_steps = []
    for case, extra in HOT_TUNE_CASES:
        step = {
            "case": case,
            "dry_run_command": tune_plan_command(args, case, extra, False),
        }
        if run_commands_enabled:
            step["run_command"] = tune_plan_command(args, case, extra, True)
        else:
            step["run_command_omitted_until"] = (
                f"--include-run-commands --safety-token {SAFETY_TOKEN}"
            )
        tuning_steps.append(step)

    status_gate_command = [
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        r"C:\Users\DZ\.codex\skills\mcu-build-flash\scripts\mcu_flash_guard.ps1",
        "-Tool",
        "status-only",
        "-FirmwareRoot",
        str(repo_root()),
        "-Target",
        args.target,
        "-ProbeUid",
        args.probe_uid,
        "-StatusFrequency",
        str(args.frequency),
        "-Frequency",
        str(args.frequency),
        "-TransportKind",
        "wired",
        "-RequireProbeUid",
        "-RequireRawDap",
        "-CmsisVidPid",
        "FAED:4870",
        "-PackPath",
        args.pack,
        "-SkipBuild",
        "-SkipFlash",
        "-NoReset",
    ]

    return {
        "schema": "line_trace_g017_hardware_gate_plan_v1",
        "mode": "run-command-plan" if run_commands_enabled else "dry-run-plan",
        "requires_user_confirmation": "G-017 hardware-safety-confirmation",
        "run_commands_enabled": run_commands_enabled,
        "safety_token": SAFETY_TOKEN if run_commands_enabled else "",
        "findings": findings,
        "inputs": {
            "elf": args.elf,
            "target": args.target,
            "probe_uid": args.probe_uid,
            "expected_probe_uid": EXPECTED_PROBE_UID,
            "frequency": args.frequency,
            "pack": args.pack,
            "pack_exists": pack_exists,
            "pack_sha256": pack_sha,
            "expected_pack_sha256": EXPECTED_PACK_SHA256,
        },
        "resolved_symbols": symbols,
        "safety_checklist": [
            "MSPM0 board and line sensor are connected.",
            f"Only target probe UID {EXPECTED_PROBE_UID} is selected.",
            "Motor power is disconnected, driver is disabled, or chassis is safely lifted.",
            "Only one SWD owner will access the probe.",
            "Stop immediately if raw DAP/status gate fails.",
            "Never use SWD to write GPIO/PWM/UART/I2C/SPI/Timer peripheral registers.",
        ],
        "stages": [
            {
                "id": "E-002",
                "name": "fresh GNU build",
                "commands": [[
                    "mingw32-make",
                    "-f",
                    "Makefile.mspm0g3507",
                    "clean",
                    "all",
                ]],
                "stop_if": "build fails or required symbols are missing",
            },
            {
                "id": "G-017",
                "name": "user hardware safety confirmation",
                "commands": [],
                "stop_if": "user cannot confirm board/sensor/probe/motor safety state",
            },
            {
                "id": "E-002H",
                "name": "raw DAP/status-only gate",
                "commands": [status_gate_command] if run_commands_enabled else [],
                "omitted_until": "" if run_commands_enabled else (
                    f"--include-run-commands --safety-token {SAFETY_TOKEN}"
                ),
                "stop_if": "RAW_DAP_OK is not true or core status is unavailable",
            },
            {
                "id": "E-003",
                "name": "bench sensor mapping samples",
                "cases": bench_steps,
                "validate_command": [
                    "python",
                    "tools\\line_trace_bench_validate.py",
                    "evidence\\e003\\e003-*.json",
                    "--strict",
                ],
                "stop_if": "any channel bit, error direction, sensor_status, or confidence check fails",
            },
            {
                "id": "E-005",
                "name": "hot tuning RAM block plan and real writeback",
                "cases": tuning_steps,
                "validate_command": [
                    "python",
                    "tools\\line_trace_swd_tune_validate.py",
                    "evidence\\e005\\e005-*.json",
                    "--strict",
                ],
                "stop_if": "unsafe state, failed writeback, missing applied/rejected/rollback evidence",
            },
            {
                "id": "E-007",
                "name": "freeze last-known-good profile after real evidence",
                "commands": [[
                    "python",
                    "tools\\line_trace_profile_freeze.py",
                    "--plan",
                    "evidence\\e005\\e005-hot_kp_step.json",
                    "--profile-class",
                    "last-known-good",
                    "--name",
                    "mspm0g3507-line-trace-lkg",
                    "--out-json",
                    "evidence\\e007\\profile-last-known-good.json",
                    "--out-header",
                    "evidence\\e007\\line_trace_profile_last_known_good.h",
                ]],
                "stop_if": "source.hardware_evidence is not true or E-003/E-004/E-005 are incomplete",
            },
        ],
    }


def write_markdown(path: Path, plan: dict[str, Any]) -> None:
    lines = [
        "# G-017 Hardware Gate Plan",
        "",
        f"- mode: `{plan['mode']}`",
        f"- requires: `{plan['requires_user_confirmation']}`",
        f"- run_commands_enabled: `{str(plan['run_commands_enabled']).lower()}`",
        "",
        "## Safety Checklist",
        "",
    ]
    lines.extend(f"- {item}" for item in plan["safety_checklist"])
    if plan["findings"]:
        lines.extend(["", "## Findings", ""])
        lines.extend(f"- {item}" for item in plan["findings"])
    lines.extend(["", "## Resolved Symbols", ""])
    for name, addr in plan["resolved_symbols"].items():
        lines.append(f"- `{name}`: `{addr}`")
    lines.extend(["", "## Stages", ""])
    for stage in plan["stages"]:
        lines.append(f"### {stage['id']} {stage['name']}")
        if "commands" in stage:
            if not stage["commands"] and stage.get("omitted_until"):
                lines.extend([
                    "",
                    "```powershell",
                    f"(omitted until {stage['omitted_until']})",
                    "```",
                ])
            for command in stage["commands"]:
                lines.extend(["", "```powershell", command_text(command), "```"])
        if "cases" in stage:
            for case in stage["cases"]:
                lines.append(f"- `{case['case']}`")
                lines.extend(["", "```powershell", command_text(case["dry_run_command"]), "```"])
                if case.get("run_command") is not None:
                    lines.extend(["", "```powershell", command_text(case["run_command"]), "```"])
                elif case.get("run_command_omitted_until"):
                    lines.extend([
                        "",
                        "```powershell",
                        f"(run command omitted until {case['run_command_omitted_until']})",
                        "```",
                    ])
        if "validate_command" in stage:
            lines.extend(["", "Validate:", "", "```powershell"])
            lines.append(command_text(stage["validate_command"]))
            lines.append("```")
        lines.extend(["", f"Stop if: {stage['stop_if']}", ""])
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    root = repo_root()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--elf",
        default=str(root / "build/mspm0g3507-line-trace-smoke/line_trace_smoke.elf"),
    )
    parser.add_argument("--nm", default="arm-none-eabi-nm")
    parser.add_argument("--target", default="mspm0g3507")
    parser.add_argument("--probe-uid", default=EXPECTED_PROBE_UID)
    parser.add_argument("--frequency", type=int, default=500000)
    parser.add_argument("--pack", default=DEFAULT_PACK)
    parser.add_argument("--samples", type=int, default=3)
    parser.add_argument("--interval-ms", type=int, default=200)
    parser.add_argument("--out-json", type=Path, default=root / "evidence/e008/e008-g017-plan.json")
    parser.add_argument("--out-md", type=Path, default=root / "evidence/e008/e008-g017-plan.md")
    parser.add_argument("--include-run-commands", action="store_true")
    parser.add_argument("--safety-token", default="")
    args = parser.parse_args()

    if args.samples <= 0:
        raise SystemExit("--samples must be positive")
    if args.interval_ms < 0:
        raise SystemExit("--interval-ms must be non-negative")

    args.elf = str(Path(args.elf).resolve())
    args.nm = find_tool(args.nm)
    symbols = resolve_symbols(args.nm, Path(args.elf))
    plan = build_plan(args, symbols)

    args.out_json.parent.mkdir(parents=True, exist_ok=True)
    args.out_json.write_text(json.dumps(plan, indent=2, ensure_ascii=False), encoding="utf-8")
    write_markdown(args.out_md, plan)
    print(json.dumps({
        "status": "pass" if not plan["findings"] else "warn",
        "mode": plan["mode"],
        "run_commands_enabled": plan["run_commands_enabled"],
        "out_json": str(args.out_json),
        "out_md": str(args.out_md),
        "findings": plan["findings"],
    }, indent=2, ensure_ascii=False))
    return 0 if not plan["findings"] else 1


if __name__ == "__main__":
    sys.exit(main())
