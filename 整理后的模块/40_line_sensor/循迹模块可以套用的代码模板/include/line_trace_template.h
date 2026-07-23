#ifndef LINE_TRACE_TEMPLATE_H
#define LINE_TRACE_TEMPLATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_TRACE_MAX_CHANNELS 8u
#define LINE_TRACE_TUNING_MAGIC 0x4C54554Eu
#define LINE_TRACE_TUNING_VERSION 1u
#define LINE_TRACE_APPLY_REQUEST 1u
#define LINE_TRACE_BENCH_MAGIC 0x4C54424Eu
#define LINE_TRACE_BENCH_VERSION 1u

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

typedef enum {
    LINE_TRACE_SENSOR_OK = 0,
    LINE_TRACE_SENSOR_STALE = 1,
    LINE_TRACE_SENSOR_FAULT = 2
} line_trace_sensor_status_t;

typedef enum {
    LINE_TRACE_THRESHOLD_NONE = 0,
    LINE_TRACE_THRESHOLD_DIGITAL_GPIO = 1,
    LINE_TRACE_THRESHOLD_COMPARATOR = 2,
    LINE_TRACE_THRESHOLD_I2C = 3
} line_trace_threshold_mode_t;

typedef enum {
    LINE_TRACE_TUNING_IDLE = 0,
    LINE_TRACE_TUNING_PENDING = 1,
    LINE_TRACE_TUNING_APPLIED = 2,
    LINE_TRACE_TUNING_REJECTED = 3,
    LINE_TRACE_TUNING_ROLLED_BACK = 4
} line_trace_tuning_status_t;

typedef enum {
    LINE_TRACE_ERROR_OK = 0,
    LINE_TRACE_ERROR_BAD_MAGIC = 1,
    LINE_TRACE_ERROR_BAD_VERSION = 2,
    LINE_TRACE_ERROR_BAD_CHANNEL_COUNT = 3,
    LINE_TRACE_ERROR_BAD_ACTIVE_LEVEL = 4,
    LINE_TRACE_ERROR_BAD_WEIGHT = 5,
    LINE_TRACE_ERROR_BAD_CONTROL_PARAM = 6,
    LINE_TRACE_ERROR_DANGEROUS_JUMP = 7,
    LINE_TRACE_ERROR_UNSAFE_STATE = 8,
    LINE_TRACE_ERROR_SENSOR_FAULT = 9
} line_trace_error_code_t;

typedef enum {
    LINE_TRACE_SAFETY_DISARMED = 0,
    LINE_TRACE_SAFETY_ARMED = 1,
    LINE_TRACE_SAFETY_RUNNING = 2,
    LINE_TRACE_SAFETY_RUNNING_TUNE_SAFE = 3,
    LINE_TRACE_SAFETY_FAULT = 4
} line_trace_safety_state_t;

typedef uint8_t (*line_trace_read_level_fn)(uint8_t channel, void *user);

typedef struct {
    uint8_t raw_bits;
    uint8_t channel_count;
    line_trace_active_level_t active_level;
    uint8_t sensor_enable_mask;
    uint8_t threshold_status;
    line_trace_sensor_status_t sensor_status;
} line_trace_sample_frame_t;

typedef struct {
    uint8_t channel_count;
    line_trace_active_level_t active_level;
    const int16_t *weights;
    line_trace_read_level_fn read_level;
    void *user;
    uint8_t sensor_enable_mask;
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
    uint8_t threshold_status;
    line_trace_sensor_status_t sensor_status;
} line_trace_result_t;

typedef struct {
    line_trace_config_t cfg;
    int16_t last_position;
    int16_t last_error;
    line_trace_lost_dir_t last_dir;
    uint8_t has_last;
    line_trace_sample_frame_t last_frame;
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
    int16_t weights[LINE_TRACE_MAX_CHANNELS];
    int16_t kp;
    int16_t kd;
    int16_t base_speed;
    int16_t max_correction;
    int16_t corner_slow_error;
    int16_t min_speed_percent;
    int16_t search_speed;
    int16_t ramp_step;
} line_trace_algorithm_params_t;

