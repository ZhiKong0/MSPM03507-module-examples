# 90 完整赛题应用

状态：`source-index`

## 模块目的

把官方基准、工具链模板、底盘闭环、循迹、IMU、显示、通信和视觉接成可验证的电赛控制题小车应用骨架。当前主预测方向是 MSPM0 视觉导航与定点任务小车。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | 2024H 小车任务结构、FreeRTOS 任务、app_state 中介 | `porting-candidate` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | MSPM0G + K230 全栈、调参、视觉/舵机/小车综合 | `runtime-proven` |
| `待整理的文件夹/github/JamieK32__ti-contest` | 赛题应用候选，待二次抽证据 | `source-index` |
| `待整理的文件夹/github/abcuer__2025-NUEDC-E-Ti_CAR` | 2025E TI 小车候选 | `source-index` |

## 证据路径

- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/README.md:8`：MSPM0G3507 + FreeRTOS。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/README.md:49` 到 `:60`：五个 FreeRTOS 任务分工。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/README.md:223` 到 `:225`：`sensor_task`、`control_task`、`main_task` 角色。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Sources/chassis_system.c:119` 到 `:127`：编码器、line sensor、wheel、chassis、line controller 初始化。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Sources/app_state.c:24`：应用状态初始化入口。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Sources/app_state.h:149` 到 `:164`：app_state API。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:3`：MSPM0G3507 + K230 全栈并标注实机验证。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:93` 到 `:100`：列出姿态、PID、BLE、OLED、VOFA、调试助手等功能。

## 下一步

1. 建立 `app_architecture.md`，规定任务层、设备层、控制层、通信层的数据流。
2. 第一版完整应用只做“小车能安全启动、直线、循迹、停车、显示状态、串口停止”。
3. 视觉和双车协同先作为扩展状态，等底盘闭环稳定后再接。

## 许可和构建备注

完整赛题应用最容易绑定具体年份和硬件。正式合并时只保留通用任务骨架和接口，题目特化内容放到独立 app 子目录。
