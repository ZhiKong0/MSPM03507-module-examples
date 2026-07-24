# 拓展板模块接线表

本表对应当前 `2026_天猛星车载最小上板工程骨架` 代码里已经接入的模块。范围以 `syscfg/ti_msp_dl_config.h` 和 `docs/PIN_ASSIGNMENT.md` 为准：UART0/JY61P、128x160 RGB TFT LCD、TB6612 + MG513xgmr、电机、蜂鸣器、RGB 已进入代码骨架；八路数字循迹和编码器引脚只做预留，当前工程默认不启用八路循迹采样和编码器计数逻辑。

## 0. 上板总原则

| 项目 | 接线/处理 | 说明 |
|---|---|---|
| 主供电 | 12V 输入接拓展板 `12V` / `12V_POWER` | 主要给电机电源侧和板载降压使用。 |
| 逻辑供电 | 外设 VCC 接拓展板 `+5V` 或 3.3V | JY61P、TFT LCD、串口模块、蜂鸣器、RGB 模块上板前先按丝印确认供电范围。 |
| 共地 | 所有模块 `GND` 必须和拓展板 `GND` 共地 | UART、软件 SPI、PWM、GPIO 都依赖共地。 |
| 电机安全 | 首次上电悬空电机或架空小车 | 当前代码默认 `BOARD_APP_ENABLE_MOTOR_OUTPUT=0`，不会主动给电机输出速度。 |
| 引脚来源 | 以代码宏为最终落点 | 原理图中 UART、IMU、备用 IO 有复用区，真正编译工程以本表代码宏为准。 |

## 1. UART0 / JY61P / 当前 TX-RX 串口模块

| 端子/信号 | 接到拓展板/MSPM0G3507 | 代码宏/外设 | 参数 | 状态 |
|---|---|---|---|---|
| `VCC` | `+5V` | - | 按模块版本确认供电范围 | 已接入 |
| `GND` | `GND` | - | 共地 | 已接入 |
| MCU `UART0_TX` | `PA0` | `GPIO_UART_0_TX_PIN` / `UART0_TX` | MCU 发出数据 | 已接入 |
| MCU `UART0_RX` | `PA1` | `GPIO_UART_0_RX_PIN` / `UART0_RX` | MCU 接收数据 | 已接入 |
| 外部模块 `TXD` | `PA1` | `UART0_RX` | 模块发，MCU 收 | 推荐接法 |
| 外部模块 `RXD` | `PA0` | `UART0_TX` | MCU 发，模块收 | 推荐接法 |

通信配置：`UART0`，`115200 8N1`，BUSCLK 32MHz，`IBRD=17`，`FBRD=23`。

注意：UART 接线要交叉，`模块_TXD -> MCU_RX_PA1`，`模块_RXD <- MCU_TX_PA0`。这个 UART0 当前建议给 JY61P 或你的 TX/RX 串口模块独占，不要再同时接 USB-TTL 或 MPU6050 复用线。

## 2. 八路数字循迹预留

