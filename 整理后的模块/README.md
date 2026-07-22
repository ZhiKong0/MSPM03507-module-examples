# 整理后的模块

状态：`source-index`

这里是从 `待整理的文件夹/` 候选池进入正式整理路线的索引区。当前阶段先不复制第三方源码，只建立模块边界、候选来源和证据链；后续每个模块通过编译、烧录和实机验证后，再进入 `portable` 或 `ready` 状态。

## 整理主线

第一轮按“能跑起来的小车最小闭环”拆：

1. `00_official_baseline`：TI 官方 GPIO、UART、PWM、QEI、SysConfig 最小基准。
2. `10_toolchain_templates`：CMake/Makefile + ARM GCC + pyOCD/probe-rs 的统一构建模板。
3. `20_board_bsp`：板级 pin map、时钟、中断、SysConfig 生成边界。
4. `30_motor_encoder`：TB6612/PWM、编码器、速度环、差速底盘。
5. `40_line_sensor`：灰度/红外循迹、线位置、置信度、丢线处理。
6. `50_imu_pose`：MPU6050/JY/IMU 航向、Yaw 控制、直线保持。
7. `60_display_ui`：OLED、状态页、按键或调参页面。
8. `70_comm_tuning`：UART、BLE、VOFA、PID 调参协议。
9. `80_vision_coprocessor`：K230/OpenMV/MaixCam 双芯视觉协议。
10. `90_competition_apps`：2024H、2025E、2026 预测视觉导航小车应用骨架。
11. `99_reference_only`：许可证或来源限制较强的参考资料。

## 状态标签

- `source-index`：已经定位来源和证据行，尚未复制源码。
- `porting-candidate`：接口和逻辑适合重写或迁移。
- `build-candidate`：有清晰 CMake/Makefile/GCC 构建入口。
- `runtime-proven`：本地或上游 README 有实机运行证据。
- `license-restricted`：只能参考思想，不能直接合并源码。

## 当前约束

- 正式区优先服务电赛控制题小车：差速底盘、编码器、灰度巡线、IMU 航向、OLED、串口调参、视觉协处理器。
- 主验收路径是 `CMake/Makefile + arm-none-eabi + pyOCD/DAPLink`。
- Keil、CCS、SysConfig 工程作为迁移来源和引脚/初始化证据，不作为第一验收路径。
- 对无许可证、GPL 或非商业限制来源，先放 `99_reference_only` 或只抽接口思想，避免直接复制合并。
