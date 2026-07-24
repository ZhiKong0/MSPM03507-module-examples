# 2026 天猛星拓展板例程模板

状态：`template-v0.1-board-bsp`

这个目录根据用户提供的 `2026电赛天猛星最终版` 原理图图片整理 MSPM0G3507 拓展板 BSP 模板。它不是完整驱动合集，而是先把板级资源统一成一层稳定接口：电源、TB6612、电机编码器、OLED、蜂鸣器、舵机、串口、IMU、RGB、备用 IO 都从同一个 pin map 和安全 smoke test 入口开始。

## 设计目标

1. 先固定板级 pin map，并把“已识别、待核对、冲突需决策”分开。
2. 代码核心不直接依赖 `ti_msp_dl_config.h`，通过 `tmx_board_ops_t` 接入 SysConfig/DriverLib。
3. smoke test 默认不让电机转、不让舵机动；运动类动作必须显式解锁。
4. 后续循迹、电机闭环、OLED 菜单、IMU 姿态、串口调参都可以复用这一层接口。

## 目录

| 文件 | 作用 |
|---|---|
| `include/tmx_expansion_board.h` | 对外 API、pin map 数据结构、安全门和 smoke step 定义 |
| `src/tmx_expansion_board.c` | 可移植 BSP 模板实现，不依赖 TI 头文件 |
| `examples/smoke_test_main.c` | 推荐上板 smoke test 调用顺序 |
| `examples/mspm0g3507_syscfg_adapter_example.c` | MSPM0G3507 SysConfig/DriverLib 适配示例骨架 |
| `tests/test_tmx_expansion_board_mock.c` | PC/mock 验证安全门、pin map 和基础动作 |
| `SCHEMATIC_PIN_MAP.md` | 从原理图图片整理出的接口表 |
| `docs/SMOKE_TEST_PLAN.md` | 上板测试顺序和通过标准 |
| `docs/SAFETY_NOTES.md` | 电源、电机、舵机、IMU/串口冲突的安全边界 |
| `CMakeLists.txt` | 独立 object library 和可选 host 测试入口 |

## 快速接入

1. 把 `include/` 和 `src/` 加入你的 MSPM0G3507 工程。
2. 用 SysConfig 生成 GPIO、Timer PWM、I2C0、UART0/1/2/3。
3. 在板级文件里实现 `tmx_board_ops_t` 的回调，可参考 `examples/mspm0g3507_syscfg_adapter_example.c`。
4. 先调用 `TmxBoard_Init()`，它会立即进入安全失能状态。
5. 按 `docs/SMOKE_TEST_PLAN.md` 的顺序逐项执行 smoke step。
6. 只有在电机方向、编码器方向、电源限流和机械固定都确认后，才调用 `TmxBoard_ArmMotors(&board, TMX_BOARD_MOTOR_ARM_COOKIE)`。

## 本地验证

不接硬件也可以先验证 BSP 模板的安全逻辑：

```powershell
Push-Location -LiteralPath '整理后的模块/20_board_bsp/2026_天猛星拓展板例程模板'
gcc -std=c99 -Wall -Wextra -Werror -I include -c src\tmx_expansion_board.c -o "$env:TEMP\tmx_expansion_board.o"
gcc -std=c99 -Wall -Wextra -Werror -I include tests\test_tmx_expansion_board_mock.c src\tmx_expansion_board.c -o "$env:TEMP\tmx_expansion_board_mock.exe"
& "$env:TEMP\tmx_expansion_board_mock.exe"
Pop-Location
```

也可以在上级 CMake 工程里打开：

```cmake
set(TMX_EXPANSION_BUILD_TESTS ON)
add_subdirectory("整理后的模块/20_board_bsp/2026_天猛星拓展板例程模板")
```

## 当前注意点

- TB6612 的 `STBY` 在图中接到 `+5V`，软件无法靠 STBY 硬关断；默认策略是方向脚低、PWM 为 0，并要求显式 arm。
- MPU6050 区域的 `SCL/SDA` 网标显示为 `UART0_RX_PA1/UART0_TX_PA0`，旁边 JY 类串口模块也用 UART0；正式接线前必须决定“MPU6050 I2C”还是“JY 串口 IMU”。
- RGB 三针区能从图上识别出 `PB22/PB5/PA18`，但 LED 类型、供电方向和有效电平需要上板核对。
- 备用 IO 排针已经整理在 `SCHEMATIC_PIN_MAP.md`，适合后续给巡线传感器、按键、拨码或调试接口分配。
