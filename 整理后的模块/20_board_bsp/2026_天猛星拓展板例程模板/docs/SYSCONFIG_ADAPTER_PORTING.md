# SysConfig Adapter Porting

这个文档说明如何把 `examples/mspm0g3507_syscfg_adapter_example.c` 接到真实 MSPM0G3507 SysConfig 工程。

## 目标

适配层负责把模板的 `tmx_board_ops_t` 回调连接到 TI DriverLib：

- GPIO：TB6612 方向脚、蜂鸣器、RGB。
- Timer PWM：TB6612 `PWMA/PWMB`，舵机 `TIMA0_C0..C3`。
- I2C：OLED 地址探测。
- UART：smoke ping 串口输出。

核心 BSP `src/tmx_expansion_board.c` 不包含 `ti_msp_dl_config.h`，这样算法和板级接口可以先在 PC/mock 上验证。

## 推荐 SysConfig 命名

| 资源 | 原理图引脚 | 推荐 SysConfig 名称 | 适配宏 |
|---|---|---|---|
| TB6612 PWM | `TIMG0_C0_PA12` / `TIMG0_C1_PA13` | `TMX_MOTOR_PWM` | `TMX_SYSCFG_MOTOR_PWM_INST` |
| 舵机 PWM | `TIMA0_C1_PA22`、`PA15`、`PA17`、`PA21` | `TMX_SERVO_PWM` | `TMX_SYSCFG_SERVO_PWM_INST` |
| OLED I2C | `I2C0_SCL_PA31` / `I2C0_SDA_PA28` | `I2C_OLED` | `TMX_SYSCFG_OLED_I2C_INST` |
| UART0 | `PA0/PA1` | `UART_0` | `TMX_SYSCFG_UART0_INST` |
| UART1 | `PB6/PB7` | `UART_1` | `TMX_SYSCFG_UART1_INST` |
| UART2 | `PA23/PA24` | `UART_2` | `TMX_SYSCFG_UART2_INST` |
| UART3 | `PB2/PB3` | `UART_3` | `TMX_SYSCFG_UART3_INST` |

如果你的 SysConfig 名称不一样，在编译选项或工程级 `board_config.h` 里定义别名即可。

## 最小宏映射

```c
#define TMX_EXAMPLE_WITH_TI_DRIVERLIB 1

#define TMX_SYSCFG_MOTOR_PWM_INST TIMG0
#define TMX_SYSCFG_MOTOR_A_PWM_CC_INDEX DL_TIMER_CC_0_INDEX
#define TMX_SYSCFG_MOTOR_B_PWM_CC_INDEX DL_TIMER_CC_1_INDEX
#define TMX_SYSCFG_MOTOR_PWM_PERIOD_TICKS 3200u

#define TMX_SYSCFG_SERVO_PWM_INST TIMA0
#define TMX_SYSCFG_SERVO1_CC_INDEX DL_TIMER_CC_1_INDEX
#define TMX_SYSCFG_SERVO2_CC_INDEX DL_TIMER_CC_2_INDEX
#define TMX_SYSCFG_SERVO3_CC_INDEX DL_TIMER_CC_3_INDEX
#define TMX_SYSCFG_SERVO4_CC_INDEX DL_TIMER_CC_0_INDEX
#define TMX_SYSCFG_SERVO_PWM_PERIOD_TICKS 64000u
#define TMX_SYSCFG_SERVO_PWM_PERIOD_US 20000u

#define TMX_SYSCFG_OLED_I2C_INST I2C0
#define TMX_SYSCFG_UART0_INST UART0
#define TMX_SYSCFG_UART1_INST UART1
#define TMX_SYSCFG_UART2_INST UART2
#define TMX_SYSCFG_UART3_INST UART3
```

GPIO 默认按原理图引脚映射：

- `PB17/PB19`：TB6612 AIN1/AIN2
- `PA16/PB24`：TB6612 BIN1/BIN2
- `PA7`：蜂鸣器
- `PB22/PB5/PA18`：RGB 三路

如果你在 SysConfig 里改了引脚，也用 `TMX_SYSCFG_*_PORT/PIN` 宏覆盖。

## PWM 极性

适配层默认按 `compare = duty_permille * period / 1000` 写入 compare。若实测 duty 越大输出越小，定义：

```c
#define TMX_SYSCFG_PWM_COMPARE_INVERTED 1
```

第一次电机测试仍然要走 `TmxBoard_ArmMotors()`，并且把 `max_motor_duty_permille` 限制到 100 到 200。

## I2C OLED Probe

默认使用 0 字节 TX transfer 作为 ACK 探测。如果你的 DriverLib/SysConfig 组合不接受 0 长度 transfer，可以改为发送一个 OLED control byte：

```c
#define TMX_SYSCFG_I2C_PROBE_SEND_OLED_CONTROL_BYTE 1
```

真实 smoke test 里 `TmxBoard_OledProbe()` 默认探测地址 `0x3C`；如果模块是 `0x3D`，修改 `tmx_board_config_t.oled_i2c_addr`。

## 本地验证边界

当前仓库可以直接验证两层：

- `src/tmx_expansion_board.c` 的 PC/mock 安全逻辑。
- `examples/mspm0g3507_syscfg_adapter_example.c` 在未定义 `TMX_EXAMPLE_WITH_TI_DRIVERLIB` 时的可包含性，确保普通 host 构建不会误依赖 TI 头文件。

真实 DriverLib 分支必须放到完整 MSPM0G3507 SysConfig 工程内编译，因为 `ti_msp_dl_config.h` 还会继续包含工程生成的 TI 设备族头文件，例如 `ti/devices/DeviceFamily.h`。如果只把零散 `ti_msp_dl_config.c/.h` 拿到本仓库旁边编译，缺少这些生成头时会先报头文件缺失，这不代表适配层逻辑已经失败。

## 接入顺序

1. 在真实工程里先调用 `SYSCFG_DL_init()`。
2. 调用 `Board_BuildTmxOps()` 构造 ops。
3. 调用 `TmxBoard_Init()`，确认所有执行器处于安全失能。
4. 按 `docs/SMOKE_TEST_PLAN.md` 逐项执行。

不要在未完成电源和机械安全确认前执行电机或舵机 movement smoke。
