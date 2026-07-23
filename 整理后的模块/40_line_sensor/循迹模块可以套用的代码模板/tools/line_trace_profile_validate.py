#!/usr/bin/env python3
"""Validate reusable line-trace tuning profile JSON files."""

from __future__ import annotations

import argparse
import json
import sys
import zlib
from pathlib import Path
from typing import Any


PROFILE_VERSION = 1
PROFILE_CLASSES = {"dry-run-candidate", "bench-candidate", "last-known-good"}


def stable_crc32(payload: dict[str, Any]) -> int:
    clone = dict(payload)
    clone.pop("crc32", None)
    clone.pop("source_file", None)
    encoded = json.dumps(clone, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return zlib.crc32(encoded) & 0xFFFFFFFF


def load_profile(path: Path) -> dict[str, Any]:
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    if isinstance(data, dict) and data.get("template_only") is True:
        return data
    if not isinstance(data, dict):
        raise SystemExit(f"profile JSON must be an object: {path}")
    data.setdefault("source_file", str(path))
    return data


def require_list(values: Any, count: int, name: str, findings: list[str]) -> list[int]:
    if not isinstance(values, list) or len(values) != count:
        findings.append(f"{name} must be a list of {count} items")
        return []
    out: list[int] = []
    for index, value in enumerate(values):
        if not isinstance(value, int):
            findings.append(f"{name}[{index}] must be int")
            continue
        out.append(value)
    return out


def validate_algorithm(algorithm: dict[str, Any], findings: list[str]) -> None:
    weights = require_list(algorithm.get("weights"), 8, "algorithm.weights", findings)
    for index, weight in enumerate(weights):
        if weight < -200 or weight > 200:
            findings.append(f"algorithm.weights[{index}] out of range [-200, 200]: {weight}")

    checks = [
        ("kp", 0, 2000),
        ("kd", 0, 2000),
        ("base_speed", 0, 5000),
        ("max_correction", 0, 10000),
        ("corner_slow_error", 0, 1000),
        ("min_speed_percent", 0, 100),
        ("search_speed", 0, 5000),
        ("ramp_step", 0, 5000),
    ]
    for name, low, high in checks:
        value = algorithm.get(name)
        if not isinstance(value, int):
            findings.append(f"algorithm.{name} must be int")
        elif value < low or value > high:
            findings.append(f"algorithm.{name} out of range [{low}, {high}]: {value}")


def channel_mask(channel_count: int) -> int:
    if channel_count >= 8:
        return 0xFF
    return (1 << channel_count) - 1


def validate_sensor(sensor: dict[str, Any], findings: list[str]) -> None:
    channel_count = sensor.get("channel_count")
    if not isinstance(channel_count, int) or channel_count <= 0 or channel_count > 8:
        findings.append(f"sensor.channel_count out of range [1, 8]: {channel_count}")
        channel_count = 8

    active_level = sensor.get("active_level")
    if active_level not in (0, 1):
        findings.append(f"sensor.active_level must be 0 or 1: {active_level}")

    channel_order = require_list(sensor.get("channel_order"), 8, "sensor.channel_order", findings)
    if channel_order and any(channel_order[index] != 0 for index in range(channel_count)):
        seen: set[int] = set()
        for index in range(channel_count):
            value = channel_order[index]
            if value < 0 or value >= channel_count:
                findings.append(f"sensor.channel_order[{index}] outside channel_count: {value}")
            if value in seen:
                findings.append(f"sensor.channel_order duplicate channel: {value}")
            seen.add(value)

    thresholds = require_list(sensor.get("threshold"), 8, "sensor.threshold", findings)
    for index, value in enumerate(thresholds):
        if value < 0 or value > 0xFFFF:
            findings.append(f"sensor.threshold[{index}] out of u16 range: {value}")

    threshold_mode = sensor.get("threshold_mode")
    if threshold_mode not in (0, 1, 2, 3):
        findings.append(f"sensor.threshold_mode invalid: {threshold_mode}")
    i2c_addr = sensor.get("i2c_addr")
    if not isinstance(i2c_addr, int) or i2c_addr < 0 or i2c_addr > 0x7F:
        findings.append(f"sensor.i2c_addr invalid: {i2c_addr}")
    if threshold_mode == 3 and i2c_addr == 0:
        findings.append("sensor.i2c_addr must be nonzero for I2C mode")

    enable_mask = sensor.get("sensor_enable_mask")
    if not isinstance(enable_mask, int) or enable_mask < 0 or enable_mask > 0xFF:
        findings.append(f"sensor.sensor_enable_mask invalid: {enable_mask}")
    elif enable_mask & ~channel_mask(channel_count):
        findings.append(f"sensor.sensor_enable_mask outside channel_count: 0x{enable_mask:02x}")


def validate_profile(profile: dict[str, Any], require_lkg: bool) -> list[str]:
    findings: list[str] = []
    if profile.get("template_only") is True:
        return findings

    if profile.get("profile_version") != PROFILE_VERSION:
        findings.append(f"profile_version must be {PROFILE_VERSION}")
    profile_class = profile.get("profile_class")
    if profile_class not in PROFILE_CLASSES:
        findings.append(f"profile_class invalid: {profile_class}")
    if require_lkg and profile_class != "last-known-good":
        findings.append(f"profile_class must be last-known-good, got {profile_class}")

    expected_crc = stable_crc32(profile)
    actual_crc = profile.get("crc32")
    if actual_crc != expected_crc:
        findings.append(f"crc32 mismatch: expected 0x{expected_crc:08x}, got {actual_crc!r}")

    algorithm = profile.get("algorithm")
    sensor = profile.get("sensor")
    if not isinstance(algorithm, dict):
        findings.append("algorithm must be an object")
    else:
        validate_algorithm(algorithm, findings)
    if not isinstance(sensor, dict):
        findings.append("sensor must be an object")
    else:
        validate_sensor(sensor, findings)

    source = profile.get("source")
    if not isinstance(source, dict):
        findings.append("source must be an object")
        source = {}
    hardware_evidence = bool(source.get("hardware_evidence"))
    if profile_class == "last-known-good" and not hardware_evidence:
        findings.append("last-known-good requires source.hardware_evidence=true")
    if profile_class == "dry-run-candidate" and hardware_evidence:
        findings.append("dry-run-candidate should not claim hardware_evidence=true")

    verification = profile.get("verification")
    if not isinstance(verification, dict):
        findings.append("verification must be an object")
    else:
        if verification.get("plan_expect") != "applied":
            findings.append("verification.plan_expect must be applied")
        if verification.get("plan_validation_status") != "pass":
            findings.append("verification.plan_validation_status must be pass")
        required = verification.get("required_before_flash_commit")
        if not isinstance(required, list) or len(required) < 3:
            findings.append("verification.required_before_flash_commit must list hardware gates")

    return findings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("profiles", nargs="+", type=Path)
    parser.add_argument("--require-lkg", action="store_true")
    parser.add_argument("--out-json", type=Path, default=None)
    args = parser.parse_args()

    results = []
    all_findings: list[str] = []
    for path in args.profiles:
        profile = load_profile(path)
        findings = validate_profile(profile, args.require_lkg)
        status = "pass" if not findings else "fail"
        results.append({
            "path": str(path),
            "status": status,
            "profile_class": profile.get("profile_class", ""),
            "crc32": profile.get("crc32", ""),
            "findings": findings,
        })
        all_findings.extend(f"{path}: {item}" for item in findings)

    summary = {
        "status": "pass" if not all_findings else "fail",
        "profile_count": len(results),
        "results": results,
        "findings": all_findings,
    }
    if args.out_json is not None:
        args.out_json.parent.mkdir(parents=True, exist_ok=True)
        args.out_json.write_text(json.dumps(summary, indent=2, ensure_ascii=False),
                                 encoding="utf-8")
    print(json.dumps(summary, indent=2, ensure_ascii=False))
    return 0 if not all_findings else 1


if __name__ == "__main__":
    sys.exit(main())
