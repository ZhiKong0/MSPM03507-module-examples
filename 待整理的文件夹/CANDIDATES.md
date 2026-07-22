# MSPM0G3507 电赛候选代码清单

更新时间：2026-07-22

## 首批优先候选

| 优先级 | 名称 | 来源 | 导入方式 | 许可证 | 适配价值 | 下一步 |
|---|---|---|---|---|---|---|
| A | PIDCarTemplate-MSPM0G3507 | https://github.com/menoking/PIDCarTemplate-MSPM0G3507 | submodule | 未声明 | 两/四轮 PID 小车模板，适合拆底盘闭环和运动控制骨架 | 检查 PWM、编码器、PID、SysConfig/工程结构 |
| A | mspm0g3507-car-kit | https://github.com/2262727886-stack/mspm0g3507-car-kit | submodule | 未声明 | 电赛智能小车开发套件，含底盘、IMU、PID、K230视觉追踪、蓝牙遥控 | 优先看模块分层和 K230/蓝牙接口 |
| A | 2024-NUEDC-H-TI_CAR | https://github.com/abcuer/2024-NUEDC-H-TI_CAR | submodule | 未声明 | 2024 H 题惯性导航和巡线自动行驶小车，题型贴近控制主线 | 检查灰度、IMU、路径状态机 |
| A | SmartPatrol | https://github.com/XinDevs/SmartPatrol | submodule | 未声明 | 双轮差速、灰度巡线、ICM42688/Mahony、串级 PID、MaixCam、OLED、串口调参 | 重点拆 IMU 航向、视觉巡检、在线调参 |
| A | Electric-Race-Control | https://github.com/Kuriharamio/Electric-Race-Control | submodule | MIT | MSPM0G3507 电赛控制题模块化框架 | 可作为正式整理架构参考 |
| B | mspm0-modules | https://github.com/Torris-Yin/mspm0-modules | submodule | 未声明 | MSPM0 常用模块驱动集合，含 OLED、MPU6050、BNO08x、VL53L0X 等 | 拆外设驱动适配层 |
| B | ZLC_MSPM0_Peripheral_Library | https://github.com/ZhijianLi2003/ZLC_MSPM0_Peripheral_Library | submodule | GPL-3.0 | 2024 电赛 H 题一等奖相关 MSPM0 外设库 | 学习结构，后续合并需注意 GPL |
| B | Ti_MSPM0G3507_Car | https://github.com/superbusycool/Ti_MSPM0G3507_Car | submodule | 未声明 | 电赛小车代码，含 OLED、MPU6050、编码器中断、巡线逻辑 | 作为模块参考，不直接信任巡线效果 |
| B | 2024-dian-sai-h-line-follower | https://github.com/kaka12331/2024-dian-sai-h-line-follower | submodule | 未声明 | 2024 H 循迹小车，灰度、编码器 PID、陀螺仪盲走 | 对比 2024 H 解法 |
| B | 2025-NUEDC-E-Ti_CAR | https://github.com/abcuer/2025-NUEDC-E-Ti_CAR | submodule | 未声明 | 2025 E 自瞄云台小车，含小车和云台题型思路 | 拆云台/视觉/执行机构模式 |
| B | TI-MSPM0G3507 | https://github.com/danshoujieyi/TI-MSPM0G3507 | submodule | 未声明 | 裸机、FreeRTOS、RT-Thread 工程模板 | 仅作 RTOS/工程模板参考 |
| B | MSPM0G3507-CMAKE-GCC-Template | https://github.com/Cxxhh/MSPM0G3507-CMAKE-GCC-Template | submodule | 未声明 | CMake + ARM GCC 裸机模板，贴合本地首选工具链 | 可作为后续统一构建底座参考 |
| C | WeActStudio.MSPM0G3507CoreBoard | https://github.com/WeActStudio/WeActStudio.MSPM0G3507CoreBoard | source-card | 未确认 | WeAct MSPM0G3507 核心板资料和例程入口 | 确认是否有可直接复用例程 |
| C | g3507_framework | https://github.com/Bees-creation/g3507_framework | source-card | 未确认 | MSPM0G3507 代码框架，星标少但更新新 | 后续抽查结构 |

## 第二批候选

| 优先级 | 名称 | 来源 | 导入方式 | 许可证 | 适配价值 | 下一步 |
|---|---|---|---|---|---|---|
| A | mspm0g3507-smart-car | https://github.com/Eliauk-Camelia/mspm0g3507-smart-car | submodule | 未声明 | 轮趣 WHEELTEC MSPM0G3507 小车，含 MPU6050 DMP、TB6612、PI 闭环、OLED、灰度巡线 | 对比电机闭环和 MPU6050 初始化 |
| A | nuedc_control_freertos | https://github.com/xy1092/nuedc_control_freertos | submodule | 未声明 | FreeRTOS 健康监测、ICM45688 标定、模块化 NUEDC 小车固件 | 拆任务划分和健康监测 |
| B | MSPM0G3507_for_car | https://github.com/LieWill/MSPM0G3507_for_car | submodule | 未声明 | MSPM0G3507 小车工程 | 抽查驱动完整性 |
| B | SeekFree_MSPM0G3507_Opensource_Library | https://github.com/woai66/SeekFree_MSPM0G3507_Opensource_Library | submodule | 未声明 | 2025 TI 板电赛工程/逐飞库方向 | 检查可复用驱动和工程结构 |
| B | ros2car_mspm0 | https://github.com/Pansamic/ros2car_mspm0 | submodule | 未声明 | MSPM0G3507 直流电机小车驱动，可能适合上位机/ROS2 接口 | 看串口协议和底盘控制 |
| B | Dual-vehicle-Collaborative-Intelligent-Medicine-Delivery-Cart | https://github.com/CXCXHH/Dual-vehicle-Collaborative-Intelligent-Medicine-Delivery-Cart | submodule | 未声明 | 双车协同、K230/MaixCam、PID、编码器、灰度、ZigBee，参考 2021 F 题 | 作为双车协同扩展候选 |
| C | TI2026_jy61p_car_temp_proj | https://github.com/Ykdzds/TI2026_jy61p_car_temp_proj | 待重试 | 未声明 | 2026 天猛星 MSPM0G3507 小车模板，JY61P、蓝牙 PID 调试 | GitHub 连接超时，后续重试导入 |
| C | mspm0g3507_project | https://github.com/Zlxwy/mspm0g3507_project | submodule | 未声明 | MSPM0G3507 工程集合 | 后续抽查结构 |
| C | TI_mspm0G3507_balance_car | https://github.com/huber2025/TI_mspm0G3507_balance_car | submodule | 未声明 | 2026 校赛 TI 主控平衡车 | 作为平衡车控制参考 |
| C | TI_MSPM0G3507_Trace_Tracking_Module | https://github.com/Adnis018/TI_MSPM0G3507_Trace_Tracking_Module | submodule | 未声明 | PID 巡线小车母版 | 对比巡线 PID 实现 |

## 后续搜索方向

- MSPM0G3507 + TB6612 / DRV8833 / PWM / encoder
- MSPM0G3507 + 灰度巡线 / line follower / 2024 H
- MSPM0G3507 + ICM42688 / MPU6050 / BNO085 / JY901S
- MSPM0G3507 + OLED 菜单 / UART 蓝牙调参
- MSPM0G3507 + K230 / MaixCam / OpenMV 视觉接口
- MSPM0G3507 + CMake / GCC / pyOCD / probe-rs
