#!/usr/bin/env python3
"""Generate a safe SWD RAM tuning-block write plan for line tracing."""

from __future__ import annotations

import argparse
import json
import shutil
import struct
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any


TUNING_SYMBOL = "g_line_tuning_block"
TELEMETRY_SYMBOL = "g_line_telemetry"

TUNING_MAGIC = 0x4C54554E
TUNING_VERSION = 1
TUNING_SIZE_BYTES = 104
TUNING_WORDS = TUNING_SIZE_BYTES // 4
APPLY_REQUEST = 1

ACTIVE_LEVELS = {
    "active-low": 0,
    "active-high": 1,
}

THRESHOLD_MODES = {
    "none": 0,
    "digital-gpio": 1,
    "comparator": 2,
    "i2c": 3,
}

SAFETY_STATES = {
    "DISARMED": 0,
    "ARMED": 1,
    "RUNNING": 2,
    "RUNNING_TUNE_SAFE": 3,
    "FAULT": 4,
}

ERROR_OK = "LINE_TRACE_ERROR_OK"
ERROR_BAD_CHANNEL_COUNT = "LINE_TRACE_ERROR_BAD_CHANNEL_COUNT"
ERROR_BAD_ACTIVE_LEVEL = "LINE_TRACE_ERROR_BAD_ACTIVE_LEVEL"
ERROR_BAD_WEIGHT = "LINE_TRACE_ERROR_BAD_WEIGHT"
ERROR_BAD_CONTROL_PARAM = "LINE_TRACE_ERROR_BAD_CONTROL_PARAM"
ERROR_DANGEROUS_JUMP = "LINE_TRACE_ERROR_DANGEROUS_JUMP"
ERROR_UNSAFE_STATE = "LINE_TRACE_ERROR_UNSAFE_STATE"
ERROR_SENSOR_FAULT = "LINE_TRACE_ERROR_SENSOR_FAULT"


@dataclass
class AlgorithmParams:
    weights: list[int]
    kp: int
    kd: int
    base_speed: int
    max_correction: int
    corner_slow_error: int
    min_speed_percent: int
    search_speed: int
    ramp_step: int


@dataclass
class SensorParams:
    channel_count: int
    active_level: int
    channel_order: list[int]
    threshold: list[int]
    threshold_mode: int
    i2c_addr: int
    sensor_enable_mask: int


DEFAULT_ALGORITHM = AlgorithmParams(
    weights=[-35, -25, -15, -5, 5, 15, 25, 35],
    kp=12,
    kd=4,
    base_speed=260,
    max_correction=500,
    corner_slow_error=25,
    min_speed_percent=65,
    search_speed=180,
    ramp_step=10,
)

DEFAULT_SENSOR = SensorParams(
    channel_count=8,
    active_level=ACTIVE_LEVELS["active-low"],
    channel_order=[0] * 8,
    threshold=[0] * 8,
    threshold_mode=THRESHOLD_MODES["digital-gpio"],
    i2c_addr=0,
    sensor_enable_mask=0xFF,
)


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


def parse_i16_list(text: str, expected: int, name: str) -> list[int]:
    values = [int(item.strip(), 0) for item in text.split(",") if item.strip()]
    if len(values) != expected:
        raise SystemExit(f"--{name} needs {expected} comma-separated values")
    return values


def parse_u8_list(text: str, expected: int, name: str) -> list[int]:
    values = parse_i16_list(text, expected, name)
    for value in values:
        if value < 0 or value > 0xFF:
            raise SystemExit(f"--{name} item out of u8 range: {value}")
    return values


def parse_u16_list(text: str, expected: int, name: str) -> list[int]:
    values = parse_i16_list(text, expected, name)
    for value in values:
        if value < 0 or value > 0xFFFF:
            raise SystemExit(f"--{name} item out of u16 range: {value}")
    return values


def channel_mask(channel_count: int) -> int:
    if channel_count >= 8:
        return 0xFF
    return (1 << channel_count) - 1


def load_current(path_text: str) -> tuple[AlgorithmParams, SensorParams]:
    if not path_text:
        return DEFAULT_ALGORITHM, DEFAULT_SENSOR
    data = json.loads(Path(path_text).read_text(encoding="utf-8-sig"))
    algorithm = AlgorithmParams(**data["algorithm"])
    sensor = SensorParams(**data["sensor"])
    normalize_params(algorithm, sensor)
    return algorithm, sensor


