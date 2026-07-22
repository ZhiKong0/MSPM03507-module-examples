# 50 IMU 与姿态

状态：`source-index`

## 模块目的

整理航向保持和姿态输入：MPU6050、JY/IMU601、BNO 系列接口、Yaw 连续角处理、直线保持、转向控制和调试校准流程。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | MPU6050、Yaw controller、sensor_task 融合 | `porting-candidate` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | MPU6050 DMP、IMU601 UART、转弯 PID、引脚映射 | `runtime-proven` |
| `待整理的文件夹/github/Torris-Yin__mspm0-modules` | IMU 模块候选，待二次抽证据 | `source-index` |

## 证据路径

- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/README.md:9`：主控制路径包含 IMU、编码器和 line sensor 融合。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CMakeLists.txt:17` 和 `:43`：工程引入 `Drivers/MPU6050` 并收集 MPU6050 源码。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Sources/app_state.h:13` 到 `:24`：应用状态保存 IMU ready、stable、yaw、gyro 等字段。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Control/yaw_controller.c:50` 到 `:65`：Yaw controller 使用目标航向、当前航向、gyro_z 更新。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CLAUDE.md:155` 到 `:185`：记录 IMU 调试、方向和灵敏度校准流程。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:163` 和 `:164`：列出 MPU6050 与 IMU601 两种已验证 IMU 方案。

## 下一步

1. 拆出 `imu_pose_contract.md`：`ready`、`stable`、`yaw_deg`、`gyro_z_dps`、`uptime_ms`。
2. BNO085、MPU6050、JY61/IMU601 分成不同后端，不能和 OLED 绑定成一个物理接口。
3. 先用串口/日志验证 yaw 方向和 90 度比例，再把航向闭环接入底盘。

## 许可和构建备注

MPU DMP 相关代码常带第三方许可和较大内存开销。正式使用前要单独检查许可证、RAM 占用和是否适合 MSPM0G3507 的 32 KB SRAM。