| 循迹通道 | 接到拓展板/MSPM0G3507 | 代码宏 | GPIO 方向 | 默认有效电平 | 状态 |
|---|---|---|---|---|---|
| `CH0` | `PB25` | `LINE_TRACE_CH0_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH1` | `PB18` | `LINE_TRACE_CH1_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH2` | `PB21` | `LINE_TRACE_CH2_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH3` | `PB0` | `LINE_TRACE_CH3_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH4` | `PB1` | `LINE_TRACE_CH4_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH5` | `PB10` | `LINE_TRACE_CH5_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH6` | `PB11` | `LINE_TRACE_CH6_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `CH7` | `PB14` | `LINE_TRACE_CH7_PIN` | 输入，上拉 | 黑线低电平 `active-low` | 预留，默认关闭 |
| `VCC` | `+5V` | - | - | 确认模块输出电平兼容 | 预留 |
| `GND` | `GND` | - | - | 共地 | 预留 |

当前 Makefile 默认 `BOARD_APP_ENABLE_LINE_GPIO=0`，未接八路循迹时遥测会标成 `LINE_TRACE_SENSOR_STALE`。如果后续接回八路数字循迹，再把这个宏改成 1；若循迹板是黑线输出高电平，需要把有效电平改为 `LINE_TRACE_ACTIVE_HIGH`。

## 3. TB6612 + MG513xgmr 电机

| TB6612/电机端子 | 接到拓展板/MSPM0G3507 | 代码宏/外设 | 作用 | 状态 |
|---|---|---|---|---|
| `PWMA` | `PA12` | `GPIO_PWM_0_C0_PIN` / `TIMG0_CCP0` | Motor A PWM | 已接入 |
| `AIN1` | `PB17` | `MOTOR_A_IN1_PIN` | Motor A 方向脚 1 | 已接入 |
| `AIN2` | `PB19` | `MOTOR_A_IN2_PIN` | Motor A 方向脚 2 | 已接入 |
| `AO1/AO2` | MG513xgmr A 电机两根线 | - | Motor A 输出到电机 | 需上板核对正反转 |
| `PWMB` | `PA13` | `GPIO_PWM_0_C1_PIN` / `TIMG0_CCP1` | Motor B PWM | 已接入 |
| `BIN1` | `PA16` | `MOTOR_B_IN1_PIN` | Motor B 方向脚 1 | 已接入 |
| `BIN2` | `PB24` | `MOTOR_B_IN2_PIN` | Motor B 方向脚 2 | 已接入 |
| `BO1/BO2` | MG513xgmr B 电机两根线 | - | Motor B 输出到电机 | 需上板核对正反转 |
| `STBY` | `+5V` | 硬件拉高 | TB6612 待机释放 | 已接入 |
| `VCC` | `+5V` | - | TB6612 逻辑电源 | 已接入 |
| `VM` | `12V_POWER` | - | 电机电源 | 已接入 |
| `GND` | `GND` | - | 共地 | 已接入 |

当前 PWM 周期宏为 `PWM_0_PERIOD_TICKS=3200`。首次调车时先悬空轮子，只给 10% 左右占空比，确认 Motor A/B 与左右轮、正反转方向、PWM 极性，再进入循迹闭环。

## 4. 128x160 RGB TFT LCD

| LCD 端子 | 接到拓展板/MSPM0G3507 | 代码宏/外设 | 参数 | 状态 |
|---|---|---|---|---|
| `VCC` | 3.3V 或 `+5V` | - | 按 LCD 模块丝印确认供电范围 | 已接入 |
| `GND` | `GND` | - | 共地 | 已接入 |
| `SCL` | `PA31` | `TFT_LCD_SCL_PIN` / GPIO 软件 SPI SCK | 原 OLED SCL 位置复用 | 已接入 |
| `SDA` | `PA28` | `TFT_LCD_SDA_PIN` / GPIO 软件 SPI MOSI | 原 OLED SDA 位置复用 | 已接入 |
| `RES` | `PA30` | `TFT_LCD_RES_PIN` | 默认低有效复位 | 默认建议，需核对实物 |
| `DC` | `PB15` | `TFT_LCD_DC_PIN` | 低=命令，高=数据 | 默认建议，需核对实物 |
| `CS` | `PA10` | `TFT_LCD_CS_PIN` | 低有效片选 | 默认建议，需核对实物 |
| `BLK` | `PB16` | `TFT_LCD_BLK_PIN` | 默认高有效背光 | 默认建议，需核对实物 |

当前代码按 ST7735 兼容 128x160 RGB565 SPI 屏初始化，启动后画彩条。`SCL/SDA` 在这个屏上不是 I2C；OLED 探测已默认关闭，`dl_i2c.c` 也不再编入当前应用。

## 5. 蜂鸣器

| 蜂鸣器端子 | 接到拓展板/MSPM0G3507 | 代码宏 | GPIO 方向 | 状态 |
|---|---|---|---|---|
| `SIG` | `PA7` | `BUZZER_PIN` | 输出 | 已接入 |
| `VCC` | `+5V` | - | - | 已接入 |
| `GND` | `GND` | - | - | 已接入 |

当前按普通 GPIO 输出处理。若实际是无源蜂鸣器，后续应改成定时器 PWM；若是有源蜂鸣器，GPIO 拉高/拉低即可响停。

## 6. RGB / 指示灯

| RGB 端子 | 接到拓展板/MSPM0G3507 | 代码宏 | GPIO 方向 | 状态 |
|---|---|---|---|---|
| `RGB0_SIG` | `PB22` | `RGB0_PIN` | 输出 | 已接入，需核对有效电平 |
| `RGB1_SIG` | `PB5` | `RGB1_PIN` | 输出 | 已接入，需核对有效电平 |
| `RGB2_SIG` | `PA18` | `RGB2_PIN` | 输出 | 已接入，需核对有效电平 |
| `VCC` | `+5V` | - | - | 需核对模块类型 |
| `GND` | `GND` | - | - | 共地 |

当前代码按普通 GPIO 指示灯处理。若实际接的是 WS2812 或三色集成 RGB，需要重写驱动方式；如果只是三路普通 LED/模块，则保留 GPIO 即可。

## 7. 编码器预留

| 编码器端子 | 预留接到 MSPM0G3507 | 建议外设 | 当前代码状态 | 备注 |
|---|---|---|---|---|
| Motor 2 Encoder A | `PA26` | `TIMG8_C0` | 未启用 | 预留 |
| Motor 2 Encoder B | `PA27` | `TIMG8_C1` | 未启用 | 预留 |
| Motor 1 Encoder A | `PA25` | `TIMG12_C1` | 未启用 | 预留 |
| Motor 1 Encoder B | `PA14` | `TIMG12_C0` | 未启用 | 预留 |
| `VCC` | `+5V` | - | 未启用 | 按电机编码器线束确认 |
| `GND` | `GND` | - | 未启用 | 共地 |

编码器暂时只写进接线预留，不进入当前编译初始化。真正上闭环速度控制前，需要确认左右轮归属、A/B 相序、正转计数方向、减速比和每圈脉冲数。

## 8. 当前最小上板核对顺序

1. 只接电源，确认 `12V`、`+5V`、`GND` 正常。
2. 接 JY61P，观察 `jy61p_angle_frames` 是否增加，yaw/pitch/roll 是否变化。
3. 接当前 TX/RX 串口模块，确认模块 TXD 接 PA1、模块 RXD 接 PA0，观察 `uart_rx_bytes` 是否增加。
4. 接 TFT LCD，确认背光和启动彩条，读 `lcd_init_ok` 是否为 1。
5. 接蜂鸣器和 RGB，确认 GPIO 有效电平。
6. 架空小车后接 TB6612 和 MG513xgmr，单独验证 A/B 电机方向。
7. 最后再启用 `BOARD_APP_ENABLE_MOTOR_OUTPUT` 并接入真实循迹动作。

## 9. 需要二次确认的点

| 项目 | 原因 | 建议 |
|---|---|---|
| Motor A/B 对应左轮还是右轮 | 原理图和车架线束无法单靠代码确认 | 上板低速点动后在文档里补充。 |
| AO1/AO2、BO1/BO2 正反转 | 电机线顺序会影响方向 | 若反了，优先交换方向宏或电机线，不要盲目加大 PWM。 |
| 循迹有效电平 | 不同 TX/RX 板黑线输出可能相反 | 用手遮挡单路后看 `active_bits`。 |
| LCD 控制器 | 128x160 常见是 ST7735/ST7735S，但也可能有偏移差异 | 若有花屏、偏色、显示偏移，先调 `TFT_LCD_X_OFFSET`、`TFT_LCD_Y_OFFSET`、`MADCTL/COLMOD`。 |
| LCD RES/DC/CS/BLK 实物脚位 | 原 720 原理图只可靠确认 PA31/PA28 显示口，控制脚来自当前默认分配 | 用丝印、万用表或高清原理图核对；若不同，只改 `TFT_LCD_*` 宏。 |
| RGB 模块类型 | 普通 GPIO LED 与 WS2812 驱动完全不同 | 当前 LCD 默认没有占用 RGB 三路；上电前仍要看模块丝印和数据线数量。 |
| UART0 复用 | JY61P、TX/RX 串口模块和部分 IMU/调试串口可能抢 PA0/PA1 | 当前工程建议 UART0 只给一个串口模块独占；需要多个串口时迁到 UART1/2/3。 |
