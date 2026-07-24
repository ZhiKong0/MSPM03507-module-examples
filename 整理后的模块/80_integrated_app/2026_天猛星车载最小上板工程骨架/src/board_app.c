#include "board_app.h"

#include <string.h>

#include "tft_lcd_min.h"
#include "ti_msp_dl_config.h"

#ifndef BOARD_APP_ENABLE_MOTOR_OUTPUT
#define BOARD_APP_ENABLE_MOTOR_OUTPUT 0
#endif

#ifndef BOARD_APP_ENABLE_LINE_GPIO
#define BOARD_APP_ENABLE_LINE_GPIO 0
#endif

#ifndef BOARD_APP_ENABLE_TFT_LCD
#define BOARD_APP_ENABLE_TFT_LCD 1
#endif

#ifndef BOARD_APP_ENABLE_OLED_PROBE
#define BOARD_APP_ENABLE_OLED_PROBE 0
#endif

#define BOARD_APP_BOOT_COOKIE 0x544D5843u
#define BOARD_APP_LINE_BASE_SPEED 0

tmx_board_ops_t Board_BuildTmxOps(void);

volatile board_app_runtime_t g_board_app_runtime;
volatile line_trace_tuning_block_t g_board_app_line_tuning;
volatile line_trace_bench_snapshot_t g_board_app_line_snapshot;

static tmx_board_t g_board;
static jy61p_min_t g_jy61p;
static line_trace_t g_line_trace;
static line_trace_result_t g_line_result;
static line_trace_controller_t g_line_controller;
static line_trace_motor_cmd_t g_line_cmd;
static line_trace_algorithm_params_t g_line_algorithm = {
    .weights = { -70, -50, -30, -10, 10, 30, 50, 70 },
    .kp = 6,
    .kd = 18,
    .base_speed = BOARD_APP_LINE_BASE_SPEED,
    .max_correction = 500,
    .corner_slow_error = 45,
    .min_speed_percent = 35,
    .search_speed = 120,
    .ramp_step = 20,
};
static line_trace_sensor_params_t g_line_sensor = {
    .channel_count = 8u,
    .active_level = LINE_TRACE_ACTIVE_LOW,
    .channel_order = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u },
    .threshold = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
    .threshold_mode = LINE_TRACE_THRESHOLD_DIGITAL_GPIO,
    .i2c_addr = 0u,
    .sensor_enable_mask = 0xffu,
};

#if BOARD_APP_ENABLE_LINE_GPIO
static uint8_t line_pin_is_high(uint8_t channel)
{
    uint32_t pin = 0u;

    switch (channel) {
    case 0u:
        pin = LINE_TRACE_CH0_PIN;
        break;
    case 1u:
        pin = LINE_TRACE_CH1_PIN;
        break;
    case 2u:
        pin = LINE_TRACE_CH2_PIN;
        break;
    case 3u:
        pin = LINE_TRACE_CH3_PIN;
        break;
    case 4u:
        pin = LINE_TRACE_CH4_PIN;
        break;
    case 5u:
        pin = LINE_TRACE_CH5_PIN;
        break;
    case 6u:
        pin = LINE_TRACE_CH6_PIN;
        break;
    case 7u:
        pin = LINE_TRACE_CH7_PIN;
        break;
    default:
        return 0u;
    }

    return (DL_GPIO_readPins(LINE_TRACE_GPIO_PORT, pin) != 0u) ? 1u : 0u;
}

static uint8_t read_line_raw_bits(void)
{
    uint8_t raw = 0u;

    for (uint8_t i = 0u; i < 8u; ++i) {
        if (line_pin_is_high(i) != 0u) {
            raw = (uint8_t)(raw | (uint8_t)(1u << i));
        }
    }

    return raw;
}
#endif

static void poll_jy61p_uart(void)
{
    uint8_t byte = 0u;

    while (DL_UART_Main_receiveDataCheck(UART_0_INST, &byte)) {
        Jy61pMin_FeedByte(&g_jy61p, byte);
    }
}

static void motor_apply_if_enabled(const line_trace_motor_cmd_t *cmd)
{
#if BOARD_APP_ENABLE_MOTOR_OUTPUT
    if ((cmd == 0) || (g_board_app_runtime.safety_state != LINE_TRACE_SAFETY_RUNNING)) {
        return;
    }

    (void)cmd;
#else
    (void)cmd;
#endif
}

static void refresh_runtime(const line_trace_telemetry_t *telemetry)
{
    g_board_app_runtime.loop_counter++;
    g_board_app_runtime.uart_rx_bytes = g_jy61p.byte_count;
    g_board_app_runtime.jy61p_angle_frames = g_jy61p.angle_frame_count;
    g_board_app_runtime.jy61p_gyro_frames = g_jy61p.gyro_frame_count;
    g_board_app_runtime.jy61p_bad_checksums = g_jy61p.bad_checksum_count;
    g_board_app_runtime.jy61p_roll_cdeg = g_jy61p.roll_cdeg;
    g_board_app_runtime.jy61p_pitch_cdeg = g_jy61p.pitch_cdeg;
    g_board_app_runtime.jy61p_yaw_cdeg = g_jy61p.yaw_cdeg;
    g_board_app_runtime.line_raw_bits = g_line_result.raw_bits;
    g_board_app_runtime.line_active_bits = g_line_result.active_bits;
    g_board_app_runtime.line_detected = g_line_result.detected;
    g_board_app_runtime.line_position = g_line_result.position;
    g_board_app_runtime.line_error = g_line_result.error;
    g_board_app_runtime.line_gpio_enabled = BOARD_APP_ENABLE_LINE_GPIO ? 1u : 0u;
    g_board_app_runtime.motors_armed = g_board.motors_armed;
    g_board_app_runtime.motor_output_enabled = BOARD_APP_ENABLE_MOTOR_OUTPUT ? 1u : 0u;
    g_board_app_runtime.safety_state = telemetry->safety_state;
}

