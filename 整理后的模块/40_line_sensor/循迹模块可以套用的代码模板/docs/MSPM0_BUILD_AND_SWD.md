# MSPM0G3507 构建与 SWD 读回方法

## 目标

本页用于补齐巡线模块的 E-002/E-002H 证据门：先证明模板能按
Cortex-M0+ / MSPM0G3507 目标构建，再给出后续通过 SWD 读取 RAM 调参块、
telemetry 和 bench snapshot 的方法。

## GNU 构建

在模板目录执行：

```powershell
Push-Location -LiteralPath '整理后的模块/40_line_sensor/循迹模块可以套用的代码模板'
mingw32-make -f Makefile.mspm0g3507 clean all
Pop-Location
```

构建产物：

- `build/mspm0g3507-line-trace-smoke/line_trace_smoke.elf`
- `build/mspm0g3507-line-trace-smoke/line_trace_smoke.hex`
- `build/mspm0g3507-line-trace-smoke/line_trace_smoke.bin`
- `build/mspm0g3507-line-trace-smoke/line_trace_smoke.map`
- `build/mspm0g3507-line-trace-smoke/line_trace_smoke.symbols.txt`

smoke ELF 使用 MSPM0G3507 常见地址：

- Flash: `0x00000000`, 128 KiB
- SRAM: `0x20200000`, 32 KiB
- Initial SP: `0x20208000`

## 可读回符号

构建完成后必须能在 fresh ELF 里看到：

- `g_line_tuning_block`
- `g_line_telemetry`
- `g_line_bench_snapshot`
- `g_line_smoke_counter`
- `g_line_smoke_cookie`

这些符号是后续 SWD 读回的低风险观测点。每次真实 SWD 操作前都必须从
fresh ELF 解析地址，不要手抄旧地址。

## SWD 读回 dry-run

默认不访问硬件，只解析 ELF 并生成 pyOCD 读命令：

```powershell
Push-Location -LiteralPath '整理后的模块/40_line_sensor/循迹模块可以套用的代码模板'
python tools\line_trace_swd_readback.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164
Pop-Location
```

台架传感器验证使用专用采集工具：

```powershell
python tools\line_trace_bench_capture.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case dry_run_center
```

热调参计划使用：

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case hot_kp_step `
  --seq 1 `
  --kp 42
```

以上命令默认都是 dry-run，不 attach、不 flash、不 reset、不写 RAM。

## 真实 SWD 读回

真实读回前必须满足硬件门：

- 明确目标 UID，默认 `031305620164`。
- 停掉其它 pyOCD、OpenOCD、SWD 面板或调参工具。
- 按 MSPM0 pyOCD 流程确认 raw DAP/status 正常。
- 不通过 SWD 直接写 GPIO、PWM、UART、I2C、SPI 或电机外设寄存器。

读回命令：

```powershell
python tools\line_trace_swd_readback.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --pack C:\Users\DZ\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack `
  --run
```

`--run` 只执行 pyOCD `status/read32/exit`，不写内存、不擦除、不烧录、不复位。

## 后续证据门

- E-003：接真实 GPIO/模拟阈值/I2C 传感器，记录 `raw_bits/active_bits/error/confidence`。
- E-004：低速连续 3 圈或 3 分钟，验证不丢线、不剧烈摆动、能搜索恢复。
- E-005：进入 `RUNNING_TUNE_SAFE` 后，通过 RAM 参数块热调 `kp/kd/base_speed/max_correction`，并验证越界拒绝与危险突变回滚。