def normalize_params(algorithm: AlgorithmParams, sensor: SensorParams) -> None:
    algorithm.weights = list(algorithm.weights[:8]) + [0] * max(0, 8 - len(algorithm.weights))
    sensor.channel_order = list(sensor.channel_order[:8]) + [0] * max(0, 8 - len(sensor.channel_order))
    sensor.threshold = list(sensor.threshold[:8]) + [0] * max(0, 8 - len(sensor.threshold))


def apply_cli_overrides(args: argparse.Namespace,
                        algorithm: AlgorithmParams,
                        sensor: SensorParams) -> None:
    for field in [
        "kp",
        "kd",
        "base_speed",
        "max_correction",
        "corner_slow_error",
        "min_speed_percent",
        "search_speed",
        "ramp_step",
    ]:
        value = getattr(args, field)
        if value is not None:
            setattr(algorithm, field, value)

    if args.weights:
        algorithm.weights = parse_i16_list(args.weights, 8, "weights")

    if args.channel_count is not None:
        sensor.channel_count = args.channel_count
    if args.active_level:
        sensor.active_level = ACTIVE_LEVELS[args.active_level]
    if args.channel_order:
        sensor.channel_order = parse_u8_list(args.channel_order, 8, "channel-order")
    if args.threshold:
        sensor.threshold = parse_u16_list(args.threshold, 8, "threshold")
    if args.threshold_mode:
        sensor.threshold_mode = THRESHOLD_MODES[args.threshold_mode]
    if args.i2c_addr is not None:
        sensor.i2c_addr = args.i2c_addr
    if args.sensor_enable_mask is not None:
        sensor.sensor_enable_mask = args.sensor_enable_mask
    normalize_params(algorithm, sensor)


def validate_algorithm(algorithm: AlgorithmParams, findings: list[str]) -> str:
    for index, weight in enumerate(algorithm.weights):
        if weight < -200 or weight > 200:
            findings.append(f"weight[{index}] out of range [-200, 200]: {weight}")
            return ERROR_BAD_WEIGHT

    checks = [
        ("kp", algorithm.kp, 0, 2000),
        ("kd", algorithm.kd, 0, 2000),
        ("base_speed", algorithm.base_speed, 0, 5000),
        ("max_correction", algorithm.max_correction, 0, 10000),
        ("corner_slow_error", algorithm.corner_slow_error, 0, 1000),
        ("min_speed_percent", algorithm.min_speed_percent, 0, 100),
        ("search_speed", algorithm.search_speed, 0, 5000),
        ("ramp_step", algorithm.ramp_step, 0, 5000),
    ]
    for name, value, low, high in checks:
        if value < low or value > high:
            findings.append(f"{name} out of range [{low}, {high}]: {value}")
            return ERROR_BAD_CONTROL_PARAM
    return ERROR_OK


def channel_order_is_default(sensor: SensorParams) -> bool:
    return all(sensor.channel_order[index] == 0 for index in range(sensor.channel_count))


def validate_sensor(sensor: SensorParams, findings: list[str]) -> str:
    if sensor.channel_count <= 0 or sensor.channel_count > 8:
        findings.append(f"channel_count out of range [1, 8]: {sensor.channel_count}")
        return ERROR_BAD_CHANNEL_COUNT
    if sensor.active_level not in ACTIVE_LEVELS.values():
        findings.append(f"active_level invalid: {sensor.active_level}")
        return ERROR_BAD_ACTIVE_LEVEL
    if sensor.threshold_mode not in THRESHOLD_MODES.values():
        findings.append(f"threshold_mode invalid: {sensor.threshold_mode}")
        return ERROR_SENSOR_FAULT
    if sensor.sensor_enable_mask & ~channel_mask(sensor.channel_count):
        findings.append(
            f"sensor_enable_mask has bits outside channel_count: 0x{sensor.sensor_enable_mask:02x}"
        )
        return ERROR_BAD_CHANNEL_COUNT
    if sensor.threshold_mode == THRESHOLD_MODES["i2c"] and (
        sensor.i2c_addr == 0 or sensor.i2c_addr > 0x7F
    ):
        findings.append(f"i2c_addr invalid for I2C mode: 0x{sensor.i2c_addr:02x}")
        return ERROR_SENSOR_FAULT

    if channel_order_is_default(sensor):
        return ERROR_OK

    seen = set()
    for index in range(sensor.channel_count):
        channel = sensor.channel_order[index]
        if channel >= sensor.channel_count:
            findings.append(f"channel_order[{index}] outside channel_count: {channel}")
            return ERROR_BAD_CHANNEL_COUNT
        if channel in seen:
            findings.append(f"channel_order duplicate channel: {channel}")
            return ERROR_BAD_CHANNEL_COUNT
        seen.add(channel)
    return ERROR_OK


