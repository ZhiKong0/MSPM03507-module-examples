# 循迹模块可以套用的代码模板

状态：`template-v0.1`

这个文件夹给 MSPM0G3507 小车项目提供一个可套用的循迹模板。它不是直接复制任何上游工程源码，而是把本仓库候选池里已经出现的好结构重新整理成一个可移植版本：

- 硬件层只负责读 GPIO 高低电平。
- 传感器层输出 `raw_bits / active_bits / position / error / confidence / lost_dir`。
- 控制层把 `line error` 转换成左右轮目标速度。
- 丢线时按最后一次有效偏差方向搜索，不把偏差清零。

## 目录

| 文件 | 作用 |
|---|---|
| `include/line_trace_template.h` | 对外 API 和数据结构 |
| `src/line_trace_template.c` | 可移植循迹算法实现 |
| `examples/mspm0g3507_adapter_example.c` | MSPM0G3507 GPIO 适配示例 |
| `examples/integration_loop_example.c` | 控制循环接入示例 |
| `SOURCE_ANALYSIS.md` | 现成循迹代码分析和取舍 |
| `CMakeLists.txt` | 独立 object library 构建入口 |

## 接入步骤

1. 把 `include/` 和 `src/` 加入你的工程。
2. 在自己的板级文件里实现一个读通道函数：

```c
static uint8_t board_line_read_level(uint8_t index, void *user)
{
    (void)user;
    /* return 1 if GPIO is high, 0 if GPIO is low. */
}
```

3. 配置通道数、黑线电平、权重和读函数。
4. 每 5-20 ms 调用一次 `LineTrace_Update()`。
5. 把 `LineTrace_ControllerStep()` 产生的 `target_left/target_right` 交给你的速度环或电机 PWM 层。

## 推荐配置

7 路灰度：

```c
static const int16_t weights7[] = { -30, -20, -10, 0, 10, 20, 30 };
```

8 路灰度：

```c
static const int16_t weights8[] = { -35, -25, -15, -5, 5, 15, 25, 35 };
```

如果你的模块是“黑线输出低电平”，配置 `LINE_TRACE_ACTIVE_LOW`；如果黑线输出高电平，配置 `LINE_TRACE_ACTIVE_HIGH`。

## 验收顺序

1. 静态打印 `raw_bits`，确认每一路遮住黑线时位图变化正确。
2. 再看 `active_bits`，确认黑线电平极性没有反。
3. 左右移动黑线，确认 `error` 左负右正。
4. 只让小车低速循迹，不开高速。
5. 加入丢线搜索和速度环。

## 注意

模板默认 `error < 0` 表示线在左边，差速输出会让左轮减速、右轮加速。若你的电机方向或左右轮定义相反，先改电机层，不要把循迹层的符号到处打补丁。
