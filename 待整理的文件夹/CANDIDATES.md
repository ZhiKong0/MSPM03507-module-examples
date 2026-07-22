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
| C | TI2026_jy61p_car_temp_proj | https://github.com/Ykdzds/TI2026_jy61p_car_temp_proj | submodule | 未声明 | 2026 天猛星 MSPM0G3507 小车模板，JY61P、蓝牙 PID 调试 | 检查蓝牙在线 PID 调试协议 |
| C | mspm0g3507_project | https://github.com/Zlxwy/mspm0g3507_project | submodule | 未声明 | MSPM0G3507 工程集合 | 后续抽查结构 |
| C | TI_mspm0G3507_balance_car | https://github.com/huber2025/TI_mspm0G3507_balance_car | submodule | 未声明 | 2026 校赛 TI 主控平衡车 | 作为平衡车控制参考 |
| C | TI_MSPM0G3507_Trace_Tracking_Module | https://github.com/Adnis018/TI_MSPM0G3507_Trace_Tracking_Module | submodule | 未声明 | PID 巡线小车母版 | 对比巡线 PID 实现 |

## 第三批候选

| 优先级 | 名称 | 来源 | 导入方式 | 许可证 | 适配价值 | 下一步 |
|---|---|---|---|---|---|---|
| A | littlecar-mspm0g3507 | https://github.com/termcczec/littlecar-mspm0g3507 | submodule | 未声明 | MSPM0G3507 小车，含驱动测试和完整应用描述 | 扫描驱动测试用例和主控应用结构 |
| A | MSPM0_Car | https://github.com/gugugu531/MSPM0_Car | submodule | 未声明 | MSPM0G3507 小车，可导入 CCS ticlang 和 Keil5 | 对比 CCS/Keil 双工程组织方式 |
| A | mspm0g3507-irtracking-car | https://github.com/shu005/mspm0g3507-irtracking-car | submodule | 未声明 | 红外/灰度循迹小车候选 | 拆巡线传感器读取和控制逻辑 |
| A | M0_Car_Project | https://github.com/CheeeezBlue/M0_Car_Project | submodule | 未声明 | 天猛星 MSPM0G3507 电赛智能小车 | 抽查底盘和任务状态机 |
| A | TI-Cup-2024-ProblemH-AutoCar | https://github.com/bigandstone/TI-Cup-2024-ProblemH-AutoCar | submodule | 未声明 | 2024 TI 电赛 H 题自动驾驶小车，灰度+IMU 四任务 | 重点看四任务路径与 IMU 修正 |
| A | MSPM0G3507-Car | https://github.com/5ee511/MSPM0G3507-Car | submodule | 未声明 | 八路灰度直角弯巡线，TB6612、编码器速度 PI、巡线 PD | 适合拆成灰度、速度环、巡线 PD 模块 |
| B | Motor_Queen | https://github.com/Cttangg/Motor_Queen | submodule | 未声明 | TI MSPM0G3507 track tracking car | 对比小车底层结构 |
| B | Metal-wire-tracking-car-Based-on-mspm0g3507 | https://github.com/bigandstone/Metal-wire-tracking-car-Based-on-mspm0g3507 | submodule | 未声明 | MSPM0G3507 金属丝传感器循迹，参考 2016 TI 杯控制题 | 作为非黑线循迹题型参考 |
| B | MSPM0G3507_MOTORCONTROL | https://github.com/QwQuintOO/MSPM0G3507_MOTORCONTROL | submodule | 未声明 | MSPM0G3507 电机控制候选 | 抽取电机控制和 PWM/方向封装 |
| B | simple-standard-car | https://github.com/giaoLRL/simple-standard-car | submodule | 未声明 | 八路灰度循迹小车项目 | 对比八路灰度标准车实现 |
| B | mspm0g3507-modules | https://github.com/glf-loser/mspm0g3507-modules | submodule | 未声明 | JY61P IMU 与 Track Motor 控制模块 | 拆 JY61P 姿态和电机控制接口 |
| B | TMX_MSPM0G3507_ProjectTemplate | https://github.com/persica123/TMX_MSPM0G3507_ProjectTemplate | submodule | 未声明 | MSPM0G3507 智能车工程模板 | 作为天猛星工程模板候选 |
| B | xunjixiaoche | https://github.com/juliantreee/xunjixiaoche | submodule | 未声明 | MSPM0G3507 line follower | 作为简化巡线小车参考 |
| B | MSPM0G3507_examples | https://github.com/Comet966/MSPM0G3507_examples | submodule | 未声明 | LP-MSPM0G3507 示例集合 | 用于补外设基础例程 |
| B | MSPM0_Driver_Lib | https://github.com/dzzz-qcxf-studio/MSPM0_Driver_Lib | submodule | 未声明 | 从实际跑通电赛项目提取的天猛星模块化驱动库 | 优先扫描驱动目录和已验证模块 |
| B | er2024 | https://github.com/zhujs1103/er2024 | submodule | 未声明 | ER2024 电赛 MSPM0G3507 巡线+作业一体化平台 | 参考项目管理和平台化结构 |

