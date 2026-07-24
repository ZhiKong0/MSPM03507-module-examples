# 20 板级 BSP

状态：`template-index`

这个目录沉淀 MSPM0G3507 板级初始化边界：电源、GPIO、I2C、UART、Timer/PWM、SysConfig 生成文件、pin map、中断入口和拓展板资源分配。BSP 层只负责“硬件怎么接、外设怎么安全初始化”，不要把循迹算法、电机闭环或比赛策略写进来。

## 已整理模板

| 目录 | 作用 | 当前状态 |
|---|---|---|
| `2026_天猛星拓展板例程模板/` | 根据 `2026电赛天猛星最终版` 原理图整理的拓展板 pin map、安全 smoke test 和 SysConfig 适配骨架 | `template-v0.1-board-bsp` |

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| 官方 SDK `gpio_toggle_output`、`uart_rw_multibyte_fifo_poll` | SysConfig 初始化顺序、GPIO/UART pin 定义 | `source-index` |
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | 实车 pin map、FreeRTOS 任务、设备对象层边界 | `porting-candidate` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | TB6612、OLED、MPU6050、K230、调试串口 pin 映射 | `porting-candidate` |

## 证据路径

- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/ti_msp_dl_config.c`：官方 SysConfig 初始化拆分为 power、GPIO、SYSCTL、UART。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/gpio_toggle_output/gpio_toggle_output.syscfg`：GPIO 组和 LED pin 分配。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CLAUDE.md`：上游把设备层和控制算法层拆开。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md`：列出 TB6612、OLED、MPU6050、K230 UART、调试 UART 的 pin 映射。
- `2026_天猛星拓展板例程模板/SCHEMATIC_PIN_MAP.md`：本仓库根据用户提供原理图图片整理的天猛星拓展板 pin map。

## 下一步

1. 用原始 EasyEDA 工程或高清导出图复核 `2026_天猛星拓展板例程模板/SCHEMATIC_PIN_MAP.md` 里的待核对项。
2. 把 `examples/mspm0g3507_syscfg_adapter_example.c` 接到真实 SysConfig 宏，补全 Timer PWM、I2C probe、UART write。
3. 电机/编码器方向确认后，把 Motor A/B 与左右轮关系写入 `30_motor_encoder`。
4. 确认 IMU 走 MPU6050 I2C 还是 JY 串口后，再进入 `50_imu_pose`。

## 许可和构建备注

BSP 是最容易发生“引脚冲突”和“复制来源混乱”的层。正式代码优先重写本仓库自己的 `board.h/board.c`；第三方 pin 表只作为证据和迁移参考。主验收路径仍然是 `CMake/Makefile + arm-none-eabi + pyOCD/DAPLink`，Keil/CCS/SysConfig 工程作为迁移参考。
