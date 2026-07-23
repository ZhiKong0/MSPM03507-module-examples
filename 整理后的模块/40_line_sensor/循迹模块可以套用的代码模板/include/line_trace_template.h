#ifndef LINE_TRACE_TEMPLATE_H
#define LINE_TRACE_TEMPLATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_TRACE_MAX_CHANNELS 8u

typedef enum {
    LINE_TRACE_ACTIVE_LOW = 0,
    LINE_TRACE_ACTIVE_HIGH = 1
} line_trace_active_level_t;

typedef enum {
    LINE_TRACE_LOST_NONE = 0,
    LINE_TRACE_LOST_LEFT = -1,
    LINE_TRACE_LOST_RIGHT = 1
} line_trace_lost_dir_t;

typedef enum {
    LINE_TRACE_PATTERN_NO_LINE = 0,
    LINE_TRACE_PATTERN_NORMAL = 1,
    LINE_TRACE_PATTERN_ALL_ACTIVE = 2,
    LINE_TRACE_PATTERN_SPLIT = 3
} line_trace_pattern_t;

typedef uint8_t (*line_trace_read_level_fn)(uint8_t channel, void *user);

typedef struct {
    uint8_t channel_count;
    line_trace_active_level_t active_level;
    const int16_t *weights;
    line_trace_read_level_fn read_level;
    void *user;
} line_trace_config_t;

typedef struct {
    uint8_t raw_bits;
    uint8_t active_bits;
    uint8_t active_count;
    int16_t position;
    int16_t error;
    uint8_t confidence;
    uint8_t detected;
    line_trace_lost_dir_t lost_dir;
    line_trace_pattern_t pattern;
} line_trace_result_t;

typedef struct {
    line_trace_config_t cfg;
    int16_t last_position;
    int16_t last_error;
    line_trace_lost_dir_t last_dir;
    uint8_t has_last;
} line_trace_t;

typedef struct {
    int16_t kp;
    int16_t kd;
    int16_t max_correction;
    int16_t corner_slow_error;
    int16_t min_speed_percent;
    int16_t search_speed;
    int16_t ramp_step;
} line_trace_controller_config_t;

typedef struct {
    line_trace_controller_config_t cfg;
    int16_t last_error;
    int16_t current_left;
    int16_t current_right;
    uint8_t has_last_error;
} line_trace_controller_t;

typedef struct {
    int16_t target_left;
    int16_t target_right;
    int16_t correction;
    uint8_t searching;
} line_trace_motor_cmd_t;

void LineTrace_Init(line_trace_t *trace, const line_trace_config_t *cfg);
uint8_t LineTrace_Update(line_trace_t *trace, line_trace_result_t *out);
void LineTrace_ControllerInit(line_trace_controller_t *ctrl,
                              const line_trace_controller_config_t *cfg);
void LineTrace_ControllerReset(line_trace_controller_t *ctrl);
void LineTrace_ControllerStep(line_trace_controller_t *ctrl,
                              const line_trace_result_t *line,
                              int16_t base_speed,
                              line_trace_motor_cmd_t *out);

#ifdef __cplusplus
}
#endif

#endif
