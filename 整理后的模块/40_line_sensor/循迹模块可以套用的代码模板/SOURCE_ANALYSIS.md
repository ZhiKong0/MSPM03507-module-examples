# 现成循迹代码分析

## 结论

本地候选池里已经有可用的现成循迹代码。最适合抽模板的是 DQ103 的模块化分层，加上 Adnis018 的最小加权误差思路、zby 的 8 路 GPIO 读取示例、2262727886 car-kit 的丢线搜索和 PWM 斜坡经验。

本模板采用“重写实现”：保留接口思想和算法结构，不直接复制第三方源码。

## 候选 1：DQ103/mspm0-car-2024-h

本地路径：`待整理的文件夹/github/DQ103__mspm0-car-2024-h`

上游：`https://github.com/DQ103/mspm0-car-2024-h.git`

许可证：MIT，本地 `LICENSE` 已确认。

证据：

- `Drivers/LineTracker/linetracker.c:114`：`LineTracker_Init()` 初始化 7 路传感器。
- `Drivers/LineTracker/linetracker.c:134` 到 `:163`：读取原始 GPIO，按黑线电平转换成传感器位图。
- `Drivers/LineTracker/linetracker.c:188` 到 `:208`：用权重加权平均计算线位置。
- `Drivers/LineTracker/linetracker.c:218` 到 `:276`：根据位图和线位置判断在线、左转、右转、丢线等状态。
- `Control/line_controller.c:30` 到 `:42`：控制层只吃 `line_error`，检测不到线时输出 0。

可取之处：

- 驱动、设备对象、控制器边界清楚。
- 线位置输出独立，不和电机 PWM 直接耦合。
- 支持黑线高/低电平和通道反序。

不足：

- 引脚仍绑定具体 SysConfig 宏。
- `LineController_Update()` 丢线时直接输出 0，需要更完整的搜索策略。

## 候选 2：Adnis018/TI_MSPM0G3507_Trace_Tracking_Module

本地路径：`待整理的文件夹/github/Adnis018__TI_MSPM0G3507_Trace_Tracking_Module`

上游：`https://github.com/Adnis018/TI_MSPM0G3507_Trace_Tracking_Module.git`

许可证：未在本地发现 LICENSE，正式区不直接复制源码。

证据：

- `Tracker.h:13`：定义 8 通道。
- `Tracker.h:18`：默认权重为 `-16,-9,-4,-1,1,4,9,16`。
- `Tracker.c:25` 到 `:39`：读取 8 路 GPIO 位图。
- `Tracker.c:42` 到 `:63`：计算加权中心误差，并用 sentinel 表示脱线。

可取之处：

- 小而清楚，适合做“线位估计最小版”。
- 权重可外部传入。

不足：

- 全灭和全亮都直接判脱线，遇到起跑线、终点线、十字线时需要上层再区分。
- 直接绑定 GPIOB 和 SysConfig 宏。

## 候选 3：zby123581-maker/mspm0g3507-tracking-car

本地路径：`待整理的文件夹/github/zby123581-maker__mspm0g3507-tracking-car`

上游：`https://github.com/zby123581-maker/mspm0g3507-tracking-car.git`

许可证：本地未发现 LICENSE，正式区不直接复制源码。

证据：

- `gray_sensor/gray_sensor.c:32` 到 `:41`：读 8 路 GPIO，高电平白色，低电平黑线。
- `xunji/xunji.c:24` 到 `:65`：根据黑线左右边缘计算 `line_position` 和 `line_deviation`。
- `xunji/xunji.c:68` 到 `:101`：把偏差送入转向 PID，并做弯道减速。
- `xunji/xunji.c:104` 到 `:176`：提供丢线检查和左右搜索状态机。

可取之处：

- 从原始灰度到差速控制链路完整。
- 边缘中点算法对宽黑线比较直观。

不足：

- `lost_time` 留了 TODO，不能直接作为可靠状态机。
- 电机、PID、灰度读数耦合较紧。

## 候选 4：2262727886-stack/mspm0g3507-car-kit

本地路径：`待整理的文件夹/github/2262727886-stack__mspm0g3507-car-kit`

上游：`https://github.com/2262727886-stack/mspm0g3507-car-kit.git`

许可证：本地未发现 LICENSE，正式区不直接复制源码。

证据：

- `imu601/empty.c:154` 到 `:171`：8 路循迹 GPIO 输入初始化。
- `imu601/empty.c:173` 到 `:187`：active-low 方式读取 8 路位图。
- `imu601/empty.c:340` 到 `:355`：用 `-35..35` 权重计算误差。
- `imu601/empty.c:488` 到 `:525`：有效线位时差速控制，丢线时按最后方向搜索，并用斜坡更新 PWM。

可取之处：

- 丢线搜索策略很实用。
- 控制输出有斜坡，适合真实小车减少突变。

不足：

- 单文件工程，和 IMU、OLED、按键、电机强耦合。
- 需要抽象后才能复用。

## 模板取舍

采用：

- DQ103 的模块边界。
- Adnis018/DQ103 的加权平均线位算法。
- zby 的边缘/宽线意识，但不把它作为唯一算法。
- car-kit 的最后方向搜索和斜坡输出。

不采用：

- 固定 GPIOB/固定 SysConfig 宏。
- 全白时把偏差直接清零。
- 控制器直接操作电机。
- 未确认许可证来源的源码复制。
