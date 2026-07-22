# TI MSPM0 SDK 2.11.00.07 官方例程候选

来源：`F:\QQchat_Files\mspm0_sdk_2_11_00_07.zip`

导入日期：2026-07-22

## 导入范围

- `examples/nortos/LP_MSPM0G3507/`：LP_MSPM0G3507 NoRTOS 官方例程。
- `examples/rtos/LP_MSPM0G3507/`：LP_MSPM0G3507 RTOS 官方例程。
- `source/ti/driverlib/`：TI DriverLib 源码/头文件，已排除预编译库。
- `source/ti/devices/msp/`：MSP 设备寄存器和启动相关头文件。
- `source/third_party/CMSIS/Core/Include/`：CMSIS Core 头文件。
- 顶层官方文档：license、manifest、release notes、known issues、`.metadata/product.json`。

## 排除范围

- 未导入完整 SDK 压缩包。
- 未导入 `.a`、`.bin`、`.out`、`.elf`、`.hex`、`.obj`、`.map` 等预编译库、镜像和构建产物。
- 未导入完整 `.metadata/.tirex` 依赖缓存。
- 未导入工具安装包、完整文档树和无关开发板例程。

## 使用说明

这里是“待整理官方候选集”，适合查外设初始化、SysConfig 配置、GCC/Keil/IAR/CCS 工程组织、以及后续拆成电赛模块。

官方 `gcc/makefile` 通常依赖 SDK 根目录的 `imports.mak`、SysConfig、工具链路径和预编译 `driverlib.a`。本仓库为避免提交二进制和整包 SDK，没有包含这些构建依赖；如果要直接编译，请安装同版 TI MSPM0 SDK `2.11.00.07`，并在本地 SDK 环境中配置 `imports.mak`、SysConfig 和 ARM GNU 工具链。

## 价值标注

- 优先级：A
- 导入方式：vendored-copy
- 许可证：TI 官方 SDK 许可，见 `license_mspm0_sdk_2_11_00_07.txt` 和 `manifest_mspm0_sdk_2_11_00_07.html`
- 下一步：从 `driverlib`、`msp_subsystems`、`demos/out_of_box`、`bsl`、`motor_control_*` 中拆出 GPIO/UART/I2C/SPI/PWM/QEI/ADC/BSL/电机控制等基础模块候选。
