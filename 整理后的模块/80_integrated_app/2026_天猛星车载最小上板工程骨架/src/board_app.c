#include "board_app.h"

#include <string.h>

#include "nuedc_2024_h_app.h"
#include "rotary_menu.h"
#include "tft_lcd_min.h"
#include "ti_msp_dl_config.h"

#ifndef BOARD_APP_ENABLE_MOTOR_OUTPUT
#define BOARD_APP_ENABLE_MOTOR_OUTPUT 0
#endif

#ifndef BOARD_APP_ENABLE_LINE_GPIO
#define BOARD_APP_ENABLE_LINE_GPIO 0
#endif

#ifndef BOARD_APP_ENABLE_LINE_MUX_4051
#define BOARD_APP_ENABLE_LINE_MUX_4051 0
#endif

#if BOARD_APP_ENABLE_LINE_GPIO && BOARD_APP_ENABLE_LINE_MUX_4051
#error "Enable either BOARD_APP_ENABLE_LINE_GPIO or BOARD_APP_ENABLE_LINE_MUX_4051, not both."
#endif

#if BOARD_APP_ENABLE_LINE_GPIO || BOARD_APP_ENABLE_LINE_MUX_4051
#define BOARD_APP_LINE_SENSOR_ENABLED 1
#else
#define BOARD_APP_LINE_SENSOR_ENABLED 0
#endif

#ifndef BOARD_APP_LINE_MUX_SETTLE_CYCLES
#define BOARD_APP_LINE_MUX_SETTLE_CYCLES 64u
#endif

#ifndef BOARD_APP_ENABLE_TFT_LCD
#define BOARD_APP_ENABLE_TFT_LCD 1
#endif

#ifndef BOARD_APP_ENABLE_ROTARY_MENU
#define BOARD_APP_ENABLE_ROTARY_MENU 1
#endif

#ifndef BOARD_APP_ENABLE_OLED_PROBE
#define BOARD_APP_ENABLE_OLED_PROBE 0
#endif

#define BOARD_APP_BOOT_COOKIE 0x544D5843u
#define BOARD_APP_LINE_BASE_SPEED 0
#define BOARD_APP_LCD_REFRESH_MS 200u
#define BOARD_APP_EXIT_BUTTON_DEBOUNCE_MS 20u

tmx_board_ops_t Board_BuildTmxOps(void);

volatile board_app_runtime_t g_board_app_runtime;
volatile line_trace_tuning_block_t g_board_app_line_tuning;
volatile line_trace_bench_snapshot_t g_board_app_line_snapshot;
volatile nuedc_2024_h_runtime_t g_board_app_h_runtime;

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
static rotary_menu_t g_rotary_menu;
static nuedc_2024_h_app_t g_h_app;
static uint32_t g_last_lcd_render_loop;
static uint8_t g_exit_last_raw_pressed;
static uint8_t g_exit_stable_pressed;
static uint16_t g_exit_debounce_ms;

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

#if BOARD_APP_ENABLE_LINE_MUX_4051
static void set_line_mux_4051_channel(uint8_t channel)
{
    if ((channel & 0x01u) != 0u) {
        DL_GPIO_setPins(LINE_MUX_S0_PORT, LINE_MUX_S0_PIN);
    } else {
        DL_GPIO_clearPins(LINE_MUX_S0_PORT, LINE_MUX_S0_PIN);
    }

    if ((channel & 0x02u) != 0u) {
        DL_GPIO_setPins(LINE_MUX_S1_PORT, LINE_MUX_S1_PIN);
    } else {
        DL_GPIO_clearPins(LINE_MUX_S1_PORT, LINE_MUX_S1_PIN);
    }

    if ((channel & 0x04u) != 0u) {
        DL_GPIO_setPins(LINE_MUX_S2_PORT, LINE_MUX_S2_PIN);
    } else {
        DL_GPIO_clearPins(LINE_MUX_S2_PORT, LINE_MUX_S2_PIN);
    }

    delay_cycles(BOARD_APP_LINE_MUX_SETTLE_CYCLES);
}

static uint8_t read_line_mux_4051_bits(void)
{
    uint8_t raw = 0u;

    DL_GPIO_clearPins(LINE_MUX_E_PORT, LINE_MUX_E_PIN);

    for (uint8_t i = 0u; i < 8u; ++i) {
        set_line_mux_4051_channel(i);
        if (DL_GPIO_readPins(LINE_MUX_Z_PORT, LINE_MUX_Z_PIN) != 0u) {
            raw = (uint8_t)(raw | (uint8_t)(1u << i));
        }
    }

    return raw;
}
#endif

