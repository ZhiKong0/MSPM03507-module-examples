# 30 电机与编码器

状态：`source-index`

## 模块目的

整理差速小车最核心闭环：PWM 输出、TB6612/DRV8833 方向控制、编码器计数、速度 PI、轮速命令、底盘差速解算和里程反馈。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| 官方 SDK `timx_timer_mode_pwm_edge_sleep` | PWM 官方基准 | `source-index` |
| 官方 SDK `timg_qei_mode` | QEI 编码器官方基准 | `source-index` |
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | encoder/wheel/chassis 分层、FreeRTOS 实车结构 | `porting-candidate` |
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | TB6612、编码器、级联 PID、PID 调参工具 | `porting-candidate` |

## 证据路径

- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timx_timer_mode_pwm_edge_sleep/timx_timer_mode_pwm_edge_sleep.c:39`：启动 PWM 定时器实例。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timg_qei_mode/timg_qei_mode.c:41`：启动 QEI 定时器实例。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/timg_qei_mode/timg_qei_mode.c:52`：读取 QEI 方向。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Drivers/Devices/encoder_drv.c:7`：`Encoder_Init` 入口。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Drivers/Devices/encoder_drv.c:27`、`:67`、`:83`：编码器边沿 IRQ、速度、计数接口。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/Control/chassis.c:3`、`:16`、`:26`、`:43`、`:64`：底盘初始化、轮速、twist、控制步进、停止。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:63`：MSP 侧包含编码器、TB6612、舵机等执行/反馈组件。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:67`：速度 PI、位置 P、航向 PD、转弯 PID 的级联控制思路。

## 下一步

1. 定义统一接口：`motor_set_pwm(left,right)`、`encoder_sample()`、`speed_pi_update()`、`chassis_set_twist(v,w)`。
2. 先做一个单电机闭环，再扩到双轮差速，最后接入线位置/IMU 控制器。
3. 编译验收先用 host 或 mock 数据测试 PID 符号，再上 DAPLink 烧录验证 PWM/QEI。

## 许可和构建备注

不要直接混拷多个项目的电机驱动。正式实现应以官方 PWM/QEI 为底，参考第三方接口边界后重写，避免许可证和硬件 pin 假设混乱。
