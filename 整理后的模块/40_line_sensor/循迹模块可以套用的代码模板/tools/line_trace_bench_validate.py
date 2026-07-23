#!/usr/bin/env python3
"""Validate E-003 bench-capture JSON files for line sensor mapping."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any


DEFAULT_REQUIRED_CASES = [
    "white_idle",
    "ch0_black",
    "ch1_black",
    "ch2_black",
    "ch3_black",
    "ch4_black",
    "ch5_black",
    "ch6_black",
    "ch7_black",
]


def parse_bits(value: Any) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        return int(value, 0)
    raise ValueError(f"unsupported bit value: {value!r}")


def as_i32(value: Any) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        return int(value, 0)
    raise ValueError(f"unsupported integer value: {value!r}")


def load_samples(paths: list[Path]) -> list[dict[str, Any]]:
    samples: list[dict[str, Any]] = []
    for path in paths:
        data = json.loads(path.read_text(encoding="utf-8-sig"))
        if isinstance(data, dict) and data.get("template_only") is True:
            continue
        if isinstance(data, dict) and isinstance(data.get("samples"), list):
            for sample in data["samples"]:
                item = dict(sample)
                item.setdefault("source_file", str(path))
                item.setdefault("case", data.get("case", path.stem))
                samples.append(item)
        elif isinstance(data, list):
            for sample in data:
                item = dict(sample)
                item.setdefault("source_file", str(path))
                samples.append(item)
        else:
            raise SystemExit(f"unsupported capture JSON shape: {path}")
    return samples


def first_sample_by_case(samples: list[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    out: dict[str, dict[str, Any]] = {}
    for sample in samples:
        case = str(sample.get("case", ""))
        if case and case not in out:
            out[case] = sample
    return out


def check_case_presence(samples_by_case: dict[str, dict[str, Any]],
                        required_cases: list[str],
                        findings: list[str]) -> None:
    for case in required_cases:
        if case not in samples_by_case:
            findings.append(f"missing case: {case}")


def check_channel_cases(samples_by_case: dict[str, dict[str, Any]],
                        channel_count: int,
                        center_left: int,
                        center_right: int,
                        findings: list[str]) -> None:
    for channel in range(channel_count):
        case = f"ch{channel}_black"
        sample = samples_by_case.get(case)
        if not sample:
            continue

        active_bits = parse_bits(sample.get("active_bits", 0))
        error = as_i32(sample.get("error", 0))
        confidence = as_i32(sample.get("confidence", 0))
        sensor_status = as_i32(sample.get("sensor_status", 0))
        bit = 1 << channel

        if (active_bits & bit) == 0:
            findings.append(f"{case}: active_bits does not include bit {channel}: 0x{active_bits:02x}")
        if sensor_status != 0:
            findings.append(f"{case}: sensor_status is not OK: {sensor_status}")
        if confidence <= 0:
            findings.append(f"{case}: confidence should be positive, got {confidence}")

        if channel < center_left and error >= 0:
            findings.append(f"{case}: expected negative error for left channel, got {error}")
        elif channel > center_right and error <= 0:
            findings.append(f"{case}: expected positive error for right channel, got {error}")


def check_white_idle(samples_by_case: dict[str, dict[str, Any]],
                     max_active_count: int,
                     findings: list[str]) -> None:
    sample = samples_by_case.get("white_idle")
    if not sample:
        return
    active_count = as_i32(sample.get("active_count", 0))
    if active_count > max_active_count:
        findings.append(
            f"white_idle: active_count should be <= {max_active_count}, got {active_count}"
        )


def check_left_to_right(samples: list[dict[str, Any]], findings: list[str]) -> None:
    sweep = [sample for sample in samples if sample.get("case") == "left_to_right"]
    if len(sweep) < 2:
        return
    sweep.sort(key=lambda item: as_i32(item.get("sample_index", 0)))
    errors = [as_i32(sample.get("error", 0)) for sample in sweep]
    if min(errors) >= 0:
        findings.append(f"left_to_right: no negative left-side error observed: {errors}")
    if max(errors) <= 0:
        findings.append(f"left_to_right: no positive right-side error observed: {errors}")
    if errors[-1] <= errors[0]:
        findings.append(f"left_to_right: final error should be greater than first: {errors}")


def write_template(path: Path) -> None:
    template = {
        "template_only": True,
        "purpose": "Replace sample values with real line_trace_bench_capture.py --run output.",
        "required_cases": DEFAULT_REQUIRED_CASES + ["left_to_right", "no_line_after_seen"],
        "capture_command_example": (
            "python tools\\line_trace_bench_capture.py --elf build\\mspm0g3507-line-trace-smoke\\"
            "line_trace_smoke.elf --target mspm0g3507 --probe-uid 031305620164 --case ch0_black "
            "--samples 3 --out-json evidence\\e003-ch0-black.json --out-csv evidence\\e003-ch0-black.csv --run"
        ),
        "expected_fields": [
            "case",
            "sample_index",
            "raw_bits",
            "active_bits",
            "active_count",
            "error",
            "confidence",
            "sensor_status",
            "error_code",
        ],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(template, indent=2, ensure_ascii=False), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("captures", nargs="*", type=Path, help="bench capture JSON files")
    parser.add_argument("--channel-count", type=int, default=8)
    parser.add_argument("--center-left", type=int, default=3)
    parser.add_argument("--center-right", type=int, default=4)
    parser.add_argument("--white-max-active-count", type=int, default=0)
    parser.add_argument("--require-case", action="append", default=[])
    parser.add_argument("--strict", action="store_true", help="require default E-003 cases")
    parser.add_argument("--write-template", type=Path, default=None)
    parser.add_argument("--out-json", type=Path, default=None)
    args = parser.parse_args()

    if args.write_template is not None:
        write_template(args.write_template)
        print(f"wrote template: {args.write_template}")
        return 0

    if not args.captures:
        raise SystemExit("no capture JSON files provided")

    samples = load_samples(args.captures)
    samples_by_case = first_sample_by_case(samples)
    findings: list[str] = []

    required_cases = list(args.require_case)
    if args.strict:
        required_cases = DEFAULT_REQUIRED_CASES + required_cases

    check_case_presence(samples_by_case, required_cases, findings)
    check_white_idle(samples_by_case, args.white_max_active_count, findings)
    check_channel_cases(samples_by_case, args.channel_count, args.center_left,
                        args.center_right, findings)
    check_left_to_right(samples, findings)

    report = {
        "status": "pass" if not findings else "fail",
        "sample_count": len(samples),
        "case_count": len(samples_by_case),
        "cases": sorted(samples_by_case),
        "findings": findings,
    }
    if args.out_json is not None:
        args.out_json.parent.mkdir(parents=True, exist_ok=True)
        args.out_json.write_text(json.dumps(report, indent=2, ensure_ascii=False),
                                 encoding="utf-8")
    print(json.dumps(report, indent=2, ensure_ascii=False))
    return 0 if not findings else 1


if __name__ == "__main__":
    sys.exit(main())