def absdiff_gt(a: int, b: int, limit: int) -> bool:
    return abs(a - b) > limit


def validate_running_change(next_algorithm: AlgorithmParams,
                            next_sensor: SensorParams,
                            current_algorithm: AlgorithmParams,
                            current_sensor: SensorParams,
                            safety_state: str,
                            findings: list[str]) -> str:
    if safety_state in {"RUNNING", "FAULT"}:
        findings.append(f"safety_state rejects tuning: {safety_state}")
        return ERROR_UNSAFE_STATE
    if safety_state != "RUNNING_TUNE_SAFE":
        return ERROR_OK

    cold_fields = [
        ("channel_count", next_sensor.channel_count, current_sensor.channel_count),
        ("active_level", next_sensor.active_level, current_sensor.active_level),
        ("threshold_mode", next_sensor.threshold_mode, current_sensor.threshold_mode),
        ("i2c_addr", next_sensor.i2c_addr, current_sensor.i2c_addr),
        ("sensor_enable_mask", next_sensor.sensor_enable_mask, current_sensor.sensor_enable_mask),
    ]
    for name, next_value, current_value in cold_fields:
        if next_value != current_value:
            findings.append(
                f"{name} is cold while running: current={current_value}, next={next_value}"
            )
            return ERROR_DANGEROUS_JUMP

    for index in range(8):
        if next_sensor.channel_order[index] != current_sensor.channel_order[index]:
            findings.append(f"channel_order[{index}] changed while running")
            return ERROR_DANGEROUS_JUMP
        if absdiff_gt(next_algorithm.weights[index], current_algorithm.weights[index], 20):
            findings.append(
                f"weights[{index}] step too large: current={current_algorithm.weights[index]}, "
                f"next={next_algorithm.weights[index]}"
            )
            return ERROR_DANGEROUS_JUMP
        if absdiff_gt(next_sensor.threshold[index], current_sensor.threshold[index], 100):
            findings.append(
                f"threshold[{index}] step too large: current={current_sensor.threshold[index]}, "
                f"next={next_sensor.threshold[index]}"
            )
            return ERROR_DANGEROUS_JUMP

    running_checks = [
        ("kp", next_algorithm.kp, current_algorithm.kp, 150),
        ("kd", next_algorithm.kd, current_algorithm.kd, 150),
        ("base_speed", next_algorithm.base_speed, current_algorithm.base_speed, 200),
        ("max_correction", next_algorithm.max_correction, current_algorithm.max_correction, 300),
        ("search_speed", next_algorithm.search_speed, current_algorithm.search_speed, 200),
        ("ramp_step", next_algorithm.ramp_step, current_algorithm.ramp_step, 100),
    ]
    for name, next_value, current_value, limit in running_checks:
        if absdiff_gt(next_value, current_value, limit):
            findings.append(
                f"{name} step too large: current={current_value}, next={next_value}, limit={limit}"
            )
            return ERROR_DANGEROUS_JUMP

    if next_algorithm.corner_slow_error != current_algorithm.corner_slow_error:
        findings.append("corner_slow_error is fixed during RUNNING_TUNE_SAFE")
        return ERROR_DANGEROUS_JUMP
    if next_algorithm.min_speed_percent != current_algorithm.min_speed_percent:
        findings.append("min_speed_percent is fixed during RUNNING_TUNE_SAFE")
        return ERROR_DANGEROUS_JUMP
    return ERROR_OK


def validate_candidate(next_algorithm: AlgorithmParams,
                       next_sensor: SensorParams,
                       current_algorithm: AlgorithmParams,
                       current_sensor: SensorParams,
                       safety_state: str) -> tuple[str, list[str]]:
    findings: list[str] = []
    error = validate_algorithm(next_algorithm, findings)
    if error == ERROR_OK:
        error = validate_sensor(next_sensor, findings)
    if error == ERROR_OK:
        error = validate_running_change(
            next_algorithm, next_sensor, current_algorithm, current_sensor,
            safety_state, findings
        )
    return error, findings


def pack_i16_pair(left: int, right: int) -> int:
    data = struct.pack("<hh", left, right)
    return struct.unpack("<I", data)[0]