static uint8_t gpio_pin_is_high(GPIO_Regs *port, uint32_t pin)
{
    return (DL_GPIO_readPins(port, pin) != 0u) ? 1u : 0u;
}

static void stop_h_task_and_outputs(void)
{
    Nuedc2024H_Stop(&g_h_app);
    (void)TmxBoard_DisarmMotors(&g_board);
    LineTrace_ControllerReset(&g_line_controller);
    memset(&g_line_cmd, 0, sizeof(g_line_cmd));
    g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_DISARMED;
}

static void start_h_mode(nuedc_2024_h_mode_t mode, uint8_t require_arm)
{
    if ((require_arm != 0u) &&
        (g_board_app_runtime.safety_state != LINE_TRACE_SAFETY_ARMED)) {
        g_board_app_runtime.menu_status_code = 1u;
        return;
    }

    Nuedc2024H_Start(&g_h_app, mode);
    if (require_arm != 0u) {
        g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_RUNNING;
    }
    g_board_app_runtime.menu_status_code = 0u;
}

static void handle_menu_action(rotary_menu_action_t action)
{
    if (action == ROTARY_MENU_ACTION_NONE) {
        return;
    }

    g_board_app_runtime.rotary_last_action = (uint8_t)action;
    g_rotary_menu.redraw_requested = 1u;

    switch (action) {
    case ROTARY_MENU_ACTION_ARM_TOGGLE:
        if (g_board_app_runtime.safety_state == LINE_TRACE_SAFETY_DISARMED) {
            if (TmxBoard_ArmMotors(&g_board, TMX_BOARD_MOTOR_ARM_COOKIE) == TMX_RESULT_OK) {
                g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_ARMED;
                g_board_app_runtime.menu_status_code = 0u;
            } else {
                g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_FAULT;
                g_board_app_runtime.menu_status_code = 2u;
            }
        } else {
            stop_h_task_and_outputs();
        }
        break;
    case ROTARY_MENU_ACTION_STOP:
        stop_h_task_and_outputs();
        g_board_app_runtime.menu_status_code = 0u;
        break;
    case ROTARY_MENU_ACTION_SELF_TEST:
        (void)TmxBoard_SmokeStep(&g_board, TMX_SMOKE_BUZZER_TICK);
        (void)TmxBoard_SmokeStep(&g_board, TMX_SMOKE_RGB_CYCLE);
        g_board_app_runtime.menu_status_code = 0u;
        break;
    case ROTARY_MENU_ACTION_H_BASIC_AB:
        start_h_mode(NUEDC_2024_H_MODE_BASIC_AB, 1u);
        break;
    case ROTARY_MENU_ACTION_H_BASIC_LOOP:
        start_h_mode(NUEDC_2024_H_MODE_BASIC_LOOP, 1u);
        break;
    case ROTARY_MENU_ACTION_H_FIGURE8:
        start_h_mode(NUEDC_2024_H_MODE_FIGURE8, 1u);
        break;
    case ROTARY_MENU_ACTION_H_FIGURE8_4LAP:
        start_h_mode(NUEDC_2024_H_MODE_FIGURE8_4LAP, 1u);
        break;
    case ROTARY_MENU_ACTION_SENSOR_CHECK:
        start_h_mode(NUEDC_2024_H_MODE_SENSOR_CHECK, 0u);
        break;
    case ROTARY_MENU_ACTION_MOTOR_CHECK:
        start_h_mode(NUEDC_2024_H_MODE_MOTOR_CHECK, 1u);
        break;
    default:
        break;
    }
}

static void poll_rotary_menu(void)
{
#if BOARD_APP_ENABLE_ROTARY_MENU
    RotaryMenu_Update(&g_rotary_menu,
                      gpio_pin_is_high(ROTARY_ENC_A_PORT, ROTARY_ENC_A_PIN),
                      gpio_pin_is_high(ROTARY_ENC_B_PORT, ROTARY_ENC_B_PIN),
                      gpio_pin_is_high(ROTARY_ENC_C_PORT, ROTARY_ENC_C_PIN));
    handle_menu_action(RotaryMenu_ConsumeAction(&g_rotary_menu));
#endif
}

