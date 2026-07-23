# 巡线调参 SOP

## 0. 总原则

每轮只改一类问题：

- 看不准线：先查传感器和 `active_bits`。
- 看得准但转向错：查 weights、左右轮方向和 error 符号。
- 会转但抖：小步调 `kp/kd/max_correction/ramp_step`。
- 能跑但慢：最后小步提高 `base_speed`。

## 1. 无硬件预检

```powershell
gcc -std=c99 -Wall -Wextra -Werror -Iinclude src\line_trace_template.c tests\test_line_trace_mock.c -o %TEMP%\line_trace_mock_tests.exe
%TEMP%\line_trace_mock_tests.exe
mingw32-make -f Makefile.mspm0g3507 clean all
```

## 2. 台架门 E-003

不开电机，只采样：

- `white_idle`
- `ch0_black` 到 `ch7_black`
- `left_to_right`
- `no_line_after_seen`

通过条件：每路遮挡 bit 正确，左侧 error 为负，右侧 error 为正。

## 3. 低速门 E-004

先用保守参数：

- 低 `base_speed`
- 较小 `kp`
- 足够小 `max_correction`
- 较大的 `ramp_step` 限制突变

通过条件：连续 3 圈或 3 分钟，无不可恢复丢线，无明显剧烈摆动。

## 4. 热调门 E-005

只在 `RUNNING_TUNE_SAFE` 中小步修改：

- `kp`
- `kd`
- `base_speed`
- `max_correction`

每次写入必须记录：

- `seq`
- `status`
- `error_code`
- `applied_seq/rejected_seq`
- telemetry 或 bench snapshot

## 5. 冻结 profile

只有 applied 且验证通过的计划才能 freeze。dry-run 只能生成
`dry-run-candidate`；真实车测通过后才能升级 `last-known-good`。

## 6. 不做的事

- 不用 SWD 直接写 PWM/GPIO/I2C/SPI/UART 外设寄存器。
- 不把一次偶然能跑的参数直接写 Flash。
- 不同时调传感器阈值和控制参数。
- 不在普通 `RUNNING` 状态热写控制参数。

