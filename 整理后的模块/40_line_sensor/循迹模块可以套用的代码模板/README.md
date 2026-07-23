# 循迹模块可以套用的代码模板

状态：`template-v0.2-loop-skeleton`

这个目录给 MSPM0G3507 小车项目提供一个可复用的巡线模块模板。它不是直接复制第三方工程代码，而是把候选例程里的可取结构重新整理成独立实现：

- 算法层只处理数字化后的 `raw_bits / active_bits / position / error / confidence / lost_dir`。
- 适配层负责把 GPIO、模拟阈值/比较器或 I2C 巡线传感器转换成统一采样帧。
- 控制层把 `line error` 转换成左右轮目标速度，不直接绑定具体 PWM 或电机驱动。
- 调参层提供 RAM 参数块、显式 `apply_flag`、状态回读和范围校验，适合通过 SWD/pyOCD 做热调参。
- 证据层输出 telemetry frame，用于 PC/mock、台架、低速车测和热调参闭环。

## 目录

| 文件 | 作用 |
|---|---|
| `include/line_trace_template.h` | 对外 API、统一采样帧、调参块、telemetry 结构 |
| `src/line_trace_template.c` | 可移植巡线算法和调参 apply 实现 |
| `examples/mspm0g3507_adapter_example.c` | MSPM0G3507 GPIO/统一采样帧适配示例 |
| `examples/integration_loop_example.c` | 20 ms 控制循环、SWD 调参和 telemetry 接入示例 |
| `examples/mspm0g3507_smoke_main.c` | 不依赖 SysConfig 的 MSPM0G3507 smoke ELF 入口 |
| `examples/profile_usage_example.c` | 把冻结 profile 接入算法和控制器初始化的 C 示例 |
| `Makefile.mspm0g3507` | 使用 arm-none-eabi GNU 构建 smoke ELF/HEX/BIN |
| `tools/line_trace_swd_readback.py` | 从 ELF 自动解析 SWD 读回符号并生成 pyOCD 读命令 |
| `tools/line_trace_bench_capture.py` | 采集 `g_line_bench_snapshot` 并生成 E-003 JSON/CSV 证据 |
| `tools/line_trace_bench_validate.py` | 校验 E-003 采样是否满足通道映射和左右误差规则 |
| `tools/line_trace_swd_tune_plan.py` | 生成 E-005 SWD 热调参 RAM 参数块写入计划，默认 dry-run |
| `tools/line_trace_swd_tune_validate.py` | 校验 E-005 热调参计划/证据是否覆盖 applied/rejected/rollback |
| `tools/line_trace_profile_freeze.py` | 把已验证的 applied 调参计划冻结成可复用 JSON/C profile |
| `tools/line_trace_profile_validate.py` | 校验 profile CRC、参数范围和 last-known-good 硬件证据边界 |
| `tools/line_trace_hardware_gate_plan.py` | 生成 G-017 后的台架采样、热调参和 LKG 冻结执行计划，默认不访问硬件 |
| `evidence/e003/` | 真实台架采样证据的默认存放位置和模板 |
| `evidence/e005/` | 热调参 dry-run/真实车测证据的默认存放位置和模板 |
| `evidence/e007/` | 调参 profile freeze 和未来持久化证据模板 |
| `evidence/e008/` | G-017 硬件安全门执行计划和 dry-run 计划证据 |
| `tests/test_line_trace_mock.c` | PC/mock 算法与调参验证 |
| `docs/MSPM0_BUILD_AND_SWD.md` | MSPM0 构建、符号读回和后续硬件验证方法 |
| `docs/BENCH_SENSOR_VERIFICATION.md` | 台架传感器映射验证流程 |
| `docs/HOT_TUNING_VERIFICATION.md` | SWD 热调参 dry-run、真实写入和回滚验收流程 |
| `docs/PERSISTENCE_STRATEGY.md` | profile 状态、回滚和未来 Flash A/B commit 策略 |
| `docs/TUNING_SOP.md` | 台架、低速、热调、冻结 profile 的调参 SOP |
| `docs/LOOP_SPEC.md` | 已确认的 goal/loop 规格 |
| `docs/VERIFICATION_MATRIX.md` | 五层证据门和验收矩阵 |
| `SOURCE_ANALYSIS.md` | 候选巡线例程分析和取舍 |
| `CMakeLists.txt` | 独立 object library 和可选 host 测试入口 |

## 接入步骤

1. 把 `include/` 和 `src/` 加入工程。
2. 在板级文件里读取传感器，并填充 `line_trace_sample_frame_t`。
3. 调用 `LineTrace_UpdateFromFrame()` 得到 `line_trace_result_t`。
4. 调用 `LineTrace_ControllerStep()` 得到左右轮目标速度。
5. 在控制循环安全点调用 `LineTrace_ApplyTuningBlock()` 处理 SWD 写入的参数块。
6. 用 `LineTrace_FillTelemetry()` 输出证据帧，供 SWD、串口、OLED 或上位机读取。

旧的逐通道回调入口 `LineTrace_Update()` 仍然保留，适合最简单的 GPIO 模块；新项目建议优先使用统一采样帧入口。

