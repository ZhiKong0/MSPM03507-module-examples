# Pin Assignment

本表是当前可编译骨架的实际绑定，依据天猛星拓展板原理图整理结果和官方 MSPM0G3507 PINCM 宏。

## UART0 / JY61P / 当前 TX-RX 串口模块

| 模块 | 信号 | MSPM0G3507 | PINCM | 说明 |
|---|---|---|---|---|
| UART0 | MCU TX | PA0 | IOMUX_PINCM1_PF_UART0_TX | MCU 发出数据 |
| UART0 | MCU RX | PA1 | IOMUX_PINCM2_PF_UART0_RX | MCU 接收数据 |
| 外部串口模块 | TXD -> MCU RX | PA1 | IOMUX_PINCM2_PF_UART0_RX | 模块发，MCU 收 |
| 外部串口模块 | RXD <- MCU TX | PA0 | IOMUX_PINCM1_PF_UART0_TX | MCU 发，模块收 |

参数：115200 8N1，32MHz BUSCLK，IBRD=17，FBRD=23。

推荐接法：外部模块 `TXD` 接 `PA1/UART0_RX`，外部模块 `RXD` 接 `PA0/UART0_TX`，`GND` 必须共地，`VCC` 按模块丝印接 3.3V 或 5V。不要把两个模块同时挂在 UART0 上；如果 JY61P 和另一个 TX/RX 串口模块都要用，应把其中一个迁到 UART1/2/3。

## 八路数字循迹预留

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

当前硬件没有接八路循迹，Makefile 默认 `BOARD_APP_ENABLE_LINE_GPIO=0`。若后续改回八路数字循迹，再把它改成 1；默认打开内部弱上拉。若你的循迹模块为黑线高电平，把 `g_line_sensor.active_level` 改成 `LINE_TRACE_ACTIVE_HIGH` 或后续通过调参块更新。

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

## 128x160 RGB TFT LCD

当前工程不再初始化 OLED/I2C 探测，改为 ST7735 兼容 4 线 SPI TFT LCD 最小驱动。`SCL`/`SDA` 在这种 LCD 模块上不是 I2C，而是 SPI 时钟和 MOSI 数据线。

| LCD 端子 | MSPM0G3507 | PINCM | 代码宏 | 说明 |
|---|---|---|---|---|
| VCC | 3.3V 或 +5V | board net | - | 按 LCD 模块丝印确认，信号电平来自 MSPM0 3.3V GPIO |
| GND | GND | board net | - | 必须和主控共地 |
| SCL | PA31 | IOMUX_PINCM6 | `TFT_LCD_SCL_PIN` | 软件 SPI SCK，复用原 OLED SCL 位置 |
| SDA | PA28 | IOMUX_PINCM3 | `TFT_LCD_SDA_PIN` | 软件 SPI MOSI，复用原 OLED SDA 位置 |
| RES | PA30 | IOMUX_PINCM5 | `TFT_LCD_RES_PIN` | LCD 复位，默认低有效 |
| DC | PB15 | IOMUX_PINCM32 | `TFT_LCD_DC_PIN` | 命令/数据选择，低=命令，高=数据 |
| CS | PA10 | IOMUX_PINCM21 | `TFT_LCD_CS_PIN` | 片选，低有效 |
| BLK | PB16 | IOMUX_PINCM33 | `TFT_LCD_BLK_PIN` | 背光，默认高有效 |

PA31/PA28 是原理图可确认的显示口 SCL/SDA；PA30/PB15/PA10/PB16 是当前代码给 RES/DC/CS/BLK 分配的备用 IO 默认值，实物若已焊到别的 P1/P2 引脚，只改 `syscfg/ti_msp_dl_config.h` 的 `TFT_LCD_*` 宏即可。

## 编码器预留

| 电机 | 通道 | MSPM0G3507 | 建议外设 |
|---|---|---|---|
| Motor 2 | A/B | PA26 / PA27 | TIMG8_C0/C1 |
| Motor 1 | A/B | PA25 / PA14 | TIMG12_C1/C0 |

编码器接口在此骨架里先不启用。原因是 MG513xgmr 实际方向、减速比、线序和 QEI 极性需要上板确认。
