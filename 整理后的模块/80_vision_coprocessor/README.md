# 80 视觉协处理器

状态：`source-index`

## 模块目的

整理 K230、OpenMV、MaixCam 等视觉协处理器与 MSPM0G3507 的串口协议。首版目标不是在 M0 上跑视觉，而是让 M0 稳定接收目标坐标、状态码和心跳。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/04_Software/K230` | K230 MicroPython 找色块、舵机跟踪、矩形追踪 | `runtime-proven` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/03_Fireware/Contest/25E_Receiver` | MSPM0G 接收 K230 FF FE 帧并驱动小车 | `porting-candidate` |
| `待整理的文件夹/github/aluomx__Nuedc-code` | 视觉/赛题应用候选，待二次抽证据 | `source-index` |

## 证据路径

- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:55`：总体链路为 GC2093 摄像头、LAB 色块识别、UART 到 MSPM0G。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:196` 和 `:197`：K230 UART3 TX/RX 引脚和 `FF FE` 帧格式。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/04_Software/README.md:7` 到 `:14`：K230 MicroPython 与 UART 9600 8N1、`FF FE` 协议。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/04_Software/K230/k230_cam_test.py:15` 到 `:18`：K230 UART 参数和协议常量。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/04_Software/K230/k230_servo_track.py:83` 到 `:89`：视觉目标与舵机追踪相关逻辑入口。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/03_Fireware/Contest/25E_Receiver/m0g_25e_receiver.c:44` 到 `:72`：M0 侧 K230 UART 帧接收。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/03_Fireware/Contest/25E_Receiver/m0g_25e_receiver.c:206` 到 `:215`：M0 侧处理 K230 数据。

## 下一步

1. 固定视觉帧契约：帧头、类型、坐标、置信度、校验、心跳超时。
2. M0 侧先做纯串口 parser + mock 数据，不急着绑 K230 具体算法。
3. 后续和底盘任务状态机连接时，必须定义“视觉丢失后的降级动作”。

## 许可和构建备注

K230 侧脚本与 M0 固件应分开维护。正式模块只保证跨芯片协议稳定，不把视觉模型或摄像头脚本塞进 M0 工程。