## 待重试/待抽查候选

| 优先级 | 名称 | 来源 | 状态 | 适配价值 | 下一步 |
|---|---|---|---|---|---|
| B | MSPM0G3507_syscfg_freeRTOS | https://github.com/WYJxxoo/MSPM0G3507_syscfg_freeRTOS | 待重试 | 电机控制、PID、编码器、循迹、FreeRTOS 示例 | 多次 GitHub 连接失败，保留重试 |
| B | 25xiaoche | https://github.com/xieyangyingshutong/25xiaoche | 待重试 | 2025 电赛小车大部分功能，Keil + SysConfig | 多次 GitHub 连接失败，保留重试 |
| B | nuedc_control_car_freertos_template | https://github.com/xy1092/nuedc_control_car_freertos_template | submodule | Clean calibrated MSPM0G3507 FreeRTOS control car template | 对比现有 `nuedc_control_freertos` |
| B | TI-Cup-School-Competitions | https://github.com/bigandstone/TI-Cup-School-Competitions | submodule | TI 电赛校级选拔赛历届赛题代码合集 | 拆出控制题相关工程 |
| B | Grayscale-Line-Follower-Robot | https://github.com/military-doge/Grayscale-Line-Follower-Robot | submodule | MSPM0G3507 灰度循迹小车 | 对比灰度巡线和底盘控制 |
| B | Find_box_car | https://github.com/Students800/Find_box_car | source-card | OpenMV + MSPM0G3507 视觉追踪 | 仓库为空，暂不作为 submodule |
| B | Nuedc2026-MSPM0G3507-Vehicle | https://github.com/chenjingquan-hntb/Nuedc2026-MSPM0G3507-Vehicle | submodule | 2026 NUEDC MSPM0G3507 Vehicle Project | 作为 2026 小车候选 |
| B | mspm0g3507-tracking-target-car | https://github.com/asdfguai/mspm0g3507-tracking-target-car | submodule | 面向 2025 E 题跟踪目标小车 | 抽查视觉/靶标跟踪逻辑 |
| B | MSPM0-keil-project | https://github.com/iwiwb/MSPM0-keil-project | submodule | 速度闭环、角度闭环、OLED、红外循迹 | 已重试成功，后续拆闭环/OLED/循迹模块 |
| C | four-drive-yabo-sensor | https://github.com/likechen0/four-drive-yabo-sensor | submodule | 四驱小车，亚博 8 路灰度传感器 | 后续按四驱需求整理 |
| C | MSPM0G3507_Template | https://github.com/QwQuintOO/MSPM0G3507_Template | submodule | VSCode GCC + EIDE + OpenOCD + SysConfig 模板 | 已重试成功，可用于工具链模板对比 |

## 第四批候选

| 优先级 | 名称 | 来源 | 导入方式 | 许可证 | 适配价值 | 下一步 |
|---|---|---|---|---|---|---|
| A | mspm0g3507-line-tracking-car_v1.0 | https://github.com/xujh666/mspm0g3507-line-tracking-car_v1.0 | submodule | 未声明 | 基于 MSPM0G3507 的循迹小车项目 | 对比 v1.0 巡线结构 |
| A | mspm0g3507-line-tracking-car | https://github.com/xujh666/mspm0g3507-line-tracking-car | submodule | 未声明 | MSPM0G3507 八路循迹小车 | 和 v1.0 版本比较差异 |
| A | my-car-electronic-contest-prep | https://github.com/chowhound-eat/my-car-electronic-contest-prep | submodule | 未声明 | line patrol + incremental PID + MSPM0G3507 | 拆增量 PID 和巡线主循环 |
| A | mspm0g3507-smart-car | https://github.com/Yang-runze/mspm0g3507-smart-car | submodule | 未声明 | MSPM0G3507 smart car 候选 | 抽查底盘和传感器模块 |
| A | MOTOR_PID_25H | https://github.com/summer-xiying/MOTOR_PID_25H | submodule | 未声明 | MSPM0G3507 Motor PID Control Project，25H Smart Car | 重点看电机 PID 和编码器反馈 |
| B | 25-e- | https://github.com/dawn-qvq/25-e- | submodule | 未声明 | 2025 NUEDC 简易自行瞄准装置，STM32 + MSPM0G3507，视觉、二维云台、PID | 作为视觉/云台题型参考 |
| B | Nuedc-code | https://github.com/aluomx/Nuedc-code | submodule | 未声明 | MSPM0G3507 + K230 备赛代码记录 | 补 K230 视觉接口候选 |
| B | mspm0g3507-imu-lcd | https://github.com/Eliauk-Camelia/mspm0g3507-imu-lcd | submodule | 未声明 | MPU6050 姿态、ST7735S LCD、灰度传感器 | 补 IMU/LCD/灰度基础模块 |
| C | mspm0G3507cartest_record | https://github.com/kokona23/mspm0G3507cartest_record | 待重试 | 未声明 | 四电机循迹小车开发项目 | 本轮 GitHub 连接重置 |
| C | TI_MSPM0G3507_Project | https://github.com/sbjsw334/TI_MSPM0G3507_Project | 待重试 | 未声明 | 状态机五段电赛控制类项目 | 本轮 clone 超时后已清理半成目录 |
| C | TMC2209_StepMotor_Driver | https://github.com/Tacrine/TMC2209_StepMotor_Driver | 待导入 | 未声明 | TMC2209 步进电机驱动，支持 MSPM0G3507/STM32 | 后续按步进电机需求导入 |