static void poll_exit_button(void)
{
#if BOARD_APP_ENABLE_ROTARY_MENU
    uint8_t raw_pressed =
        (gpio_pin_is_high(MENU_EXIT_BUTTON_PORT, MENU_EXIT_BUTTON_PIN) == 0u) ? 1u : 0u;

    if (raw_pressed != g_exit_last_raw_pressed) {
        g_exit_last_raw_pressed = raw_pressed;
        g_exit_debounce_ms = 0u;
    } else if (g_exit_debounce_ms < BOARD_APP_EXIT_BUTTON_DEBOUNCE_MS) {
        g_exit_debounce_ms++;
    }

    if ((g_exit_debounce_ms >= BOARD_APP_EXIT_BUTTON_DEBOUNCE_MS) &&
        (raw_pressed != g_exit_stable_pressed)) {
        g_exit_stable_pressed = raw_pressed;
        g_board_app_runtime.exit_button_down = raw_pressed;
        g_rotary_menu.redraw_requested = 1u;

        if (raw_pressed != 0u) {
            stop_h_task_and_outputs();
            g_board_app_runtime.menu_status_code = 3u;
            g_board_app_runtime.exit_button_counter++;
        }
    }
#endif
}

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
    uint16_t duty_left;
    uint16_t duty_right;
    tmx_motor_dir_t dir_left;
    tmx_motor_dir_t dir_right;

    if ((cmd == 0) ||
        (g_board_app_runtime.safety_state != LINE_TRACE_SAFETY_RUNNING) ||
        (g_board.motors_armed == 0u)) {
        (void)TmxBoard_SetMotorRaw(&g_board, TMX_MOTOR_A, TMX_MOTOR_COAST, 0u);
        (void)TmxBoard_SetMotorRaw(&g_board, TMX_MOTOR_B, TMX_MOTOR_COAST, 0u);
        return;
    }

    duty_left = (cmd->target_left < 0) ? (uint16_t)(-cmd->target_left)
                                       : (uint16_t)cmd->target_left;
    duty_right = (cmd->target_right < 0) ? (uint16_t)(-cmd->target_right)
                                         : (uint16_t)cmd->target_right;
    dir_left = (cmd->target_left < 0) ? TMX_MOTOR_REVERSE : TMX_MOTOR_FORWARD;
    dir_right = (cmd->target_right < 0) ? TMX_MOTOR_REVERSE : TMX_MOTOR_FORWARD;

    (void)TmxBoard_SetMotorRaw(&g_board, TMX_MOTOR_A, dir_left, duty_left);
    (void)TmxBoard_SetMotorRaw(&g_board, TMX_MOTOR_B, dir_right, duty_right);
#else
    (void)cmd;
#endif
}

static void update_h_task_command(void)
{
    nuedc_2024_h_inputs_t inputs;
    const nuedc_2024_h_runtime_t *h_rt;

    memset(&inputs, 0, sizeof(inputs));
    inputs.line_detected = g_line_result.detected;
    inputs.line_pattern = g_line_result.pattern;
    inputs.line_error = g_line_result.error;
    inputs.yaw_cdeg = g_jy61p.yaw_cdeg;

    Nuedc2024H_Update(&g_h_app, &inputs);
    h_rt = Nuedc2024H_Runtime(&g_h_app);

    if ((h_rt == 0) || (h_rt->running == 0u) ||
        (g_board_app_runtime.safety_state != LINE_TRACE_SAFETY_RUNNING)) {
        memset(&g_line_cmd, 0, sizeof(g_line_cmd));
        if ((h_rt != 0) && ((h_rt->done != 0u) || (h_rt->fault != 0u))) {
            (void)TmxBoard_DisarmMotors(&g_board);
            g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_DISARMED;
        }
        return;
    }

    if (h_rt->drive_mode == NUEDC_2024_H_DRIVE_STRAIGHT) {
        g_line_cmd.target_left = h_rt->target_left_permille;
        g_line_cmd.target_right = h_rt->target_right_permille;
        g_line_cmd.correction = 0;
        g_line_cmd.searching = 0u;
    } else if (h_rt->drive_mode == NUEDC_2024_H_DRIVE_LINE_FOLLOW) {
#if BOARD_APP_LINE_SENSOR_ENABLED
        LineTrace_ControllerStep(&g_line_controller, &g_line_result,
                                 h_rt->base_speed_permille, &g_line_cmd);
#else
        memset(&g_line_cmd, 0, sizeof(g_line_cmd));
#endif
    } else {
        memset(&g_line_cmd, 0, sizeof(g_line_cmd));
    }
}

