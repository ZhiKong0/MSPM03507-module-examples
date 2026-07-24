#ifndef NUEDC_2024_H_APP_H
#define NUEDC_2024_H_APP_H

#include <stdint.h>

#include "line_trace_template.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUEDC_2024_H_MAGIC 0x48323032u

typedef enum {
    NUEDC_2024_H_MODE_IDLE = 0,
    NUEDC_2024_H_MODE_BASIC_AB,
    NUEDC_2024_H_MODE_BASIC_LOOP,
    NUEDC_2024_H_MODE_FIGURE8,
    NUEDC_2024_H_MODE_FIGURE8_4LAP,
    NUEDC_2024_H_MODE_SENSOR_CHECK,
    NUEDC_2024_H_MODE_MOTOR_CHECK
} nuedc_2024_h_mode_t;

typedef enum {
    NUEDC_2024_H_POINT_NONE = 0,
    NUEDC_2024_H_POINT_A,
    NUEDC_2024_H_POINT_B,
    NUEDC_2024_H_POINT_C,
    NUEDC_2024_H_POINT_D
} nuedc_2024_h_point_t;

typedef enum {
    NUEDC_2024_H_DRIVE_IDLE = 0,
    NUEDC_2024_H_DRIVE_STRAIGHT,
    NUEDC_2024_H_DRIVE_LINE_FOLLOW,
    NUEDC_2024_H_DRIVE_STOP
} nuedc_2024_h_drive_mode_t;

typedef struct {
    uint8_t line_detected;
    line_trace_pattern_t line_pattern;
    int16_t line_error;
    int16_t yaw_cdeg;
    uint8_t checkpoint_event;
    uint8_t stop_event;
} nuedc_2024_h_inputs_t;

typedef struct {
    uint32_t magic;
    uint32_t loop_counter;
    uint32_t elapsed_ms;
    uint32_t segment_elapsed_ms;
    uint32_t deadline_ms;
    uint32_t checkpoint_count;
    nuedc_2024_h_mode_t mode;
    nuedc_2024_h_drive_mode_t drive_mode;
    nuedc_2024_h_point_t from_point;
    nuedc_2024_h_point_t to_point;
    uint8_t segment_index;
    uint8_t segment_count;
    uint8_t lap_index;
    uint8_t lap_count;
    uint8_t running;
    uint8_t done;
    uint8_t fault;
    uint8_t last_line_detected;
    int16_t heading_setpoint_cdeg;
    int16_t base_speed_permille;
    int16_t target_left_permille;
    int16_t target_right_permille;
} nuedc_2024_h_runtime_t;

typedef struct {
    nuedc_2024_h_runtime_t rt;
} nuedc_2024_h_app_t;

void Nuedc2024H_Init(nuedc_2024_h_app_t *app);
void Nuedc2024H_Start(nuedc_2024_h_app_t *app, nuedc_2024_h_mode_t mode);
void Nuedc2024H_Stop(nuedc_2024_h_app_t *app);
void Nuedc2024H_Update(nuedc_2024_h_app_t *app,
                       const nuedc_2024_h_inputs_t *inputs);
const nuedc_2024_h_runtime_t *Nuedc2024H_Runtime(const nuedc_2024_h_app_t *app);
const char *Nuedc2024H_ModeName(nuedc_2024_h_mode_t mode);
const char *Nuedc2024H_DriveName(nuedc_2024_h_drive_mode_t drive_mode);
const char *Nuedc2024H_PointName(nuedc_2024_h_point_t point);

#ifdef __cplusplus
}
#endif

#endif
