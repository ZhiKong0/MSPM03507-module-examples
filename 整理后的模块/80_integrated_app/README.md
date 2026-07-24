# 80 多模块集成上板工程

状态：`build-candidate`

这里放已经跨过单模块阶段、开始把 BSP、循迹、IMU、OLED、电机驱动和工具链接成同一个可编译工程的模板。

当前首个工程：

- `2026_天猛星车载最小上板工程骨架`：MSPM0G3507 + 天猛星拓展板 + UART0/JY61P + 预留八路循迹 + TB6612 + MG513xgmr + 128x160 RGB TFT LCD。

验收优先级：

1. 能用 ARM GCC 从仓库内 SDK 源码编译出 fresh ELF/HEX/BIN。
2. 上电默认 DISARMED，电机 PWM 和方向脚归零。
3. 通过 SWD/UART/LCD 启动图案逐步确认 JY61P、当前 TX/RX 串口模块、TFT LCD 和电机接口。
4. 确认方向、极性和编码器后，再升级成闭环运行工程。
