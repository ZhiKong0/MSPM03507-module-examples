# 模块分类体系

## 目标目录模型

后续从候选池正式拆模块时，建议在仓库根部另建正式区，例如 `整理后的模块/`。候选池仍保留原样，正式区按下面分类建立。

| 分类 | 目标内容 | 首批来源 |
|---|---|---|
| `00_official_baseline` | TI 官方空工程、GPIO、时钟、中断、SysConfig、DriverLib 最小样例 | `official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib` |
| `10_toolchain_templates` | CMake/Makefile、ARM GCC、pyOCD/probe-rs、VS Code/EIDE/CCS 兼容说明 | `github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template`、`github/Ctrl-CVCV__TI-EIDE-Example` |
| `20_board_bsp` | 时钟、GPIO、delay、interrupt、board pin map、SysConfig 生成边界 | 官方 SDK、`github/DQ103__mspm0-car-2024-h/SysConfig` |
| `30_motor_encoder` | TB6612/DRV8833/PWM、方向控制、编码器计数、速度 PI、里程 | `github/2262727886-stack__mspm0g-contest-skill`、`github/Comet966__MSPM0G3507_examples`、`github/DQ103__mspm0-car-2024-h` |
| `40_line_sensor` | 4/8 路灰度、红外循迹、线位估计、丢线处理 | `github/Comet966__MSPM0G3507_examples`、`github/military-doge__Grayscale-Line-Follower-Robot` |
| `50_imu_pose` | MPU6050、ICM、BNO、JY61/JY901、航向融合、转向控制 | `github/Torris-Yin__mspm0-modules`、`github/DQ103__mspm0-car-2024-h` |
| `60_display_ui` | OLED、LCD、按键/编码器菜单、状态页、调参页 | `github/2262727886-stack__mspm0g-contest-skill`、`github/Comet966__MSPM0G3507_examples` |
| `70_comm_tuning` | UART、BLE、VOFA、PID 调参协议、日志协议 | `github/2262727886-stack__mspm0g-contest-skill`、官方 UART 例程 |
| `80_vision_coprocessor` | K230、MaixCam、OpenMV、目标坐标协议、双芯通信 | `github/2262727886-stack__mspm0g-contest-skill`、`github/aluomx__Nuedc-code` |
| `90_competition_apps` | 2024H、2025E、2026 预测小车完整任务状态机 | `github/DQ103__mspm0-car-2024-h`、`github/JamieK32__ti-contest` |
| `99_reference_only` | GPL/CC BY-NC-SA/无许可证或题型特化内容 | 所有限制性来源 |

## 状态标签

- `source-only`：只完成来源导入和目录定位。
- `build-candidate`：结构适合编译验证，但尚未跑通本地构建。
- `porting-candidate`：可迁移思路明确，但依赖 Keil/CCS/SysConfig 或硬件环境。
- `runtime-proven`：本地硬件或上游明确实机验证，并有证据链。
- `license-restricted`：许可证要求限制后续合并方式。

## 评分规则

| 维度 | A | B | C |
|---|---|---|---|
| 题型贴合 | 直接服务控制题小车主线 | 可补某个模块 | 只作思路 |
| 工具链 | CMake/Makefile + GCC 清晰 | Keil/CCS 可转 | 构建入口不清 |
| 模块边界 | 驱动/控制/任务分层清楚 | 可拆但耦合较多 | 单文件或强题目绑定 |
| 验证证据 | 有实机/稳定 tag/README 说明 | 有运行描述 | 无验证描述 |
| 许可证 | MIT/BSD/Apache/TI 官方 | GPL/CC 限制 | 无声明 |

## 第一条拆解主线

第一轮不追求“大而全”，先建立一条能跑的差速底盘主线：

1. `00_official_baseline`：从官方 `gpio_toggle_output`、`uart_echo`、`timg_qei_mode`、`timx_timer_mode_pwm_*` 抽最小外设基准。
2. `10_toolchain_templates`：用 CMake/GCC 模板统一构建入口，参考 `Cxxhh` 模板。
3. `30_motor_encoder`：抽 TB6612 + 编码器 + 速度 PI。
4. `40_line_sensor`：抽 8 路灰度线位估计，先只输出 line position 和 confidence。
5. `60_display_ui` / `70_comm_tuning`：OLED 状态页 + UART/VOFA 调参。
6. `90_competition_apps`：把上面模块接成一个可测小车任务骨架。