typedef struct {
    uint8_t channel_count;
    line_trace_active_level_t active_level;
    uint8_t channel_order[LINE_TRACE_MAX_CHANNELS];
    uint16_t threshold[LINE_TRACE_MAX_CHANNELS];
    line_trace_threshold_mode_t threshold_mode;
    uint8_t i2c_addr;
    uint8_t sensor_enable_mask;
} line_trace_sensor_params_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t seq;
    line_trace_algorithm_params_t algorithm;
    line_trace_sensor_params_t sensor;
    volatile uint32_t apply_flag;
    volatile uint32_t status;
    volatile uint32_t error_code;
    volatile uint32_t applied_seq;
    volatile uint32_t rejected_seq;
} line_trace_tuning_block_t;

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

typedef struct {
    uint8_t raw_bits;
    uint8_t active_bits;
    uint8_t active_count;
    int16_t position;
    int16_t error;
    uint8_t confidence;
    uint8_t detected;
    line_trace_pattern_t pattern;
    line_trace_lost_dir_t lost_dir;
    int16_t target_left;
    int16_t target_right;
    int16_t correction;
    uint8_t searching;
    uint8_t threshold_status;
    line_trace_sensor_status_t sensor_status;
    line_trace_tuning_status_t tuning_status;
    line_trace_error_code_t error_code;
    line_trace_safety_state_t safety_state;
    uint32_t loop_counter;
} line_trace_telemetry_t;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t size_words;
    uint32_t loop_counter;
    uint32_t raw_bits;
    uint32_t active_bits;
    uint32_t active_count;
    int32_t position;
    int32_t error;
    uint32_t confidence;
    uint32_t detected;
    uint32_t pattern;
    int32_t lost_dir;
    int32_t target_left;
    int32_t target_right;
    int32_t correction;
    uint32_t searching;
    uint32_t threshold_status;
    uint32_t sensor_status;
    uint32_t tuning_status;
    uint32_t error_code;
    uint32_t safety_state;
} line_trace_bench_snapshot_t;

void LineTrace_Init(line_trace_t *trace, const line_trace_config_t *cfg);
uint8_t LineTrace_Update(line_trace_t *trace, line_trace_result_t *out);
uint8_t LineTrace_BuildSampleFrame(const line_trace_sensor_params_t *sensor,
                                   uint8_t raw_physical_bits,
                                   uint8_t threshold_status,
                                   line_trace_sensor_status_t sensor_status,
                                   line_trace_sample_frame_t *out);
uint8_t LineTrace_UpdateFromFrame(line_trace_t *trace,
                                  const line_trace_sample_frame_t *frame,
                                  line_trace_result_t *out);
void LineTrace_ControllerInit(line_trace_controller_t *ctrl,
                              const line_trace_controller_config_t *cfg);
void LineTrace_ControllerReset(line_trace_controller_t *ctrl);
void LineTrace_ControllerStep(line_trace_controller_t *ctrl,
                              const line_trace_result_t *line,
                              int16_t base_speed,
                              line_trace_motor_cmd_t *out);
void LineTrace_TuningBlockInit(volatile line_trace_tuning_block_t *block,
                               const line_trace_algorithm_params_t *algorithm,
                               const line_trace_sensor_params_t *sensor);
uint8_t LineTrace_ApplyTuningBlock(volatile line_trace_tuning_block_t *block,
                                   line_trace_t *trace,
                                   line_trace_controller_t *ctrl,
                                   line_trace_algorithm_params_t *runtime_algorithm,
                                   line_trace_sensor_params_t *runtime_sensor,
                                   line_trace_safety_state_t safety_state);
void LineTrace_FillTelemetry(const line_trace_result_t *line,
                             const line_trace_motor_cmd_t *cmd,
                             const volatile line_trace_tuning_block_t *block,
                             line_trace_safety_state_t safety_state,
                             uint32_t loop_counter,
                             line_trace_telemetry_t *out);
void LineTrace_FillBenchSnapshot(const line_trace_telemetry_t *telemetry,
                                 line_trace_bench_snapshot_t *out);

#ifdef __cplusplus
}
#endif

#endif