def build_tuning_words(seq: int, algorithm: AlgorithmParams,
                       sensor: SensorParams, apply_flag: int) -> list[int]:
    data = bytearray(TUNING_SIZE_BYTES)
    struct.pack_into("<IHHI", data, 0, TUNING_MAGIC, TUNING_VERSION,
                     TUNING_SIZE_BYTES, seq)
    offset = 12
    for index in range(0, 8, 2):
        struct.pack_into("<I", data, offset, pack_i16_pair(
            algorithm.weights[index], algorithm.weights[index + 1]
        ))
        offset += 4
    for left, right in [
        (algorithm.kp, algorithm.kd),
        (algorithm.base_speed, algorithm.max_correction),
        (algorithm.corner_slow_error, algorithm.min_speed_percent),
        (algorithm.search_speed, algorithm.ramp_step),
    ]:
        struct.pack_into("<I", data, offset, pack_i16_pair(left, right))
        offset += 4

    struct.pack_into("<B", data, 44, sensor.channel_count)
    struct.pack_into("<I", data, 48, sensor.active_level)
    struct.pack_into("<8B", data, 52, *sensor.channel_order)
    struct.pack_into("<8H", data, 60, *sensor.threshold)
    struct.pack_into("<I", data, 76, sensor.threshold_mode)
    struct.pack_into("<B", data, 80, sensor.i2c_addr)
    struct.pack_into("<B", data, 81, sensor.sensor_enable_mask)
    struct.pack_into("<I", data, 84, apply_flag)
    struct.pack_into("<I", data, 88, 0)
    struct.pack_into("<I", data, 92, 0)
    return [struct.unpack_from("<I", data, index)[0] for index in range(0, len(data), 4)]


def build_pyocd_write_command(args: argparse.Namespace, block_addr: int,
                              words: list[int]) -> list[str]:
    cmd = [args.pyocd, "commander", "--no-config"]
    if args.pack:
        cmd.extend(["--pack", args.pack])
    if args.probe_uid:
        cmd.extend(["-u", args.probe_uid])
    cmd.extend(["-t", args.target, "-f", str(args.frequency), "-c", "status"])
    cmd.extend(["-c", f"write32 0x{block_addr + 84:08x} 0x00000000"])
    for index, word in enumerate(words):
        offset = index * 4
        if offset == 84:
            continue
        if offset >= 96:
            continue
        cmd.extend(["-c", f"write32 0x{block_addr + offset:08x} 0x{word:08x}"])
    cmd.extend(["-c", f"write32 0x{block_addr + 84:08x} 0x{APPLY_REQUEST:08x}"])
    cmd.extend(["-c", "exit"])
    return cmd


def build_pyocd_readback_command(args: argparse.Namespace, block_addr: int,
                                 telemetry_addr: int) -> list[str]:
    cmd = [args.pyocd, "commander", "--no-config"]
    if args.pack:
        cmd.extend(["--pack", args.pack])
    if args.probe_uid:
        cmd.extend(["-u", args.probe_uid])
    cmd.extend(["-t", args.target, "-f", str(args.frequency), "-c", "status"])
    cmd.extend(["-c", f"read32 0x{block_addr:08x} {TUNING_WORDS}"])
    if telemetry_addr:
        cmd.extend(["-c", f"read32 0x{telemetry_addr:08x} 6"])
    cmd.extend(["-c", "exit"])
    return cmd


