# 来源盘点

更新时间：2026-07-22

## 当前规模

| 来源层 | 目录数 | 文件数 | 体积 | 说明 |
|---|---:|---:|---:|---|
| `github/` | 75 | 54,396 | 约 3.69 GB | 主要是小车工程、模块库、工程模板、视觉/调参方案，以 submodule 保存 |
| `gitee/` | 3 | 2,764 | 约 121 MB | Gitee 小车/例程候选，以 submodule 保存 |
| `gitcode/` | 1 | 3 | 约 564 KB | GitCode 候选入口 |
| `official_ti_sdk/` | 1 | 6,809 | 约 83.4 MB | TI MSPM0 SDK 2.11.00.07 的 LP_MSPM0G3507 相关候选 |

## 文件类型观察

| 类型 | 命中数 | 处理判断 |
|---|---:|---|
| `.c` | 15,391 | 主体源码 |
| `.h` | 13,265 | 主体头文件 |
| `.syscfg` | 765 | MSPM0/SysConfig 配置，后续需要对照生成代码 |
| `.projectspec` | 1,040 | CCS/Theia 工程导入入口 |
| `.uvprojx/.uvoptx/.sct` | 多 | Keil 工程参考，不作为主线验收 |
| `.o/.d/.a/.map/.out/.elf/.hex/.bin` | 多 | 多数来自第三方上游仓库内部，后续正式拆模块时不得带入 |

## 官方 SDK 内容

TI 官方 SDK 导入说明见 `official_ti_sdk/mspm0_sdk_2_11_00_07/README.md:7` 和 `official_ti_sdk/mspm0_sdk_2_11_00_07/README.md:16`。

| 官方 SDK 分类 | 文件数 | 价值 |
|---|---:|---|
| `driverlib` | 3,413 | GPIO/UART/I2C/SPI/ADC/Timer/QEI 等官方外设例程核心 |
| `msp_subsystems` | 734 | ADC、PWM、桥接、滤波、调度等组合例程 |
| `demos` | 366 | out_of_box、角度编码器、通信和器件控制 demo |
| `bsl` | 233 | BSL 主机/接口/二级 BSL，和后续恢复烧录直接相关 |
| `edgeAI` | 130 | AI demo，只作为特定题型参考 |
| `motor_control_*` | 165 | BLDC/PMSM/FOC 等电机控制参考，不直接替代 TB6612 小车底盘主线 |
| `rtos/LP_MSPM0G3507` | 168 | FreeRTOS 驱动例程和任务模板 |

## 第一批高价值第三方来源

| 候选 | 角色 | 证据 | 第一用途 |
|---|---|---|---|
| `github/DQ103__mspm0-car-2024-h` | `competition-app` | README 标明 `MSPM0G3507 + FreeRTOS`、IMU+encoder+line sensor fusion，见 `github/DQ103__mspm0-car-2024-h/README.md:8`、`github/DQ103__mspm0-car-2024-h/README.md:9` | 抽 FreeRTOS 任务分层、底盘闭环、赛题状态机 |
| `github/2262727886-stack__mspm0g-contest-skill` | `competition-app` + `module-reference` | README 标明 MSPM0G3507+K230 全栈、实机验证，见 `github/2262727886-stack__mspm0g-contest-skill/README.md:3` | 抽 TB6612、编码器、PID、OLED、蓝牙、K230 串口协议 |
| `github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template` | `toolchain-template` | README 标明 CMake、ARM GCC、Ninja、Probe-rs，见 `github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/README.md:3` | 作为后续 CMake/GNU 统一工程模板参考 |
| `github/Torris-Yin__mspm0-modules` | `module-reference` | 模板入口包含 MPU6050、OLED、IMU660RB 初始化位点，见 `github/Torris-Yin__mspm0-modules/Template/main.c:41` | 抽常用外设驱动接口风格 |
| `github/Comet966__MSPM0G3507_examples` | `module-reference` | 含编码器、TB6612、OLED、K230 和灰度文档命中，见 `github/Comet966__MSPM0G3507_examples/mg513/user/headfile.h:20`、`github/Comet966__MSPM0G3507_examples/Four_way_patrol_line_CCS/八路灰度传感器手册要点.md:52` | 抽电机/编码器诊断、灰度接线与 OLED 显示经验 |

## 风险

- 第三方候选内存在构建残留和二进制文件；父仓库以 submodule 保存时可以保留来源，但正式拆模块时必须过滤。
- 部分第三方代码没有明确许可证，只能作为学习候选。
- 部分源码注释存在编码损坏迹象，正式整理时应以功能和工程验证为准。
- 官方 SDK 的 gcc makefile 仍依赖完整本地 SDK 的 `imports.mak`、SysConfig 和预编译 `driverlib.a`；这已在官方 SDK README 中说明，见 `official_ti_sdk/mspm0_sdk_2_11_00_07/README.md:24`。
