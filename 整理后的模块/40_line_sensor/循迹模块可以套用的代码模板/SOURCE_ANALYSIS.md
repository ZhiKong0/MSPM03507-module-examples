# 现成巡线代码分析

## 结论

本地候选池里已经有可参考的 MSPM0G3507 巡线小车代码。这个模板采用“结构取舍 + 重新实现”的方式：保留模块边界、加权误差、丢线搜索、斜坡输出这些通用工程思想，不直接复制许可证不清晰的第三方源码。

## 候选 1：DQ103/mspm0-car-2024-h

本地路径：`待整理的文件夹/github/DQ103__mspm0-car-2024-h`

上游：`https://github.com/DQ103/mspm0-car-2024-h.git`

许可证：本地曾确认 MIT。

可取之处：

- 驱动、设备对象、控制器边界比较清楚。
- 线位置输出独立，不和电机 PWM 直接耦合。
- 支持黑线高/低电平和通道反序。

不足：

- 引脚仍绑定具体 SysConfig 宏。
- 丢线时控制策略偏简单，需要更完整的搜索和证据输出。

## 候选 2：Adnis018/TI_MSPM0G3507_Trace_Tracking_Module

本地路径：`待整理的文件夹/github/Adnis018__TI_MSPM0G3507_Trace_Tracking_Module`

上游：`https://github.com/Adnis018/TI_MSPM0G3507_Trace_Tracking_Module.git`

许可证：本地未发现 LICENSE，正式模板不直接复制源码。

可取之处：

- 8 路 GPIO + 加权中心误差思路小而清楚。
- 权重可外部配置，适合作为最小线位估计模型参考。

不足：

- 全亮、全黑、终点线、十字线需要上层区分。
- GPIO 和 SysConfig 绑定较紧。

## 候选 3：zby123581-maker/mspm0g3507-tracking-car

本地路径：`待整理的文件夹/github/zby123581-maker__mspm0g3507-tracking-car`

上游：`https://github.com/zby123581-maker/mspm0g3507-tracking-car.git`

许可证：本地未发现 LICENSE，正式模板不直接复制源码。

可取之处：

- 从灰度读取到差速控制链路完整。
- 有边缘和宽线意识，适合真实车测时参考。

不足：

- 丢线计时部分不够完整。
- 电机、PID、灰度读取耦合较紧，复用前必须拆层。

## 候选 4：2262727886-stack/mspm0g3507-car-kit

本地路径：`待整理的文件夹/github/2262727886-stack__mspm0g3507-car-kit`

上游：`https://github.com/2262727886-stack/mspm0g3507-car-kit`

许可证：本地未发现 LICENSE，正式模板不直接复制源码。

可取之处：

- 丢线时按最后方向搜索很实用。
- 控制输出有斜坡，能减少小车突然抖动。

不足：

- 单文件工程，和 IMU、OLED、按键、电机混在一起。
- 需要抽象后才能成为可复用模块。

## 本模板取舍

采用：

- DQ103 的模块边界思想。
- 加权平均线位估计。
- 最后有效方向丢线搜索。
- 控制输出斜坡。
- SWD/RAM 参数块和 telemetry 证据帧。

不采用：

- 固定 GPIOB 或固定 SysConfig 宏。
- 全白时直接把偏差清零。
- 控制器直接操作电机硬件。
- 未确认许可证来源的源码复制。
