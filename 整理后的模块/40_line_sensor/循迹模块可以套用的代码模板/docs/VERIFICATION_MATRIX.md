# 巡线模块验证矩阵

## Evidence Gates

| Evidence ID | Gate | Requirement | Pass Rule | Status |
|---|---|---|---|---|
| E-001 | PC/mock algorithm | G-008, G-009, G-012 | host 测试覆盖无黑线、单点、多点、全 active、split、边缘丢线、active_low、调参 apply/reject | verified: see `docs/EVIDENCE_INDEX.md` |
| E-002 | MSPM0 build | G-008, G-012 | GNU/CMake 或 Makefile 能构建 MSPM0 接入示例或目标工程 | verified for GNU target build; hardware SWD readback pending |
| E-003 | Bench sensor | G-003, G-004, G-009 | 每路遮挡时 `raw_bits/active_bits` 正确，左移 error 负，右移 error 正 | tooling/template/probe-readiness verified; real bench samples pending |
| E-004 | Low-speed car | G-008, G-009 | 低速连续 3 圈或 3 分钟，无不可恢复丢线，无明显剧烈摆动 | pending hardware |
| E-005A | Hot tuning dry-run | G-006, G-007, G-010, G-016 | 生成 SWD RAM 参数块写入计划，PC 侧验证安全小步调参、越界拒绝和冷参数回滚案例 | verified for tooling only; real RAM write pending |
| E-005 | Hot tuning car | G-006, G-007, G-010 | 运动中成功修改 `kp/kd/base_speed/max_correction` 并读回 applied；危险参数被拒绝/回滚 | pending hardware |
| E-007 | Profile freeze | G-009, G-011 | applied 调参计划能冻结为带 CRC 的可复用 profile；dry-run 不可冒充 last-known-good | verified for tooling; real last-known-good still requires hardware |

## PC/mock Cases

| Case | Input | Expected |
|---|---|---|
| no_line | active-high `raw_bits=0x00` | `detected=0`, `pattern=NO_LINE` |
| single_left | active-high channel 0 | `error < 0`, `lost_dir=LEFT` |
| single_right | active-high channel 7 | `error > 0`, `lost_dir=RIGHT` |
| center_pair | active-high channels 3 and 4 | `error=0`, `pattern=NORMAL` |
| all_active | active-high `raw_bits=0xff` | `pattern=ALL_ACTIVE` |
| split | active-high channels 0 and 7 | `pattern=SPLIT` |
| edge_lost | right edge then no line | lost frame keeps last right direction |
| active_low | active-low channel 3 low | `active_bits` maps to channel 3 |
| tuning_apply | valid RAM block in ARMED | `status=APPLIED`, runtime params updated |
| tuning_reject | out-of-range or unsafe write | `status=REJECTED/ROLLED_BACK`, `error_code` recorded |

## Hardware Notes

- 台架验证时先不要开电机，只读 `raw_bits/active_bits/error/confidence`。
- 低速车测时先固定保守参数，再逐步提高 `base_speed`。
- SWD 热调参必须记录 `seq/status/error_code/telemetry`，避免只靠肉眼判断。
- 未确认 G-017 硬件安全门时，只允许执行 E-005A dry-run/validator，不执行真实 SWD RAM 写入。
- dry-run profile 只能标记为 `dry-run-candidate`；写 Flash 的 `last-known-good` 必须等真实 E-003/E-004/E-005 证据齐全。