static void refresh_runtime(const line_trace_telemetry_t *telemetry)
{
    const nuedc_2024_h_runtime_t *h_rt = Nuedc2024H_Runtime(&g_h_app);

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
    g_board_app_runtime.line_mux_4051_enabled = BOARD_APP_ENABLE_LINE_MUX_4051 ? 1u : 0u;
    g_board_app_runtime.motors_armed = g_board.motors_armed;
    g_board_app_runtime.motor_output_enabled = BOARD_APP_ENABLE_MOTOR_OUTPUT ? 1u : 0u;
    g_board_app_runtime.safety_state = telemetry->safety_state;
    g_board_app_runtime.rotary_selected = g_rotary_menu.selected;
    g_board_app_runtime.rotary_button_down = g_rotary_menu.button_down;
    g_board_app_runtime.rotary_detents = g_rotary_menu.detent_count;

    if (h_rt != 0) {
        g_board_app_h_runtime = *h_rt;
        g_board_app_runtime.h_mode = (uint8_t)h_rt->mode;
        g_board_app_runtime.h_drive_mode = (uint8_t)h_rt->drive_mode;
        g_board_app_runtime.h_from_point = (uint8_t)h_rt->from_point;
        g_board_app_runtime.h_to_point = (uint8_t)h_rt->to_point;
        g_board_app_runtime.h_segment_index = h_rt->segment_index;
        g_board_app_runtime.h_segment_count = h_rt->segment_count;
        g_board_app_runtime.h_lap_index = h_rt->lap_index;
        g_board_app_runtime.h_lap_count = h_rt->lap_count;
        g_board_app_runtime.h_running = h_rt->running;
        g_board_app_runtime.h_done = h_rt->done;
        g_board_app_runtime.h_fault = h_rt->fault;
        g_board_app_runtime.h_elapsed_ms = h_rt->elapsed_ms;
        g_board_app_runtime.h_deadline_ms = h_rt->deadline_ms;
        g_board_app_runtime.h_target_left_permille = h_rt->target_left_permille;
        g_board_app_runtime.h_target_right_permille = h_rt->target_right_permille;
    }
}

#if BOARD_APP_ENABLE_TFT_LCD
static const char *safety_name(uint8_t safety_state)
{
    switch ((line_trace_safety_state_t)safety_state) {
    case LINE_TRACE_SAFETY_ARMED:
        return "ARMED";
    case LINE_TRACE_SAFETY_RUNNING:
        return "RUN";
    case LINE_TRACE_SAFETY_RUNNING_TUNE_SAFE:
        return "TUNE";
    case LINE_TRACE_SAFETY_FAULT:
        return "FAULT";
    default:
        return "DISARM";
    }
}

static void draw_u32(uint16_t x, uint16_t y, uint32_t value, uint16_t color)
{
    char buf[11];
    int8_t pos = 10;

    buf[pos] = '\0';
    do {
        --pos;
        buf[pos] = (char)('0' + (value % 10u));
        value /= 10u;
    } while ((value != 0u) && (pos > 0));

    TftLcd_DrawString(x, y, &buf[pos], color, TFT_LCD_COLOR_BLACK, 1u);
}

static uint8_t menu_window_start(uint8_t selected, uint8_t count)
{
    const uint8_t visible_rows = 6u;
    uint8_t start = 0u;

    if (count <= visible_rows) {
        return 0u;
    }
    if (selected > 2u) {
        start = (uint8_t)(selected - 2u);
    }
    if ((uint8_t)(start + visible_rows) > count) {
        start = (uint8_t)(count - visible_rows);
    }
    return start;
}

