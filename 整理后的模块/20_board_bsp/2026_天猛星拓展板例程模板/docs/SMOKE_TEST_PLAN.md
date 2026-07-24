# Smoke Test Plan

目标：在不让小车突然动作的前提下，逐项确认拓展板电源、基础 IO、显示、通信、舵机和电机接口可控。

## 前置检查

| 检查项 | 通过标准 |
|---|---|
| 空载 12V | 电源极性正确，无明显发热 |
| 空载 5V | 电压稳定，纹波和压降在可接受范围内 |
| 主控上电 | MSPM0G3507 能下载/复位/运行最小程序 |
| 共地 | 电池、降压、主控、驱动、传感器 GND 连通 |
| 机械状态 | 车轮架空，舵机无连杆或无危险行程 |

## 推荐顺序

1. `TMX_SMOKE_SAFE_DISARM`
   - 期望：电机 PWM 为 0，方向脚为 coast，蜂鸣器/RGB/舵机输出关闭。
2. `TMX_SMOKE_BUZZER_TICK`
   - 期望：蜂鸣器短响一次；如果常响，检查有效电平。
3. `TMX_SMOKE_OLED_PROBE`
   - 期望：I2C0 上 OLED 地址 `0x3C` ACK；不 ACK 时再试 `0x3D`。
4. `TMX_SMOKE_UART0_PING`
   - 期望：UART0 发出 `TMX-EXPANSION-PING`；如果 UART0 给 IMU，用 UART1/2/3 改写适配层。
5. `TMX_SMOKE_RGB_CYCLE`
   - 期望：PB22、PB5、PA18 对应的三路灯按顺序变化；如果反相，在适配层修正。
6. `TMX_SMOKE_SERVO_CENTER_IF_ENABLED`
   - 先显式 `TmxBoard_EnableServos(&board, 1u)`，再执行。
   - 期望：舵机到中心，不撞限位。
7. `TMX_SMOKE_MOTOR_STATIC_SAFE`
   - 期望：再次确认电机仍不转。

## 电机首次转动

这一步不在默认 smoke test 里，需要人工确认后单独执行：

```c
TmxBoard_ArmMotors(&board, TMX_BOARD_MOTOR_ARM_COOKIE);
TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_FORWARD, 100u);
TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_COAST, 0u);
```

通过标准：

- 车轮架空。
- duty 不超过 10% 到 20%。
- 方向和编码器计数同步记录。
- 任意异常立即 `TmxBoard_DisarmActuators()`。

## SysConfig 接入检查

执行 smoke 前先按 `docs/SYSCONFIG_ADAPTER_PORTING.md` 核对这些宏：

- `TMX_SYSCFG_MOTOR_PWM_INST` 和两个 `DL_TIMER_CC_*_INDEX` 是否对应 `PA12/PA13`。
- `TMX_SYSCFG_SERVO_PWM_INST` 和四个舵机 compare index 是否对应 `PA22/PA15/PA17/PA21`。
- `TMX_SYSCFG_OLED_I2C_INST` 是否为原理图里的 `I2C0`。
- `TMX_SYSCFG_UART*_INST` 是否与实际调试/IMU/视觉串口分配一致。

## 记录模板

| 步骤 | 结果 | 备注 |
|---|---|---|
| SAFE_DISARM |  |  |
| BUZZER_TICK |  |  |
| OLED_PROBE |  |  |
| UART0_PING |  |  |
| RGB_CYCLE |  |  |
| SERVO_CENTER |  |  |
| MOTOR_STATIC_SAFE |  |  |
