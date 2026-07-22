# 20 板级 BSP

状态：`source-index`

## 模块目的

沉淀 MSPM0G3507 板级初始化边界：时钟、电源、GPIO、I2C、UART、Timer、SysConfig 生成文件、pin map 和中断入口。这个目录先做索引，后续再拆成可复用 BSP。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| 官方 SDK `gpio_toggle_output`、`uart_rw_multibyte_fifo_poll` | SysConfig 初始化顺序、GPIO/UART pin 定义 | `source-index` |
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | 实车 pin map、FreeRTOS 任务、设备对象层边界 | `porting-candidate` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | TB6612、OLED、MPU6050、K230、调试串口 pin 映射 | `porting-candidate` |

## 证据路径

- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/ti_msp_dl_config.c:47` 到 `:53`：官方 SysConfig 初始化拆为 power、GPIO、SYSCTL、UART。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/gpio_toggle_output/gpio_toggle_output.syscfg:19` 到 `:25`：GPIO 组和 LED pin 分配。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CLAUDE.md:63` 和 `:64`：上游把设备层和控制算法层拆开。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CLAUDE.md:120`：记录 MPU6050 I2C 引脚 PA12/PA13。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:181` 到 `:199`：列出 TB6612、OLED、MPU6050、K230 UART、调试 UART 的 pin 映射。

## 下一步

1. 新建统一 `pin_map_candidates.md`，把官方、DQ103、contest-skill 的冲突 pin 标出来。
2. 将 SysConfig 生成代码边界写清：哪些文件可生成、哪些文件必须手写维护。
3. 先不要确定最终比赛接线，等电机、OLED、BNO/MPU、视觉模块都定后再合并 pin 表。

## 许可和构建备注

BSP 是最容易发生“引脚冲突”和“复制来源混乱”的层。正式代码应优先重写本仓库自己的 `board.h/board.c`，第三方 pin 表仅作证据和迁移参考。
