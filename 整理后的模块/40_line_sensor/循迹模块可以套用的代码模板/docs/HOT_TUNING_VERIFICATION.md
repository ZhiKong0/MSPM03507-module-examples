# SWD 热调参验证方法

## 目标

E-005 用来证明巡线模块不仅能编译和台架读数，还能通过 SWD/pyOCD 修改
RAM 参数块，并由 MCU 在控制循环安全点完成 apply、拒绝或回滚。

首轮工具链分两层：

- 无硬件 dry-run：解析 fresh ELF，生成 `g_line_tuning_block` 写入计划，并在 PC 侧校验参数范围和 `RUNNING_TUNE_SAFE` 小步限制。
- 真实车测：在硬件安全门通过后，执行 RAM 写入，再用 SWD/telemetry 读回 `status/error_code/applied_seq/rejected_seq`。

## 安全边界

- 只允许写固件拥有并消费的 `g_line_tuning_block` RAM 参数块。
- 不通过 SWD 直接写 GPIO、PWM、UART、I2C、SPI、Timer 或电机驱动外设寄存器。
- `--run` 必须同时给出 `--allow-ram-write` 和明确的 `--probe-uid`。
- 真实写入前必须完成 MSPM0 raw DAP/status gate，并确认电机动力安全。
- 运动中只允许 `RUNNING_TUNE_SAFE` 小步热调；普通 `RUNNING`、`FAULT` 必须拒绝。
- 运动中不写 Flash；稳定完成车测后才进入未来的显式 commit 流程。

## Dry-Run 计划生成

先构建 fresh ELF：

```powershell
Push-Location -LiteralPath '整理后的模块/40_line_sensor/循迹模块可以套用的代码模板'
mingw32-make -f Makefile.mspm0g3507 clean all
Pop-Location
```

生成安全小步调参计划：

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case hot_kp_step `
  --seq 1 `
  --kp 42 `
  --out-json evidence\e005\e005-hot-kp-step.json
```

生成预期拒绝案例：

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case reject_bad_kp `
  --seq 90 `
  --kp 3000 `
  --expect rejected `
  --out-json evidence\e005\e005-reject-bad-kp.json
```

生成预期回滚案例：

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case rollback_cold_param `
  --seq 91 `
  --active-level active-high `
  --expect rolled_back `
  --out-json evidence\e005\e005-rollback-cold-param.json
```

校验收集到的计划/证据：

```powershell
python tools\line_trace_swd_tune_validate.py evidence\e005\e005-*.json --strict
```

## 真实 E-005 车测顺序

真实写入前先确认 G-017 硬件门：

- MSPM0 板和巡线传感器连接正确。
- 目标 UID 为 `031305620164`。
- 电机动力电源断开、车架架空或电机驱动已进入受控安全状态。
- fresh smoke/目标固件已构建。
- raw DAP/status gate 通过。

执行真实 RAM 写入时才加：

```powershell
--run --allow-ram-write
```

写入后必须独立读回：

```powershell
python tools\line_trace_swd_readback.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --pack C:\Users\DZ\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack `
  --run
```

真实 E-005 通过条件：

- `hot_kp_step`、`hot_kd_step`、`hot_base_speed_step`、`hot_max_correction_step` 均能读回 `status=APPLIED`，且 `applied_seq` 对应本次 seq。
- `reject_bad_kp` 读回 `status=REJECTED`，`error_code=BAD_CONTROL_PARAM`。
- `rollback_cold_param` 读回 `status=ROLLED_BACK`，`error_code=DANGEROUS_JUMP`。
- telemetry/bench snapshot 能看到 `safety_state=RUNNING_TUNE_SAFE`，且调参后车仍完成低速稳定巡线要求。

