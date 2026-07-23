# 巡线参数复用与持久化策略

## 目标

巡线调参不应该停在“这次好像能跑”。每个可复用参数都要能追溯来源、
验证状态和回滚边界，然后再决定是否进入未来的 Flash A/B commit。

## 三种 profile 状态

| 状态 | 来源 | 能否上车 | 能否写 Flash |
|---|---|---|---|
| `dry-run-candidate` | PC 侧 E-005A dry-run 计划 | 不能直接作为稳定车测结论 | 不能 |
| `bench-candidate` | 通过 E-003 台架映射，但未完成车测 | 可用于低速验证 | 不能 |
| `last-known-good` | E-003、E-004、E-005 真实证据均通过 | 可作为默认运行参数 | 后续显式 commit 才能写 |

## 冻结 profile

`line_trace_profile_freeze.py` 从已验证的 E-005 调参计划生成：

- profile JSON：包含 algorithm、sensor、来源、验证状态和 CRC32。
- 可选 C 头文件：包含 `kLineTraceProfileAlgorithm` 和
  `kLineTraceProfileSensor`，用于移植到工程默认配置。

Dry-run 只能生成 `dry-run-candidate`：

```powershell
python tools\line_trace_profile_freeze.py `
  --plan evidence\e005\e005-hot-kp-step.json `
  --profile-class dry-run-candidate `
  --allow-dry-run-source `
  --name mspm0g3507-8ch-dryrun-kp42 `
  --out-json evidence\e007\profile-dryrun-kp42.json `
  --out-header evidence\e007\line_trace_profile_dryrun_kp42.h
```

真实 `last-known-good` 必须来自 `hardware_evidence=true` 的真实 E-005 计划；
工具会拒绝把 dry-run 计划直接冻结为 LKG。

冻结后必须校验 profile：

```powershell
python tools\line_trace_profile_validate.py evidence\e007\profile-dryrun-kp42.json
```

该校验会检查 CRC、参数范围、`profile_class` 和硬件证据边界。C 工程接入
可参考 `examples/profile_usage_example.c`。

## 未来 Flash A/B commit 边界

首轮模板不写 Flash，但未来 commit 应遵守：

1. 只允许从 `last-known-good` profile commit。
2. commit 前记录 `profile_version / crc32 / seq / source_case / evidence_id`。
3. 写 A/B 双槽时先写非活动槽，校验 CRC 后再切换活动槽。
4. 上电加载最新有效槽；两槽都无效时回退编译默认值并上报 `CONFIG_FAULT`。
5. `restore_default` 必须需要二次确认，并保留上一个有效 profile。

## 回滚策略

- RAM 热调失败：MCU 保持 runtime 参数不变，并写回 `REJECTED/ROLLED_BACK`。
- 车测失败：profile 保持 candidate，不升级为 `last-known-good`。
- Flash 加载失败：使用编译默认值，并要求重新台架/车测。