def maybe_run(command: list[str], enabled: bool) -> int | None:
    if not enabled:
        return None
    proc = subprocess.run(command, text=True)
    return proc.returncode


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
    parser.add_argument("--case", default="hot_kp_step", help="evidence case label")
    parser.add_argument("--seq", type=int, default=1, help="tuning sequence number")
    parser.add_argument(
        "--safety-state",
        choices=sorted(SAFETY_STATES),
        default="RUNNING_TUNE_SAFE",
        help="expected firmware safety state for static validation",
    )
    parser.add_argument("--current-json", default="", help="current params JSON baseline")
    parser.add_argument("--expect", choices=["applied", "rejected", "rolled_back"], default="applied")

    parser.add_argument("--weights", default="")
    parser.add_argument("--kp", type=int, default=None)
    parser.add_argument("--kd", type=int, default=None)
    parser.add_argument("--base-speed", type=int, default=None)
    parser.add_argument("--max-correction", type=int, default=None)
    parser.add_argument("--corner-slow-error", type=int, default=None)
    parser.add_argument("--min-speed-percent", type=int, default=None)
    parser.add_argument("--search-speed", type=int, default=None)
    parser.add_argument("--ramp-step", type=int, default=None)

    parser.add_argument("--channel-count", type=int, default=None)
    parser.add_argument("--active-level", choices=sorted(ACTIVE_LEVELS), default="")
    parser.add_argument("--channel-order", default="")
    parser.add_argument("--threshold", default="")
    parser.add_argument("--threshold-mode", choices=sorted(THRESHOLD_MODES), default="")
    parser.add_argument("--i2c-addr", type=int, default=None)
    parser.add_argument("--sensor-enable-mask", type=lambda text: int(text, 0), default=None)

    parser.add_argument("--out-json", type=Path, default=None)
    parser.add_argument("--run", action="store_true", help="execute SWD write command")
    parser.add_argument(
        "--allow-ram-write",
        action="store_true",
        help="required with --run; writes only firmware-owned RAM tuning block",
    )
    args = parser.parse_args()

    if args.seq < 0:
        raise SystemExit("--seq must be non-negative")
    if args.run and (not args.allow_ram_write or not args.probe_uid):
        raise SystemExit("--run requires --allow-ram-write and --probe-uid")

    args.elf = str(Path(args.elf).resolve())
    args.nm = find_tool(args.nm)
    if args.run:
        args.pyocd = find_tool(args.pyocd)

    current_algorithm, current_sensor = load_current(args.current_json)
    candidate_algorithm = AlgorithmParams(**asdict(current_algorithm))
    candidate_sensor = SensorParams(**asdict(current_sensor))
    apply_cli_overrides(args, candidate_algorithm, candidate_sensor)

    error_code, findings = validate_candidate(
        candidate_algorithm, candidate_sensor, current_algorithm, current_sensor,
        args.safety_state
    )
    validation_status = "pass" if error_code == ERROR_OK else "fail"
    expectation_ok = (
        (args.expect == "applied" and validation_status == "pass") or
        (args.expect in {"rejected", "rolled_back"} and validation_status == "fail")
    )

    symbols = resolve_symbols(args.nm, Path(args.elf), [TUNING_SYMBOL, TELEMETRY_SYMBOL])
    block_addr = symbols[TUNING_SYMBOL]
    telemetry_addr = symbols[TELEMETRY_SYMBOL]
    if block_addr % 4:
        raise SystemExit(f"{TUNING_SYMBOL} not 4-byte aligned: 0x{block_addr:08x}")
    if telemetry_addr % 4:
        raise SystemExit(f"{TELEMETRY_SYMBOL} not 4-byte aligned: 0x{telemetry_addr:08x}")

    words = build_tuning_words(args.seq, candidate_algorithm, candidate_sensor, 0)
    write_command = build_pyocd_write_command(args, block_addr, words)
    readback_command = build_pyocd_readback_command(args, block_addr, telemetry_addr)
    write_exit_code = maybe_run(write_command, args.run)

    report: dict[str, Any] = {
        "mode": "run" if args.run else "dry-run",
        "case": args.case,
        "expect": args.expect,
        "expectation_ok": expectation_ok,
        "elf": args.elf,
        "target": args.target,
        "frequency": args.frequency,
        "safety_state": args.safety_state,
        "symbols": {
            TUNING_SYMBOL: f"0x{block_addr:08x}",
            TELEMETRY_SYMBOL: f"0x{telemetry_addr:08x}",
        },
        "layout": {
            "tuning_size_bytes": TUNING_SIZE_BYTES,
            "tuning_words": TUNING_WORDS,
            "apply_flag_offset": 84,
            "status_offset": 88,
            "error_code_offset": 92,
        },
        "current": {
            "algorithm": asdict(current_algorithm),
            "sensor": asdict(current_sensor),
        },
        "candidate": {
            "seq": args.seq,
            "algorithm": asdict(candidate_algorithm),
            "sensor": asdict(candidate_sensor),
        },
        "validation": {
            "status": validation_status,
            "error_code": error_code,
            "findings": findings,
        },
        "pyocd_write_command": write_command,
        "pyocd_readback_command": readback_command,
        "write_exit_code": write_exit_code,
        "hardware_evidence": args.run,
    }

    if args.out_json is not None:
        args.out_json.parent.mkdir(parents=True, exist_ok=True)
        args.out_json.write_text(json.dumps(report, indent=2, ensure_ascii=False),
                                 encoding="utf-8")
    print(json.dumps(report, indent=2, ensure_ascii=False))
    return 0 if expectation_ok and (write_exit_code in (None, 0)) else 1


if __name__ == "__main__":
    sys.exit(main())
