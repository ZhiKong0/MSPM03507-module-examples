#include "line_trace_template.h"

extern uint8_t Board_LineReadLevel(uint8_t channel, void *user);
extern void Motor_SetTargetSpeed(int16_t left, int16_t right);

static const int16_t g_line_weights[8] = { -35, -25, -15, -5, 5, 15, 25, 35 };

static line_trace_t g_line;
static line_trace_controller_t g_line_ctrl;

void App_LineTraceInit(void)
{
    const line_trace_config_t line_cfg = {
        .channel_count = 8u,
        .active_level = LINE_TRACE_ACTIVE_LOW,
        .weights = g_line_weights,
        .read_level = Board_LineReadLevel,
        .user = 0,
    };

    const line_trace_controller_config_t ctrl_cfg = {
        .kp = 12,
        .kd = 4,
        .max_correction = 500,
        .corner_slow_error = 25,
        .min_speed_percent = 65,
        .search_speed = 180,
        .ramp_step = 10,
    };

    LineTrace_Init(&g_line, &line_cfg);
    LineTrace_ControllerInit(&g_line_ctrl, &ctrl_cfg);
}

void App_LineTraceTask20ms(void)
{
    line_trace_result_t line;
    line_trace_motor_cmd_t cmd;

    if (LineTrace_Update(&g_line, &line) == 0u) {
        Motor_SetTargetSpeed(0, 0);
        return;
    }

    LineTrace_ControllerStep(&g_line_ctrl, &line, 260, &cmd);
    Motor_SetTargetSpeed(cmd.target_left, cmd.target_right);
}
