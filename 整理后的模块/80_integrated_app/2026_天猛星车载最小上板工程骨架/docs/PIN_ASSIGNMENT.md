# Pin Assignment

本表是当前可编译骨架的实际绑定，依据天猛星拓展板原理图整理结果和官方 MSPM0G3507 PINCM 宏。

## JY61P

| 模块 | 信号 | MSPM0G3507 | PINCM | 说明 |
|---|---|---|---|---|
| JY61P | TXD -> MCU RX | PA1 | IOMUX_PINCM2_PF_UART0_RX | UART0 RX |
| JY61P | RXD <- MCU TX | PA0 | IOMUX_PINCM1_PF_UART0_TX | UART0 TX |

参数：115200 8N1，32MHz BUSCLK，IBRD=17，FBRD=23。

## 八路 TX/RX 循迹

| 逻辑通道 | MSPM0G3507 | PINCM | 默认极性 |
|---|---|---|---|
| CH0 | PB25 | IOMUX_PINCM56 | active-low |
| CH1 | PB18 | IOMUX_PINCM44 | active-low |
| CH2 | PB21 | IOMUX_PINCM49 | active-low |
| CH3 | PB0 | IOMUX_PINCM12 | active-low |
| CH4 | PB1 | IOMUX_PINCM13 | active-low |
| CH5 | PB10 | IOMUX_PINCM27 | active-low |
| CH6 | PB11 | IOMUX_PINCM28 | active-low |
| CH7 | PB14 | IOMUX_PINCM31 | active-low |

默认打开内部弱上拉。若你的 TX/RX 模块为黑线高电平，把 `g_line_sensor.active_level` 改成 `LINE_TRACE_ACTIVE_HIGH` 或后续通过调参块更新。

## TB6612 + MG513xgmr

| TB6612 | MSPM0G3507 | PINCM/外设 | 说明 |
|---|---|---|---|
| PWMA | PA12 | TIMG0_CCP0 / IOMUX_PINCM34 | Motor A PWM |
| AIN1 | PB17 | GPIOB DIO17 / IOMUX_PINCM43 | Motor A dir |
| AIN2 | PB19 | GPIOB DIO19 / IOMUX_PINCM45 | Motor A dir |
| PWMB | PA13 | TIMG0_CCP1 / IOMUX_PINCM35 | Motor B PWM |
| BIN1 | PA16 | GPIOA DIO16 / IOMUX_PINCM38 | Motor B dir |
| BIN2 | PB24 | GPIOB DIO24 / IOMUX_PINCM52 | Motor B dir |
| STBY | +5V | board net | 硬件拉高，软件默认 PWM=0 |

首版不自动输出电机速度。真实轮向、PWM 极性和编码器相位必须悬空验证。

## OLED

| OLED | MSPM0G3507 | PINCM | 说明 |
|---|---|---|---|
| SCL | PA31 | IOMUX_PINCM6_PF_I2C0_SCL | I2C0 SCL |
| SDA | PA28 | IOMUX_PINCM3_PF_I2C0_SDA | I2C0 SDA |

默认地址 0x3C，400kHz。当前工程只做探测，不带完整 SSD1306 字库。

## 编码器预留

| 电机 | 通道 | MSPM0G3507 | 建议外设 |
|---|---|---|---|
| Motor 2 | A/B | PA26 / PA27 | TIMG8_C0/C1 |
| Motor 1 | A/B | PA25 / PA14 | TIMG12_C1/C0 |

编码器接口在此骨架里先不启用。原因是 MG513xgmr 实际方向、减速比、线序和 QEI 极性需要上板确认。
