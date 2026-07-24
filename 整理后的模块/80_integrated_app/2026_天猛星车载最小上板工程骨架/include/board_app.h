#ifndef BOARD_APP_H
#define BOARD_APP_H

#include <stdint.h>

#include "jy61p_min.h"
#include "line_trace_template.h"
#include "tmx_expansion_board.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t boot_cookie;
    uint32_t loop_counter;
    uint32_t uart_rx_bytes;
    uint32_t jy61p_angle_frames;
    uint32_t jy61p_gyro_frames;
    uint32_t jy61p_bad_checksums;
    int16_t jy61p_roll_cdeg;
    int16_t jy61p_pitch_cdeg;
    int16_t jy61p_yaw_cdeg;
    uint8_t line_raw_bits;
    uint8_t line_active_bits;
    uint8_t line_detected;
    int16_t line_position;
    int16_t line_error;
    uint8_t line_gpio_enabled;
    uint8_t lcd_init_ok;
    uint8_t lcd_backlight_on;
    uint8_t motors_armed;
    uint8_t motor_output_enabled;
    uint8_t safety_state;
} board_app_runtime_t;

void BoardApp_Init(void);
void BoardApp_Poll(void);
void BoardApp_Stop(void);

extern volatile board_app_runtime_t g_board_app_runtime;
extern volatile line_trace_tuning_block_t g_board_app_line_tuning;
extern volatile line_trace_bench_snapshot_t g_board_app_line_snapshot;

#ifdef __cplusplus
}
#endif

#endif