void BoardApp_Init(void)
{
    tmx_board_config_t board_cfg;
    tmx_board_ops_t ops;
    line_trace_config_t line_cfg;
    line_trace_controller_config_t ctrl_cfg;

    memset((void *)&g_board_app_runtime, 0, sizeof(g_board_app_runtime));
    g_board_app_runtime.boot_cookie = BOARD_APP_BOOT_COOKIE;
    g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_DISARMED;

    Jy61pMin_Init(&g_jy61p);

    TmxBoard_DefaultConfig(&board_cfg);
    board_cfg.max_motor_duty_permille = 250u;

    ops = Board_BuildTmxOps();
    (void)TmxBoard_Init(&g_board, &ops, &board_cfg);
    (void)TmxBoard_DisarmActuators(&g_board);

    memset(&line_cfg, 0, sizeof(line_cfg));
    line_cfg.channel_count = g_line_sensor.channel_count;
    line_cfg.active_level = g_line_sensor.active_level;
    line_cfg.weights = g_line_algorithm.weights;
    line_cfg.sensor_enable_mask = g_line_sensor.sensor_enable_mask;
    LineTrace_Init(&g_line_trace, &line_cfg);

    memset(&ctrl_cfg, 0, sizeof(ctrl_cfg));
    ctrl_cfg.kp = g_line_algorithm.kp;
    ctrl_cfg.kd = g_line_algorithm.kd;
    ctrl_cfg.max_correction = g_line_algorithm.max_correction;
    ctrl_cfg.corner_slow_error = g_line_algorithm.corner_slow_error;
    ctrl_cfg.min_speed_percent = g_line_algorithm.min_speed_percent;
    ctrl_cfg.search_speed = g_line_algorithm.search_speed;
    ctrl_cfg.ramp_step = g_line_algorithm.ramp_step;
    LineTrace_ControllerInit(&g_line_controller, &ctrl_cfg);
    LineTrace_TuningBlockInit(&g_board_app_line_tuning, &g_line_algorithm, &g_line_sensor);

#if BOARD_APP_ENABLE_TFT_LCD
    g_board_app_runtime.lcd_init_ok = TftLcd_Init();
    if (g_board_app_runtime.lcd_init_ok != 0u) {
        g_board_app_runtime.lcd_backlight_on = 1u;
        TftLcd_DrawBootPattern();
    }
#endif

#if BOARD_APP_ENABLE_OLED_PROBE
    (void)TmxBoard_OledProbe(&g_board);
#endif
}

void BoardApp_Poll(void)
{
    line_trace_sample_frame_t frame;
    line_trace_telemetry_t telemetry;
    uint8_t raw_bits = 0xffu;
    line_trace_sensor_status_t sensor_status = LINE_TRACE_SENSOR_STALE;

    poll_jy61p_uart();

    (void)LineTrace_ApplyTuningBlock(&g_board_app_line_tuning,
                                     &g_line_trace,
                                     &g_line_controller,
                                     &g_line_algorithm,
                                     &g_line_sensor,
                                     LINE_TRACE_SAFETY_DISARMED);

#if BOARD_APP_ENABLE_LINE_GPIO
    raw_bits = read_line_raw_bits();
    sensor_status = LINE_TRACE_SENSOR_OK;
#endif
    if (LineTrace_BuildSampleFrame(&g_line_sensor, raw_bits, 0u,
                                   sensor_status, &frame) != 0u) {
        (void)LineTrace_UpdateFromFrame(&g_line_trace, &frame, &g_line_result);
#if BOARD_APP_ENABLE_LINE_GPIO
        LineTrace_ControllerStep(&g_line_controller, &g_line_result,
                                 g_line_algorithm.base_speed, &g_line_cmd);
#else
        memset(&g_line_cmd, 0, sizeof(g_line_cmd));
#endif
    }

    motor_apply_if_enabled(&g_line_cmd);

    LineTrace_FillTelemetry(&g_line_result, &g_line_cmd, &g_board_app_line_tuning,
                            LINE_TRACE_SAFETY_DISARMED,
                            g_board_app_runtime.loop_counter, &telemetry);
    LineTrace_FillBenchSnapshot(&telemetry, (line_trace_bench_snapshot_t *)&g_board_app_line_snapshot);
    refresh_runtime(&telemetry);
}

void BoardApp_Stop(void)
{
    (void)TmxBoard_DisarmActuators(&g_board);
    LineTrace_ControllerReset(&g_line_controller);
    g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_DISARMED;
}
