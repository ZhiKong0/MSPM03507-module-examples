# 60 显示与交互

状态：`source-index`

## 模块目的

整理 OLED/LCD、按键、编码器菜单、状态页和调参页。电赛控制题里显示模块主要负责启动前状态确认、传感器诊断、PID 参数查看和故障提示。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill` | SSD1306 OLED、状态页、PID 调参显示 | `runtime-proven` |
| `待整理的文件夹/github/DQ103__mspm0-car-2024-h` | OLED task、IMU/line/速度状态显示 | `porting-candidate` |
| `待整理的文件夹/github/Comet966__MSPM0G3507_examples` | OLED 诊断页候选，待二次抽证据 | `source-index` |

## 证据路径

- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:70`：MSP 侧包含 SSD1306 OLED、JDY-31、VOFA+ 调试。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:165`：SSD1306 OLED 使用 I2C0 并标注已验证。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:194` 和 `:195`：OLED SDA/SCL 引脚为 PA28/PA31。
- `待整理的文件夹/github/2262727886-stack__mspm0g-contest-skill/README.md:250`：外设库中列出 `oled.c/h`。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/CLAUDE.md:147` 到 `:152`：Dashboard/OLED 相关状态包含 IMU、yaw、line raw。
- `待整理的文件夹/github/DQ103__mspm0-car-2024-h/archive/legacy/Sources/tasks/oled_task.c:13` 到 `:16`：OLED 显示 IMU ready、yaw、line bits/position、motion mode。

## 下一步

1. 先定义 OLED 页面：启动自检、传感器原始值、电机速度、PID 参数、故障页。
2. 按键/编码器/触摸屏都只产生统一语义动作，不直接调用电机底层。
3. OLED 和 BNO/MPU 可共用 I2C 总线，但物理接口和软件模块必须分开。

## 许可和构建备注

OLED 字库文件可能较大，正式模块要压缩字库范围。第三方 OLED 驱动可先作为参考，正式代码优先重写最小 SSD1306 I2C 驱动。
