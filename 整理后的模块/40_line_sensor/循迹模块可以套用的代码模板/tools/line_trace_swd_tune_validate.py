#!/usr/bin/env python3
"""Validate E-005 SWD hot-tuning plan/evidence JSON files."""

from __future__ import annotations

import argparse
import glob
import json
import sys
from pathlib import Path
from typing import Any


DEFAULT_REQUIRED_CASES = [
    "hot_kp_step",
    "hot_kd_step",
    "hot_base_speed_step",
    "hot_max_correction_step",
    "reject_bad_kp",
    "rollback_cold_param",
]

EXPECTED_RESULTS = {
    "hot_kp_step": "applied",
    "hot_kd_step": "applied",
    "hot_base_speed_step": "applied",
    "hot_max_correction_step": "applied",
    "reject_bad_kp": "rejected",
    "rollback_cold_param": "rolled_back",
}


def load_reports(paths: list[Path]) -> list[dict[str, Any]]:
    reports: list[dict[str, Any]] = []
    for path in expand_paths(paths):
        data = json.loads(path.read_text(encoding="utf-8-sig"))
        if isinstance(data, dict) and data.get("template_only") is True:
            continue
        if isinstance(data, dict) and isinstance(data.get("reports"), list):
            for item in data["reports"]:
                report = dict(item)
                report.setdefault("source_file", str(path))
                reports.append(report)
        elif isinstance(data, dict):
            data.setdefault("source_file", str(path))
            reports.append(data)
        elif isinstance(data, list):
            for item in data:
                report = dict(item)
                report.setdefault("source_file", str(path))
                reports.append(report)
        else:
            raise SystemExit(f"unsupported tune JSON shape: {path}")
    return reports


def expand_paths(paths: list[Path]) -> list[Path]:
    expanded: list[Path] = []
    for path in paths:
        text = str(path)
        if any(char in text for char in "*?[]"):
            matches = [Path(item) for item in sorted(glob.glob(text))]
            if not matches:
                raise SystemExit(f"glob matched no files: {text}")
            expanded.extend(matches)
        else:
            expanded.append(path)
    return expanded


def first_report_by_case(reports: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    out: dict[str, dict[str, Any]] = {}
    for report in reports:
        case = str(report.get("case", ""))
        if case and case not in out:
            out[case] = report
    return out


def require_command_has(report: dict[str, Any], token: str,
                        findings: list[str]) -> None:
    case = str(report.get("case", "unknown"))
    command = " ".join(str(item) for item in report.get("pyocd_write_command", []))
    if token not in command:
        findings.append(f"{case}: pyocd_write_command missing token {token!r}")


def check_report(report: dict[str, Any], findings: list[str]) -> None:
    case = str(report.get("case", "unknown"))
    expect = str(report.get("expect", ""))
    validation = report.get("validation", {})
    status = str(validation.get("status", ""))
    mode = str(report.get("mode", ""))

    if report.get("layout", {}).get("tuning_size_bytes") != 104:
        findings.append(f"{case}: unexpected tuning block size")
    if report.get("layout", {}).get("apply_flag_offset") != 84:
        findings.append(f"{case}: unexpected apply_flag offset")
    if "g_line_tuning_block" not in report.get("symbols", {}):
        findings.append(f"{case}: missing g_line_tuning_block symbol")
    if "g_line_telemetry" not in report.get("symbols", {}):
        findings.append(f"{case}: missing g_line_telemetry symbol")
    if mode not in {"dry-run", "run"}:
        findings.append(f"{case}: mode should be dry-run or run, got {mode!r}")
    if report.get("expectation_ok") is not True:
        findings.append(f"{case}: expectation_ok is not true")

    if expect == "applied" and status != "pass":
        findings.append(f"{case}: expected applied/pass validation, got {status}")
    if expect in {"rejected", "rolled_back"} and status != "fail":
        findings.append(f"{case}: expected rejected/rolled_back fail validation, got {status}")
    if case in EXPECTED_RESULTS and expect != EXPECTED_RESULTS[case]:
        findings.append(f"{case}: expected tag should be {EXPECTED_RESULTS[case]}, got {expect}")

    require_command_has(report, "write32", findings)
    require_command_has(report, "0x00000001", findings)


def write_template(path: Path) -> None:
    template = {
        "template_only": True,
        "purpose": "Replace with line_trace_swd_tune_plan.py dry-run or real --run JSON output.",
        "required_cases": DEFAULT_REQUIRED_CASES,
        "dry_run_examples": [
            (
                "python tools\\line_trace_swd_tune_plan.py --elf "
                "build\\mspm0g3507-line-trace-smoke\\line_trace_smoke.elf "
                "--probe-uid 031305620164 --case hot_kp_step --seq 1 --kp 42 "
                "--out-json evidence\\e005\\e005-hot-kp-step.json"
            ),
            (
                "python tools\\line_trace_swd_tune_plan.py --elf "
                "build\\mspm0g3507-line-trace-smoke\\line_trace_smoke.elf "
                "--probe-uid 031305620164 --case reject_bad_kp --seq 90 --kp 3000 "
                "--expect rejected --out-json evidence\\e005\\e005-reject-bad-kp.json"
            ),
        ],
        "real_run_extra_flags": "--run --allow-ram-write",
        "hardware_gate": (
            "Before real --run, confirm target UID, raw DAP/status, fresh firmware, "
            "motor safe state, and RUNNING_TUNE_SAFE route."
        ),
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(template, indent=2, ensure_ascii=False), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="*", type=Path)
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--require-case", action="append", default=[])
    parser.add_argument("--write-template", type=Path, default=None)
    parser.add_argument("--out-json", type=Path, default=None)
    args = parser.parse_args()

    if args.write_template is not None:
        write_template(args.write_template)
        print(f"wrote template: {args.write_template}")
        return 0
    if not args.reports:
        raise SystemExit("no tune JSON files provided")

    reports = load_reports(args.reports)
    reports_by_case = first_report_by_case(reports)
    findings: list[str] = []

    required_cases = list(args.require_case)
    if args.strict:
        required_cases = DEFAULT_REQUIRED_CASES + required_cases
    for case in required_cases:
        if case not in reports_by_case:
            findings.append(f"missing case: {case}")

    for report in reports:
        check_report(report, findings)

    summary = {
        "status": "pass" if not findings else "fail",
        "report_count": len(reports),
        "case_count": len(reports_by_case),
        "cases": sorted(reports_by_case),
        "findings": findings,
    }
    if args.out_json is not None:
        args.out_json.parent.mkdir(parents=True, exist_ok=True)
        args.out_json.write_text(json.dumps(summary, indent=2, ensure_ascii=False),
                                 encoding="utf-8")
    print(json.dumps(summary, indent=2, ensure_ascii=False))
    return 0 if not findings else 1


if __name__ == "__main__":
    sys.exit(main())
