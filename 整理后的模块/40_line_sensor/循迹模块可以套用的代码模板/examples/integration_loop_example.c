#include "line_trace_template.h"

extern uint8_t Board_LineReadLevel(uint8_t channel, void *user);
extern uint8_t Board_LineReadSampleFrame(line_trace_sample_frame_t *out);
extern uint8_t Board_LineBuildSampleFrame(const line_trace_sensor_params_t *sensor,
                                          line_trace_sample_frame_t *out);
extern void Motor_SetTargetSpeed(int16_t left, int16_t right);

static line_trace_t g_line;
static line_trace_controller_t g_line_ctrl;
static line_trace_safety_state_t g_line_safety_state = LINE_TRACE_SAFETY_ARMED;
static uint32_t g_line_loop_counter;

static line_trace_algorithm_params_t g_line_runtime_algorithm = {
    .weights = { -35, -25, -15, -5, 5, 15, 25, 35 },
    .kp = 12,
    .kd = 4,
    .base_speed = 260,
    .max_correction = 500,
    .corner_slow_error = 25,
    .min_speed_percent = 65,
    .search_speed = 180,
    .ramp_step = 10,
};

static line_trace_sensor_params_t g_line_runtime_sensor = {
    .channel_count = 8u,
    .active_level = LINE_TRACE_ACTIVE_LOW,
    .threshold_mode = LINE_TRACE_THRESHOLD_DIGITAL_GPIO,
    .i2c_addr = 0u,
    .sensor_enable_mask = 0xffu,
};

volatile line_trace_tuning_block_t g_line_tuning_block;
volatile line_trace_telemetry_t g_line_telemetry;

void App_LineTraceInit(void)
{
    const line_trace_config_t line_cfg = {
        .channel_count = g_line_runtime_sensor.channel_count,
        .active_level = g_line_runtime_sensor.active_level,
        .weights = g_line_runtime_algorithm.weights,
        .read_level = Board_LineReadLevel,
        .user = 0,
        .sensor_enable_mask = g_line_runtime_sensor.sensor_enable_mask,
    };

    const line_trace_controller_config_t ctrl_cfg = {
        .kp = g_line_runtime_algorithm.kp,
        .kd = g_line_runtime_algorithm.kd,
        .max_correction = g_line_runtime_algorithm.max_correction,
        .corner_slow_error = g_line_runtime_algorithm.corner_slow_error,
        .min_speed_percent = g_line_runtime_algorithm.min_speed_percent,
        .search_speed = g_line_runtime_algorithm.search_speed,
        .ramp_step = g_line_runtime_algorithm.ramp_step,
    };

    LineTrace_Init(&g_line, &line_cfg);
    LineTrace_ControllerInit(&g_line_ctrl, &ctrl_cfg);
    LineTrace_TuningBlockInit(&g_line_tuning_block, &g_line_runtime_algorithm,
                              &g_line_runtime_sensor);
}

void App_LineTraceSetSafetyState(line_trace_safety_state_t state)
{
    g_line_safety_state = state;
}

void App_LineTraceTask20ms(void)
{
    line_trace_sample_frame_t frame;
    line_trace_result_t line;
    line_trace_motor_cmd_t cmd;
    line_trace_telemetry_t telemetry;

    (void)LineTrace_ApplyTuningBlock(&g_line_tuning_block, &g_line, &g_line_ctrl,
                                     &g_line_runtime_algorithm, &g_line_runtime_sensor,
                                     g_line_safety_state);

    if ((Board_LineBuildSampleFrame(&g_line_runtime_sensor, &frame) == 0u) ||
        (LineTrace_UpdateFromFrame(&g_line, &frame, &line) == 0u)) {
        Motor_SetTargetSpeed(0, 0);
        return;
    }

    LineTrace_ControllerStep(&g_line_ctrl, &line, g_line_runtime_algorithm.base_speed, &cmd);
    Motor_SetTargetSpeed(cmd.target_left, cmd.target_right);

    ++g_line_loop_counter;
    LineTrace_FillTelemetry(&line, &cmd, &g_line_tuning_block, g_line_safety_state,
                            g_line_loop_counter, &telemetry);
    g_line_telemetry = telemetry;
}
