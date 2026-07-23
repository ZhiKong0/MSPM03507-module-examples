# E-003 台架传感器验证流程

## 目标

E-003 只验证“传感器看线是否正确”，不要开电机。通过 SWD 或其它只读观测方式采集 `raw_bits / active_bits / error / confidence`，确认通道映射、黑线有效电平和左右误差方向。

## 前置条件

- 已完成 E-001 PC/mock 测试。
- 已完成 E-002 MSPM0 GNU 构建。
- 目标板已烧入包含 `g_line_bench_snapshot` 的巡线 smoke 或集成固件。
- 电机电源断开，或电机驱动保持禁止状态。
- 只允许一个 SWD owner 访问探针；不要同时开 pyOCD、OpenOCD、GUI 调参器或 SWD 面板。

## 构建

```powershell
Push-Location -LiteralPath 'E:\Learning\嵌入式\电赛备赛\MSPM03507各模块例程代码\整理后的模块\40_line_sensor\循迹模块可以套用的代码模板'
mingw32-make -f Makefile.mspm0g3507 clean all
Pop-Location
```

构建时必须能解析到：

- `g_line_bench_snapshot`
- `g_line_tuning_block`
- `g_line_telemetry`
- `g_line_smoke_counter`

## Dry-run

dry-run 不访问硬件，只解析 ELF 并生成只读 pyOCD 命令：

```powershell
python tools\line_trace_bench_capture.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case dry_run_center
```

输出 JSON 中的 `pyocd_command` 只包含 `status/read32/exit`，不会写内存、擦除、烧录或复位。

## 采样序列

建议按下面顺序采样，每个 case 至少采 1 次，关键 case 可采 3 次：

| Case | 操作 | 通过条件 |
|---|---|---|
| white_idle | 所有探头离开黑线或放在白底 | `active_bits` 应接近 0，或按传感器实际背景记录 |
| ch0_black | 只遮挡/压黑第 0 路 | 对应 bit 变化，若第 0 路在左侧则 `error < 0` |
| ch1_black | 只遮挡/压黑第 1 路 | 对应 bit 变化，误差方向符合左负右正 |
| ch2_black | 只遮挡/压黑第 2 路 | 对应 bit 变化 |
| ch3_black | 只遮挡/压黑第 3 路 | 靠近中心，`error` 绝对值较小 |
| ch4_black | 只遮挡/压黑第 4 路 | 靠近中心，`error` 绝对值较小 |
| ch5_black | 只遮挡/压黑第 5 路 | 对应 bit 变化 |
| ch6_black | 只遮挡/压黑第 6 路 | 对应 bit 变化 |
| ch7_black | 只遮挡/压黑第 7 路 | 对应 bit 变化，若第 7 路在右侧则 `error > 0` |
| left_to_right | 黑线从左扫到右 | `error` 应从负数连续变化到正数 |
| no_line_after_seen | 先给有效线再移开 | `detected=0` 时保留最后 `lost_dir` |

## 单次真实采样

真实采样会访问 SWD，但仍然只读：

```powershell
python tools\line_trace_bench_capture.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --pack C:\Users\DZ\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack `
  --case ch0_black `
  --samples 3 `
  --interval-ms 200 `
  --out-json evidence\e003-ch0-black.json `
  --out-csv evidence\e003-ch0-black.csv `
  --run
```

每换一个遮挡位置，就改 `--case` 和输出文件名。

## 证据校验

采完 `white_idle` 和 `ch0_black` 到 `ch7_black` 后，运行：

```powershell
python tools\line_trace_bench_validate.py evidence\e003\e003-*.json --strict
```

校验器会检查：

- 必要 case 是否齐全。
- `chN_black` 的 `active_bits` 是否包含 bit N。
- 左侧通道是否为负误差，右侧通道是否为正误差。
- `white_idle` 的 `active_count` 是否不超过设定阈值。
- `sensor_status` 是否为 0。

如果你的巡线模块物理通道不是 0 左 7 右，先修正 `channel_order`，再重新采样；不要在算法源文件里硬改左右符号。

默认证据目录是 `evidence/e003/`。其中 `e003-template.json` 只说明字段和命令格式，校验器会跳过 `template_only=true` 的模板文件。

## 判定

E-003 通过时，证据必须能回答：

- 每一路遮挡后，正确 bit 会变化。
- `active_level` 没有反。
- `channel_order` 没有左右反或乱序；如果反了，只改适配层配置，不改算法源文件。
- 左侧黑线 `error < 0`，右侧黑线 `error > 0`。
- `confidence` 和 `active_count` 随遮挡数量合理变化。
- `sensor_status=LINE_TRACE_SENSOR_OK`，`error_code=LINE_TRACE_ERROR_OK`。

## 下一步

E-003 通过后再进入 E-004 低速车测。低速车测前仍建议保持保守 `base_speed`，先证明低速稳定，再尝试 SWD 热调参。
