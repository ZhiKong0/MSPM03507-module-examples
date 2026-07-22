# 70 通信与调参

状态：`source-index`

## 模块目的

整理 UART、BLE、VOFA、日志协议和 PID 在线调参。第一阶段优先形成稳定、可复用的速度环调参协议，再扩展到线位置、航向和任务状态。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| 官方 SDK `uart_rw_multibyte_fifo_poll` | UART0 官方收发基准 | `source-index` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/06_Tools/PID_Tuner` | Python PID 调参上位机、串口协议 | `porting-candidate` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | JDY-31 BLE、VOFA+、K230 UART 框架 | `runtime-proven` |
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | 串口命令、IMU/line dashboard | `porting-candidate` |

## 证据路径

- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/README.md:7`：UART 基准为 9600 bps。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/ti_msp_dl_config.c:120` 到 `:139`：UART 初始化、波特率和 FIFO 配置。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:98` 和 `:99`：BLE 无线调参、OLED 多页显示和 VOFA+ 波形输出。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:397`：PID_Tuner 支持仿真、串口连接实机、实时曲线和 LLM 辅助推荐。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:482` 到 `:484`：调参协议要求 9 字段 CSV 和 `SET/TARGET/STATUS/RESET/STOP` 文本命令。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/06_Tools/PID_Tuner/PID_DEMO/protocol.py:1` 和 `:33`：上位机协议定义入口。

## 下一步

1. 写 `telemetry_protocol.md`：分清 debug log、VOFA CSV、PID command、K230 frame。
2. 调参命令必须带安全边界：停止命令最高优先级，PID 参数限幅，目标速度限幅。
3. 先跑纯仿真或串口假数据，再接入真实 MSPM0G 串口。

## 许可和构建备注

Python 上位机和固件协议可以借鉴，但正式协议要以本仓库文档为准；涉及 LLM 调参时不能依赖网络作为比赛现场必要条件。