static void render_rotary_menu(void)
{
    const nuedc_2024_h_runtime_t *h_rt = Nuedc2024H_Runtime(&g_h_app);
    uint8_t count = RotaryMenu_Count();
    uint8_t start = menu_window_start(g_rotary_menu.selected, count);
    uint16_t status_color = TFT_LCD_COLOR_GREEN;

    if (g_board_app_runtime.lcd_init_ok == 0u) {
        return;
    }

    TftLcd_FillScreen(TFT_LCD_COLOR_BLACK);
    TftLcd_DrawString(0u, 0u, "MSPM0 2024 H", TFT_LCD_COLOR_CYAN,
                      TFT_LCD_COLOR_BLACK, 1u);

    for (uint8_t row = 0u; row < 6u; ++row) {
        uint8_t index = (uint8_t)(start + row);
        uint16_t y = (uint16_t)(14u + (uint16_t)row * 10u);
        uint8_t selected = (index == g_rotary_menu.selected) ? 1u : 0u;
        uint16_t bg = selected ? TFT_LCD_COLOR_WHITE : TFT_LCD_COLOR_BLACK;
        uint16_t fg = selected ? TFT_LCD_COLOR_BLACK : TFT_LCD_COLOR_WHITE;

        TftLcd_FillRect(0u, y, TFT_LCD_WIDTH, 9u, bg);
        if (index < count) {
            TftLcd_DrawString(0u, y, selected ? ">" : " ", fg, bg, 1u);
            TftLcd_DrawString(8u, y, RotaryMenu_Label(index), fg, bg, 1u);
        }
    }

    if (g_board_app_runtime.menu_status_code != 0u) {
        status_color = TFT_LCD_COLOR_ORANGE;
    }
    if ((h_rt != 0) && (h_rt->fault != 0u)) {
        status_color = TFT_LCD_COLOR_RED;
    }

    TftLcd_DrawString(0u, 78u, "SAFE:", TFT_LCD_COLOR_YELLOW,
                      TFT_LCD_COLOR_BLACK, 1u);
    TftLcd_DrawString(36u, 78u, safety_name(g_board_app_runtime.safety_state),
                      status_color, TFT_LCD_COLOR_BLACK, 1u);

    TftLcd_DrawString(0u, 90u, "MODE:", TFT_LCD_COLOR_YELLOW,
                      TFT_LCD_COLOR_BLACK, 1u);
    TftLcd_DrawString(36u, 90u,
                      (h_rt == 0) ? "IDLE" : Nuedc2024H_ModeName(h_rt->mode),
                      TFT_LCD_COLOR_WHITE, TFT_LCD_COLOR_BLACK, 1u);

    TftLcd_DrawString(0u, 102u, "DRV:", TFT_LCD_COLOR_YELLOW,
                      TFT_LCD_COLOR_BLACK, 1u);
    TftLcd_DrawString(30u, 102u,
                      (h_rt == 0) ? "IDLE" : Nuedc2024H_DriveName(h_rt->drive_mode),
                      TFT_LCD_COLOR_WHITE, TFT_LCD_COLOR_BLACK, 1u);

    TftLcd_DrawString(0u, 114u, "SEG:", TFT_LCD_COLOR_YELLOW,
                      TFT_LCD_COLOR_BLACK, 1u);
    if (h_rt != 0) {
        TftLcd_DrawString(30u, 114u, Nuedc2024H_PointName(h_rt->from_point),
                          TFT_LCD_COLOR_WHITE, TFT_LCD_COLOR_BLACK, 1u);
        TftLcd_DrawString(38u, 114u, ">", TFT_LCD_COLOR_WHITE,
                          TFT_LCD_COLOR_BLACK, 1u);
        TftLcd_DrawString(46u, 114u, Nuedc2024H_PointName(h_rt->to_point),
                          TFT_LCD_COLOR_WHITE, TFT_LCD_COLOR_BLACK, 1u);
    }

    TftLcd_DrawString(0u, 126u, "LAP:", TFT_LCD_COLOR_YELLOW,
                      TFT_LCD_COLOR_BLACK, 1u);
    if (h_rt != 0) {
        draw_u32(30u, 126u, (uint32_t)h_rt->lap_index + 1u, TFT_LCD_COLOR_WHITE);
        TftLcd_DrawString(42u, 126u, "/", TFT_LCD_COLOR_WHITE,
                          TFT_LCD_COLOR_BLACK, 1u);
        draw_u32(50u, 126u, h_rt->lap_count, TFT_LCD_COLOR_WHITE);
    }

    TftLcd_DrawString(0u, 138u, "TIME:", TFT_LCD_COLOR_YELLOW,
                      TFT_LCD_COLOR_BLACK, 1u);
    if (h_rt != 0) {
        draw_u32(36u, 138u, h_rt->elapsed_ms / 1000u, TFT_LCD_COLOR_WHITE);
        TftLcd_DrawString(60u, 138u, "S", TFT_LCD_COLOR_WHITE,
                          TFT_LCD_COLOR_BLACK, 1u);
    }

    if (g_board_app_runtime.menu_status_code == 1u) {
        TftLcd_DrawString(0u, 150u, "PRESS ARM FIRST", TFT_LCD_COLOR_ORANGE,
                          TFT_LCD_COLOR_BLACK, 1u);
    } else if ((h_rt != 0) && (h_rt->done != 0u)) {
        TftLcd_DrawString(0u, 150u, "DONE", TFT_LCD_COLOR_GREEN,
                          TFT_LCD_COLOR_BLACK, 1u);
    } else if ((h_rt != 0) && (h_rt->fault != 0u)) {
        TftLcd_DrawString(0u, 150u, "FAULT STOPPED", TFT_LCD_COLOR_RED,
                          TFT_LCD_COLOR_BLACK, 1u);
    } else if (g_board_app_runtime.menu_status_code == 3u) {
        TftLcd_DrawString(0u, 150u, "EXIT", TFT_LCD_COLOR_GREEN,
                          TFT_LCD_COLOR_BLACK, 1u);
    } else {
        TftLcd_DrawString(0u, 150u, "OK=KNOB B12=EXIT", TFT_LCD_COLOR_GRAY,
                          TFT_LCD_COLOR_BLACK, 1u);
    }

    g_rotary_menu.redraw_requested = 0u;
    g_last_lcd_render_loop = g_board_app_runtime.loop_counter;
}

