# 00 官方基准

状态：`source-index`

## 模块目的

给所有后续模块提供 MSPM0G3507 的官方最小外设基准：GPIO 点灯、UART 收发、PWM 输出、Timer QEI 编码器模式、SysConfig 生成文件和 DriverLib 初始化流程。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/gpio_toggle_output` | GPIO、LED、SysConfig pin 定义 | `source-index` |
| `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll` | UART0 TX/RX、FIFO、9600 8N1 基准 | `source-index` |
| `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timx_timer_mode_pwm_edge_sleep` | TimerG PWM 输出 | `source-index` |
| `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timg_qei_mode` | TimerG QEI 双相编码器输入 | `source-index` |

## 证据路径

- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/README.md:3`：说明该例程发送 UART 包并等待接收数据。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/README.md:16` 和 `:17`：UART0 RX/TX 分别在 PA11/PA10。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/ti_msp_dl_config.c:47` 到 `:53`：`SYSCFG_DL_init()` 依次初始化 power、GPIO、SYSCTL、UART。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timx_timer_mode_pwm_edge_sleep/timx_timer_mode_pwm_edge_sleep.c:37` 和 `:39`：初始化 SysConfig 后启动 `PWM_0_INST`。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timx_timer_mode_pwm_edge_sleep/README.md:45` 和 `:48`：PWM 输出落到 PA12/PA13。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timg_qei_mode/timg_qei_mode.c:41` 和 `:52`：启动 QEI 定时器并读取方向。

## 下一步

1. 抽一个 `official_minimal_notes.md`，记录 GPIO/UART/PWM/QEI 的官方 pin、外设实例、SysConfig 文件边界。
2. 写一份最小外设验收表：能点灯、能串口回环、能输出 PWM、能读取编码器方向。
3. 后续正式移植时只复制 TI 官方可再分发代码和自己重写的 glue，不混入第三方小车代码。

## 许可和构建备注

来源是 TI MSPM0 SDK `2.11.00.07`，后续使用时应保留 TI SDK 许可证文件和原始来源路径。官方例程通常带 gcc/ticlang/iar/keil 工程，本仓库第一验收优先取 `gcc/makefile` 或迁移到统一 CMake。
