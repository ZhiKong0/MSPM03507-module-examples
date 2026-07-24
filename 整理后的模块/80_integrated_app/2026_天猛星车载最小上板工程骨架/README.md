# 2026 天猛星车载最小上板工程骨架

状态：`build-candidate`

目标是把已经整理出的模块接成一个真实 MSPM0G3507/GNU 工程骨架，先能编译、能上板安全启动、能读到关键传感器状态，再逐步进入闭环控制。

## 接入范围

已接入：

- MCU：MSPM0G3507，32MHz SYSOSC。
- IMU：JY61P，UART0，PA0/PA1，115200 8N1。
- 循迹：8 路 TX/RX 数字输入，PB25/PB18/PB21/PB0/PB1/PB10/PB11/PB14。
- 电机驱动：TB6612，TIMG0 PA12/PA13 双 PWM，方向脚 PB17/PB19/PA16/PB24。
- 电机：MG513xgmr，首版按双路直流减速电机接口预留，编码器方向待上板确认。
- OLED：I2C0，PA31 SCL，PA28 SDA，默认地址 0x3C，首版做在线探测。
- 复用模块：`20_board_bsp/2026_天猛星拓展板例程模板` 和 `40_line_sensor/循迹模块可以套用的代码模板`。

默认安全策略：

- 上电后 `BoardApp_Init()` 立即 DISARM。
- `main()` 不会自动 arm motor，也不会输出非零 PWM。
- 循迹控制器会计算遥测，但默认不把速度命令写到电机。
- 要真实跑车，必须先完成引脚、电机方向、编码器和循迹极性验证，再显式打开 `BOARD_APP_ENABLE_MOTOR_OUTPUT` 并添加动作入口。

## 构建

```powershell
Push-Location -LiteralPath 'E:\Learning\嵌入式\电赛备赛\MSPM03507各模块例程代码\整理后的模块\80_integrated_app\2026_天猛星车载最小上板工程骨架'
mingw32-make -f Makefile.mspm0g3507 all
Pop-Location
```

输出：

- `build/tmx_car_minimal/tmx_car_minimal.elf`
- `build/tmx_car_minimal/tmx_car_minimal.hex`
- `build/tmx_car_minimal/tmx_car_minimal.bin`
- `build/tmx_car_minimal/tmx_car_minimal.symbols.txt`

## 下一步验证顺序

1. 只烧录这个安全骨架，确认电机不动。
2. 读 `g_board_app_runtime` 和 `g_board_app_line_snapshot`，确认主循环计数增加。
3. 接 JY61P，确认 `jy61p_angle_frames` 增加，yaw/pitch/roll 有变化。
4. 接八路 TX/RX，观察 raw/active bits 与黑线位置是否一致。
5. 接 OLED，观察 `oled_probe_ok` 是否为 1。
6. 电机悬空，单独验证 TB6612 A/B 方向和 PWM 极性，再进入闭环。

详细引脚见 `docs/PIN_ASSIGNMENT.md`，构建和烧录边界见 `docs/BUILD_AND_FLASH.md`。
