# Build And Flash Boundary

## 本地构建

```powershell
Push-Location -LiteralPath 'E:\Learning\嵌入式\电赛备赛\MSPM03507各模块例程代码\整理后的模块\80_integrated_app\2026_天猛星车载最小上板工程骨架'
mingw32-make -f Makefile.mspm0g3507 clean
mingw32-make -f Makefile.mspm0g3507 all
Pop-Location
```

构建成功至少应看到：

- `arm-none-eabi-size build/tmx_car_minimal/tmx_car_minimal.elf`
- `build/tmx_car_minimal/tmx_car_minimal.hex`
- `build/tmx_car_minimal/tmx_car_minimal.symbols.txt`

## 烧录边界

本工程默认安全，但第一次上板仍建议：

1. TB6612 VM/12V 先不要接，或把车轮悬空。
2. 只接 MSPM0G3507、JY61P、循迹模块、OLED 的逻辑电源和公共地。
3. 烧录后先读运行符号，确认主循环和传感器状态。
4. 再接电机电源，逐项验证 A/B 路方向。

推荐 pyOCD 参数沿用本项目 MSPM0G3507 固定流程：

```powershell
pyocd load --target mspm0g3507 --frequency 500000 .\build\tmx_car_minimal\tmx_car_minimal.hex
```

如果有多个 DAPLink，必须加精确 `-u <probe-uid>`，不要自动选择。

## 可读符号

编译后可以在 `tmx_car_minimal.symbols.txt` 里找到：

- `g_board_app_runtime`
- `g_board_app_line_tuning`
- `g_board_app_line_snapshot`

这些符号用于第一次上板时验证：

- `loop_counter` 是否递增。
- `uart_rx_bytes`、`jy61p_angle_frames` 是否递增。
- `line_raw_bits` 是否随八路 TX/RX 输入变化。
- `oled_probe_ok` 是否为 1。
- `motors_armed` 是否保持 0。