static void render_menu_if_due(void)
{
    uint32_t elapsed = g_board_app_runtime.loop_counter - g_last_lcd_render_loop;

    if ((g_rotary_menu.redraw_requested != 0u) ||
        (elapsed >= BOARD_APP_LCD_REFRESH_MS)) {
        render_rotary_menu();
    }
}
#endif

void BoardApp_Init(void)
{
    tmx_board_config_t board_cfg;
    tmx_board_ops_t ops;
    line_trace_config_t line_cfg;
    line_trace_controller_config_t ctrl_cfg;

    memset((void *)&g_board_app_runtime, 0, sizeof(g_board_app_runtime));
    memset((void *)&g_board_app_h_runtime, 0, sizeof(g_board_app_h_runtime));
    g_board_app_runtime.boot_cookie = BOARD_APP_BOOT_COOKIE;
    g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_DISARMED;

    Jy61pMin_Init(&g_jy61p);
    RotaryMenu_Init(&g_rotary_menu);
    Nuedc2024H_Init(&g_h_app);

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
        render_rotary_menu();
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
    poll_rotary_menu();
    poll_exit_button();

    (void)LineTrace_ApplyTuningBlock(&g_board_app_line_tuning,
                                     &g_line_trace,
                                     &g_line_controller,
                                     &g_line_algorithm,
                                     &g_line_sensor,
                                     (line_trace_safety_state_t)g_board_app_runtime.safety_state);

#if BOARD_APP_ENABLE_LINE_GPIO
    raw_bits = read_line_raw_bits();
    sensor_status = LINE_TRACE_SENSOR_OK;
#endif
#if BOARD_APP_ENABLE_LINE_MUX_4051
    raw_bits = read_line_mux_4051_bits();
    sensor_status = LINE_TRACE_SENSOR_OK;
#endif
    if (LineTrace_BuildSampleFrame(&g_line_sensor, raw_bits, 0u,
                                   sensor_status, &frame) != 0u) {
        (void)LineTrace_UpdateFromFrame(&g_line_trace, &frame, &g_line_result);
    }

    update_h_task_command();
    motor_apply_if_enabled(&g_line_cmd);

    LineTrace_FillTelemetry(&g_line_result, &g_line_cmd, &g_board_app_line_tuning,
                            (line_trace_safety_state_t)g_board_app_runtime.safety_state,
                            g_board_app_runtime.loop_counter, &telemetry);
    LineTrace_FillBenchSnapshot(&telemetry, (line_trace_bench_snapshot_t *)&g_board_app_line_snapshot);
    refresh_runtime(&telemetry);

#if BOARD_APP_ENABLE_TFT_LCD
    render_menu_if_due();
#endif
}

void BoardApp_Stop(void)
{
    (void)TmxBoard_DisarmActuators(&g_board);
    LineTrace_ControllerReset(&g_line_controller);
    g_board_app_runtime.safety_state = LINE_TRACE_SAFETY_DISARMED;
}
