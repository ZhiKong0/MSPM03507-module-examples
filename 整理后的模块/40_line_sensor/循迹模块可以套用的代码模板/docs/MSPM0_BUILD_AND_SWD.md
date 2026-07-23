# MSPM0G3507 构建与 SWD 读回方法

## 目标

这一页用于补齐巡线模块的 E-002 证据门：先证明模板能按 Cortex-M0+ / MSPM0G3507 目标构建，再给出后续通过 SWD 读取 RAM 调参块和 telemetry 的方法。

## GNU 构建

在模板目录执行：

```powershell
Push-Location -LiteralPath 'E:\Learning\嵌入式\电赛备赛\MSPM03507各模块例程代码\整理后的模块\40_line_sensor\循迹模块可以套用的代码模板'
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

构建完成后必须能在 ELF 里看到这些符号：

- `g_line_tuning_block`
- `g_line_telemetry`
- `g_line_bench_snapshot`
- `g_line_smoke_counter`
- `g_line_smoke_cookie`

这些符号是后续 SWD 读回的低风险观测点。`g_line_tuning_block` 和 `g_line_telemetry` 来自真实接入循环；`g_line_bench_snapshot` 是 4 字节对齐的台架采证快照；`g_line_smoke_counter` 和 `g_line_smoke_cookie` 用于证明 smoke 固件在跑。

## SWD 读回 dry-run

默认不访问硬件，只解析 ELF 并生成 pyOCD 读命令：

```powershell
Push-Location -LiteralPath 'E:\Learning\嵌入式\电赛备赛\MSPM03507各模块例程代码\整理后的模块\40_line_sensor\循迹模块可以套用的代码模板'
python tools\line_trace_swd_readback.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164
Pop-Location
```

示例 dry-run 会输出 JSON，里面包含每个符号地址和只读 `pyocd commander` 命令。

台架传感器验证使用专用采集工具：

```powershell
python tools\line_trace_bench_capture.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case dry_run_center
```

它默认解析 `g_line_bench_snapshot` 并生成 `read32 <addr> 22` 命令，不访问硬件。

## 真实 SWD 读回

真实读回前必须先满足硬件门：

- 只连接一个要操作的 CMSIS-DAP，或明确指定 `--probe-uid`。
- 不要同时运行其它 pyOCD、OpenOCD、SWD 面板或调参工具。
- 先按项目的 MSPM0 pyOCD 流程确认 raw DAP/status 正常。
- 不通过 SWD 直接写 GPIO、I2C、SPI、UART 或电机外设寄存器。

读回命令：

```powershell
python tools\line_trace_swd_readback.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --pack C:\Users\DZ\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack `
  --run
```

`--run` 只生成 pyOCD `status/read32/exit`，不执行写内存、擦除、烧录或复位。

## 后续进入 E-003

E-002 只证明“能构建、能解析符号、能准备 SWD 读回”。进入 E-003 时，还需要把真实 GPIO/模拟阈值/I2C 传感器接上台架，记录 `raw_bits/active_bits/error/confidence` 的通道映射证据。
