# 40 灰度巡线

状态：`source-index`

## 模块目的

整理灰度/红外循迹模块，首版只要求输出 `bits`、`detected`、`position` 和基础置信度；控制器只消费线位误差，不伪装独立航向。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | 7 路循迹设备层、线位置 PID、任务集成 | `porting-candidate` |
| `待整理的文件夹/github/Comet966__MSPM0G3507_examples` | 灰度/诊断候选，待二次抽证据 | `source-index` |
| `待整理的文件夹/github/military-doge__Grayscale-Line-Follower-Robot` | 灰度巡线思路参考 | `source-index` |

## 可套用模板

已新建 `循迹模块可以套用的代码模板/`。该模板重写了可移植的 GPIO 读入、线位估计、丢线方向保持和差速控制输出，适合从 4/5/7/8 路灰度模块快速接入 MSPM0G3507 小车工程。

## 证据路径

- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CMakeLists.txt:16` 和 `:42`：工程包含 `Drivers/LineTracker` 并收集 `linetracker.c`。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Drivers/Devices/line_sensor.c:5` 到 `:14`：`LineSensor_Init` 初始化 LineTracker 并清空状态。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Drivers/Devices/line_sensor.c:17` 到 `:26`：刷新传感器并输出 `bits/detected/position`。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Control/line_controller.c:30` 到 `:42`：线位置误差进入 PID，输出转向修正。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CLAUDE.md:122`：记录 7 路循迹 pin。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/docs/H_PROBLEM_PLAN.md:203` 到 `:206`：规划中明确 7 路循迹、`line_position` 输入 PID、丢线回找。

## 下一步

1. 先写 `line_sensor_contract.md`，固定输出字段：`raw_bits`、`active_count`、`position`、`confidence`、`lost_dir`。
2. 从 DQ103 的接口边界抽思路，重写一个 4/7/8 路可配置版本。
3. 把“灰度阵列前伸量”和“反向安装”作为参数，不写死在控制器里。

## 许可和构建备注

灰度模块的正式代码应尽量小而清楚，优先重写。第三方项目可保留为算法参考和 pin map 证据。