## MSPM0 smoke 构建

不接硬件也可以先证明模板能按 Cortex-M0+ / MSPM0G3507 目标编译：

```powershell
Push-Location -LiteralPath '整理后的模块/40_line_sensor/循迹模块可以套用的代码模板'
mingw32-make -f Makefile.mspm0g3507 clean all
python tools\line_trace_swd_readback.py --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf --probe-uid 031305620164
Pop-Location
```

真实上板前先读 `docs/MSPM0_BUILD_AND_SWD.md`。`line_trace_swd_readback.py` 默认是 dry-run，只解析 ELF 符号并生成 pyOCD 读命令；加 `--run` 才会真正访问 SWD。

台架传感器验证时使用：

```powershell
python tools\line_trace_bench_capture.py --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf --probe-uid 031305620164 --case ch0_black
```

这条默认也是 dry-run；加 `--run --out-json ... --out-csv ...` 才会通过 SWD 只读采样。

热调参写入计划也默认不访问硬件：

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --probe-uid 031305620164 `
  --case hot_kp_step `
  --seq 1 `
  --kp 42
```

加 `--run --allow-ram-write` 才会真正通过 SWD 写 `g_line_tuning_block`，并且真实写入前必须先完成硬件安全确认、raw DAP/status gate 和电机安全状态确认。

真实上板前可先生成 G-017 硬件门计划：

```powershell
python tools\line_trace_hardware_gate_plan.py
```

默认输出 `dry-run-plan`，只解析 fresh ELF、检查 UID/pack、写入 `evidence/e008/` 的 JSON/Markdown 计划，不 attach、不 flash、不 reset、不读 SWD、不写 RAM。需要生成真实命令清单时才加 `--include-run-commands --safety-token G017-HARDWARE-SAFE`，它仍然只写计划，不执行硬件命令。

调参计划验证通过后，可以先冻结成候选 profile 便于复用审查：

```powershell
python tools\line_trace_profile_freeze.py `
  --plan evidence\e005\e005-hot-kp-step.json `
  --profile-class dry-run-candidate `
  --allow-dry-run-source `
  --out-json evidence\e007\profile-dryrun-kp42.json `
  --out-header evidence\e007\line_trace_profile_dryrun_kp42.h
```

`dry-run-candidate` 不是可写 Flash 的参数；只有真实 E-003/E-004/E-005 证据都通过后，才能升级为 `last-known-good`。

复用 profile 前先校验：

```powershell
python tools\line_trace_profile_validate.py evidence\e007\profile-dryrun-kp42.json
```

在 C 工程中引用生成的 profile 头文件时，可参考 `examples/profile_usage_example.c`。
PowerShell 下验证 include 路径时要把 `-I` 和目录分成两个参数：

```powershell
$profileDir = "$env:TEMP\line_trace_e007_freeze"
Copy-Item "$profileDir\line_trace_profile_dryrun_kp42.h" "$profileDir\line_trace_profile_generated.h"
gcc -std=c99 -Wall -Wextra -Werror -I include -I $profileDir -c examples\profile_usage_example.c -o "$profileDir\profile_usage_example.o"
```

## 推荐权重

7 路灰度：

```c
static const int16_t weights7[] = { -30, -20, -10, 0, 10, 20, 30 };
```

8 路灰度：

```c
static const int16_t weights8[] = { -35, -25, -15, -5, 5, 15, 25, 35 };
```

如果模块是“黑线输出低电平”，配置 `LINE_TRACE_ACTIVE_LOW`；如果黑线输出高电平，配置 `LINE_TRACE_ACTIVE_HIGH`。

## 调参边界

SWD/运行时调参首版采用 RAM 参数块：

- 上位机写 `g_line_tuning_block.algorithm` 和 `g_line_tuning_block.sensor`。
- 上位机把 `apply_flag` 写成 `LINE_TRACE_APPLY_REQUEST`。
- MCU 只在控制循环安全点读取候选参数、校验范围、应用参数，并写回 `status/error_code`。
- 普通 `RUNNING` 状态拒绝热写；只有 `RUNNING_TUNE_SAFE` 接受有限幅的小步热调。
- 运动中不自动写 Flash；稳定跑完后再考虑显式 commit。

## 验收顺序

1. PC/mock：验证无黑线、单点、多点、全亮/全黑、split、边缘丢线。
2. MSPM0 编译：GNU/CMake 或 Makefile 能稳定构建。
3. 台架传感器：每个通道遮挡时 `raw_bits/active_bits` 映射正确。
4. 低速车测：连续 3 圈或 3 分钟，无不可恢复丢线，无剧烈摆动。
5. 热调参车测：运动中改 `kp/kd/base_speed/max_correction`，SWD 读回 `status=applied`；越界或危险突变能拒绝/回滚并记录 `error_code`。

## 注意

模板默认 `error < 0` 表示线在左边，差速输出会让左轮减速、右轮加速。若你的电机方向或左右轮定义相反，优先在电机层修正，不要在巡线算法层到处打补丁。
