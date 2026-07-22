# 第一轮整理处理

## 已完成的整理处理

- 建立 `待整理的文件夹/README.md` 作为候选池总入口。
- 建立 `00_整理工作台/GOAL.md`，明确统一整理 goal 和验收标准。
- 建立 `00_整理工作台/SOURCE_INVENTORY.md`，记录当前来源规模、文件类型和首批高价值来源。
- 建立 `00_整理工作台/MODULE_TAXONOMY.md`，定义后续正式模块分类、状态标签和第一条拆解主线。
- 建立 `整理后的模块/` 正式索引区，并为官方基准、工具链、BSP、底盘闭环、循迹、IMU、OLED、通信调参、视觉协处理器、完整赛题应用和仅参考资料建立 README。

## 立即可执行的下一批文件处理

| 顺序 | 处理对象 | 操作 | 产出 |
|---:|---|---|---|
| 1 | 官方 `driverlib/gpio_toggle_output`、`uart_echo`、`timg_qei_mode`、`timx_timer_mode_pwm_*` | 抽成最小外设基准索引，不改源码 | `00_official_baseline` 候选清单 |
| 2 | `github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template` | 读取 CMake、linker、probe-rs 配置，形成本地工具链模板方案 | `10_toolchain_templates` 候选清单 |
| 3 | `github/DQ103__mspm0-car-2024-h` | 抽任务分层、app_state、wheel/chassis/pid/encoder 关系 | `30_motor_encoder` 和 `90_competition_apps` 候选清单 |
| 4 | `github/2262727886-stack__mspm0g-contest-skill` | 抽 TB6612、PID、OLED、蓝牙调参、K230 串口协议 | `30/60/70/80` 模块候选清单 |
| 5 | `github/Comet966__MSPM0G3507_examples` | 抽编码器诊断、灰度上拉/开漏注意事项、OLED 诊断页 | `30_motor_encoder` 和 `40_line_sensor` 候选清单 |

## 本轮正式索引产出

| 模块 | 状态 | 说明 |
|---|---|---|
| `整理后的模块/00_official_baseline` | `source-index` | 官方 GPIO/UART/PWM/QEI/SysConfig 基准 |
| `整理后的模块/10_toolchain_templates` | `source-index` | CMake/GCC/probe-rs/官方 Makefile 候选 |
| `整理后的模块/20_board_bsp` | `source-index` | pin map、SysConfig、板级初始化边界 |
| `整理后的模块/30_motor_encoder` | `source-index` | PWM、QEI、TB6612、编码器、速度环、底盘 |
| `整理后的模块/40_line_sensor` | `source-index` | 灰度循迹 bits/detected/position/丢线 |
| `整理后的模块/50_imu_pose` | `source-index` | MPU6050、IMU601、Yaw、航向保持 |
| `整理后的模块/60_display_ui` | `source-index` | OLED 状态页、诊断页、人机交互 |
| `整理后的模块/70_comm_tuning` | `source-index` | UART、BLE、VOFA、PID 调参协议 |
| `整理后的模块/80_vision_coprocessor` | `source-index` | K230/OpenMV/MaixCam 双芯视觉协议 |
| `整理后的模块/90_competition_apps` | `source-index` | 2024H/2025E/2026 预测小车应用骨架 |
| `整理后的模块/99_reference_only` | `source-index` | 许可证或来源受限内容 |

## 清理规则

- 对 submodule 内上游已有的 `.o/.a/.d/.map/.out/.elf/.hex/.bin`，当前不在父仓库内直接删除；正式抽模块时一律过滤。
- 对官方 SDK 导入目录，继续保持已执行的过滤策略，不新增预编译库和构建产物。
- 对无许可证来源，正式区只允许写“接口思想”和“重写实现”，不复制源码。
- 对 GPL/CC BY-NC-SA 来源，先归到 `99_reference_only`，除非未来正式模块采用兼容许可证。

## 判断结论

当前候选池已经足够支撑一套严密整理路线。最稳的第一轮不是继续无限收集，而是从官方基准和 5 个高价值第三方来源开始，先整理出“可构建、可烧录、可验证”的差速底盘最小闭环。