## 第五批候选
| 优先级 | 名称 | 来源 | 导入方式 | 许可证 | 适配价值 | 下一步 |
|---|---|---|---|---|---|---|
| A | -MSPM0G3507_car_control | https://github.com/ice-cream-1024/-MSPM0G3507_car_control | submodule | 未声明 | MSPM0G3507 小车/云台控制候选，仓库存在 `ELC_gimbal` 等分支线索 | 抽查底盘、云台、传感器接口是否可拆模块 |
| B | mspm0g3507_template | https://github.com/xiaoshuai361/mspm0g3507_template | submodule | 未声明 | 近期更新的 MSPM0G3507 工程模板 | 对比 SysConfig、Keil/CCS 或 GCC 工程组织 |
| B | m0template | https://github.com/Huai-jin-6/m0template | submodule | 未声明 | MSPM0G3507 FreeRTOS project template | 对比任务划分、启动文件、RTOS 配置 |
| B | MSPM0G3507-System-Library | https://github.com/Almighty42/MSPM0G3507-System-Library | submodule | 未声明 | MSPM0G3507 系统库/基础封装候选 | 抽查外设驱动和目录完整性 |
| B | 26_TI_MSPM0G3507 | https://github.com/ergouzi332/26_TI_MSPM0G3507 | submodule | 未声明 | 2026/TI/MSPM0G3507 相关近期项目 | 判断是否包含电赛控制题模块 |
| C | MSPM0G3507_BASIC- | https://github.com/jeffrey070318/MSPM0G3507_BASIC- | 待重试 | 未声明 | 包含 CMake、FreeRTOS、BSP、motor、pid、oled、gray_sensor、vofa 等强相关目录，但本轮在 Windows 子模块导入后出现 git-lfs/索引异常 | 后续换浅克隆或独立路径重试，成功前不提交 submodule |
| C | MSPM0G3507-AutoCar | https://github.com/686my686/MSPM0G3507-AutoCar | 待重试 | 未声明 | MSPM0G3507 AutoCar 候选 | 本轮 clone 超时，后续单仓重试 |
| C | MSPM0G3507_VSCODE | https://github.com/RitsuYi/MSPM0G3507_VSCODE | 待重试 | 未声明 | 基于 VSCode 的 MSPM0G3507 项目 | 本轮 GitHub 连接重置，后续单仓重试 |
| C | MSPM0G3507_FreeRTOS | https://github.com/Ju4tdo1/MSPM0G3507_FreeRTOS | 待重试 | 未声明 | MSPM0G3507 FreeRTOS 工程 | 本轮 GitHub 连接重置，后续单仓重试 |

## 第六批候选
| 优先级 | 名称 | 来源 | 导入方式 | 许可证 | 适配价值 | 下一步 |
|---|---|---|---|---|---|---|
| B | NUEDC_2025 | https://github.com/VincentttWang/NUEDC_2025 | source-card | 未声明 | 2025 全国大学生电子设计大赛 E 题仓库；code search 命中 `2025E/工程文件/BSP/Motor_Encoder/TB6612FNG_README.md` | 本轮连接重置，后续重试 submodule；优先抽 TB6612/编码器目录 |
| B | EDC_ti | https://github.com/lxl2024666/EDC_ti | source-card | 未声明 | code search 命中 `Template/BSP/Motor_Encoder/TB6612FNG_README.md`，可作为 TB6612/编码器工程模板候选 | 本轮普通 clone 与浅克隆均连接重置，后续重试 |

## 后续搜索方向

- MSPM0G3507 + TB6612 / DRV8833 / PWM / encoder
- MSPM0G3507 + 灰度巡线 / line follower / 2024 H
- MSPM0G3507 + ICM42688 / MPU6050 / BNO085 / JY901S
- MSPM0G3507 + OLED 菜单 / UART 蓝牙调参
- MSPM0G3507 + K230 / MaixCam / OpenMV 视觉接口
- MSPM0G3507 + CMake / GCC / pyOCD / probe-rs
