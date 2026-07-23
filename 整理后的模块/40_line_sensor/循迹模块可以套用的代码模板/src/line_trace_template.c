#include "line_trace_template.h"

#include <stddef.h>
#include <string.h>

#define LINE_TRACE_WEIGHT_LIMIT 200
#define LINE_TRACE_PARAM_K_MAX 2000
#define LINE_TRACE_PARAM_BASE_SPEED_MAX 5000
#define LINE_TRACE_PARAM_CORRECTION_MAX 10000
#define LINE_TRACE_PARAM_ERROR_MAX 1000
#define LINE_TRACE_PARAM_RAMP_MAX 5000
#define LINE_TRACE_RUNNING_WEIGHT_STEP_MAX 20
#define LINE_TRACE_RUNNING_THRESHOLD_STEP_MAX 100
#define LINE_TRACE_RUNNING_K_STEP_MAX 150
#define LINE_TRACE_RUNNING_SPEED_STEP_MAX 200
#define LINE_TRACE_RUNNING_CORRECTION_STEP_MAX 300
#define LINE_TRACE_RUNNING_RAMP_STEP_MAX 100

static int16_t clamp_i16(int32_t value, int16_t min_value, int16_t max_value)
{
    if (value > max_value) {
        return max_value;
    }
    if (value < min_value) {
        return min_value;
    }
    return (int16_t)value;
}

static int16_t ramp_i16(int16_t current, int16_t target, int16_t step)
{
    if (step <= 0) {
        return target;
    }
    if (current < target) {
        current = (int16_t)(current + step);
        return (current > target) ? target : current;
    }
    if (current > target) {
        current = (int16_t)(current - step);
        return (current < target) ? target : current;
    }
    return current;
}

static uint8_t popcount8(uint8_t value)
{
    uint8_t count = 0;
    while (value != 0u) {
        count = (uint8_t)(count + (value & 1u));
        value = (uint8_t)(value >> 1u);
    }
    return count;
}

static uint8_t channel_mask(uint8_t channel_count)
{
    if (channel_count >= LINE_TRACE_MAX_CHANNELS) {
        return 0xffu;
    }
    return (uint8_t)((1u << channel_count) - 1u);
}

static int16_t abs_i16(int16_t value)
{
    return (value < 0) ? (int16_t)(-value) : value;
}

static uint8_t abs_diff_i16_gt(int16_t a, int16_t b, int16_t limit)
{
    return (abs_i16((int16_t)(a - b)) > limit) ? 1u : 0u;
}

static uint8_t abs_diff_u16_gt(uint16_t a, uint16_t b, uint16_t limit)
{
    return (a > b) ? (((uint16_t)(a - b) > limit) ? 1u : 0u)
                   : (((uint16_t)(b - a) > limit) ? 1u : 0u);
}

static uint8_t bits_are_contiguous(uint8_t bits, uint8_t channel_count)
{
    uint8_t seen_one = 0;
    uint8_t seen_zero_after_one = 0;

    for (uint8_t i = 0; i < channel_count; ++i) {
        uint8_t active = (uint8_t)((bits >> i) & 1u);
        if (active != 0u) {
            if (seen_zero_after_one != 0u) {
                return 0u;
            }
            seen_one = 1u;
        } else if (seen_one != 0u) {
            seen_zero_after_one = 1u;
        }
    }
    return 1u;
}

static uint8_t normalized_enable_mask(uint8_t enable_mask, uint8_t channel_count)
{
    uint8_t mask = channel_mask(channel_count);
    if (enable_mask == 0u) {
        return mask;
    }
    return (uint8_t)(enable_mask & mask);
}

static void copy_algorithm_from_volatile(const volatile line_trace_algorithm_params_t *src,
                                         line_trace_algorithm_params_t *dst)
{
    for (uint8_t i = 0u; i < LINE_TRACE_MAX_CHANNELS; ++i) {
        dst->weights[i] = src->weights[i];
    }
    dst->kp = src->kp;
    dst->kd = src->kd;
    dst->base_speed = src->base_speed;
    dst->max_correction = src->max_correction;
    dst->corner_slow_error = src->corner_slow_error;
    dst->min_speed_percent = src->min_speed_percent;
    dst->search_speed = src->search_speed;
    dst->ramp_step = src->ramp_step;
}

static void copy_sensor_from_volatile(const volatile line_trace_sensor_params_t *src,
                                      line_trace_sensor_params_t *dst)
{
    dst->channel_count = src->channel_count;
    dst->active_level = src->active_level;
    for (uint8_t i = 0u; i < LINE_TRACE_MAX_CHANNELS; ++i) {
        dst->channel_order[i] = src->channel_order[i];
        dst->threshold[i] = src->threshold[i];
    }
    dst->threshold_mode = src->threshold_mode;
    dst->i2c_addr = src->i2c_addr;
    dst->sensor_enable_mask = src->sensor_enable_mask;
}

static line_trace_error_code_t validate_algorithm(const line_trace_algorithm_params_t *alg)
{
    if (alg == NULL) {
        return LINE_TRACE_ERROR_BAD_CONTROL_PARAM;
    }

    for (uint8_t i = 0u; i < LINE_TRACE_MAX_CHANNELS; ++i) {
        if ((alg->weights[i] < -LINE_TRACE_WEIGHT_LIMIT) ||
            (alg->weights[i] > LINE_TRACE_WEIGHT_LIMIT)) {
            return LINE_TRACE_ERROR_BAD_WEIGHT;
        }
    }

    if ((alg->kp < 0) || (alg->kp > LINE_TRACE_PARAM_K_MAX) ||
        (alg->kd < 0) || (alg->kd > LINE_TRACE_PARAM_K_MAX) ||
        (alg->base_speed < 0) || (alg->base_speed > LINE_TRACE_PARAM_BASE_SPEED_MAX) ||
        (alg->max_correction < 0) ||
        (alg->max_correction > LINE_TRACE_PARAM_CORRECTION_MAX) ||
        (alg->corner_slow_error < 0) ||
        (alg->corner_slow_error > LINE_TRACE_PARAM_ERROR_MAX) ||
        (alg->min_speed_percent < 0) || (alg->min_speed_percent > 100) ||
        (alg->search_speed < 0) || (alg->search_speed > LINE_TRACE_PARAM_BASE_SPEED_MAX) ||
        (alg->ramp_step < 0) || (alg->ramp_step > LINE_TRACE_PARAM_RAMP_MAX)) {
        return LINE_TRACE_ERROR_BAD_CONTROL_PARAM;
    }

    return LINE_TRACE_ERROR_OK;
}

static uint8_t channel_order_is_default(const line_trace_sensor_params_t *sensor)
{
    for (uint8_t i = 0u; i < sensor->channel_count; ++i) {
        if (sensor->channel_order[i] != 0u) {
            return 0u;
        }
    }
    return 1u;
}

static line_trace_error_code_t validate_sensor(const line_trace_sensor_params_t *sensor)
{
    uint8_t seen = 0u;

    if (sensor == NULL) {
        return LINE_TRACE_ERROR_SENSOR_FAULT;
    }
    if ((sensor->channel_count == 0u) ||
        (sensor->channel_count > LINE_TRACE_MAX_CHANNELS)) {
        return LINE_TRACE_ERROR_BAD_CHANNEL_COUNT;
    }
    if ((sensor->active_level != LINE_TRACE_ACTIVE_LOW) &&
        (sensor->active_level != LINE_TRACE_ACTIVE_HIGH)) {
        return LINE_TRACE_ERROR_BAD_ACTIVE_LEVEL;
    }
    if ((sensor->threshold_mode != LINE_TRACE_THRESHOLD_NONE) &&
        (sensor->threshold_mode != LINE_TRACE_THRESHOLD_DIGITAL_GPIO) &&
        (sensor->threshold_mode != LINE_TRACE_THRESHOLD_COMPARATOR) &&
        (sensor->threshold_mode != LINE_TRACE_THRESHOLD_I2C)) {
        return LINE_TRACE_ERROR_SENSOR_FAULT;
    }
    if ((sensor->sensor_enable_mask & (uint8_t)~channel_mask(sensor->channel_count)) != 0u) {
        return LINE_TRACE_ERROR_BAD_CHANNEL_COUNT;
    }
    if ((sensor->threshold_mode == LINE_TRACE_THRESHOLD_I2C) &&
        ((sensor->i2c_addr == 0u) || (sensor->i2c_addr > 0x7fu))) {
        return LINE_TRACE_ERROR_SENSOR_FAULT;
    }

    if (channel_order_is_default(sensor) != 0u) {
        return LINE_TRACE_ERROR_OK;
    }

    for (uint8_t i = 0u; i < sensor->channel_count; ++i) {
        uint8_t ch = sensor->channel_order[i];
        if (ch >= sensor->channel_count) {
            return LINE_TRACE_ERROR_BAD_CHANNEL_COUNT;
        }
        if ((seen & (uint8_t)(1u << ch)) != 0u) {
            return LINE_TRACE_ERROR_BAD_CHANNEL_COUNT;
        }
        seen = (uint8_t)(seen | (uint8_t)(1u << ch));
    }

    return LINE_TRACE_ERROR_OK;
}

static line_trace_error_code_t validate_running_change(
    const line_trace_algorithm_params_t *next_alg,
    const line_trace_sensor_params_t *next_sensor,
    const line_trace_algorithm_params_t *current_alg,
    const line_trace_sensor_params_t *current_sensor,
    line_trace_safety_state_t safety_state)
{
    if (safety_state == LINE_TRACE_SAFETY_RUNNING) {
        return LINE_TRACE_ERROR_UNSAFE_STATE;
    }
    if (safety_state == LINE_TRACE_SAFETY_FAULT) {
        return LINE_TRACE_ERROR_UNSAFE_STATE;
    }
    if (safety_state != LINE_TRACE_SAFETY_RUNNING_TUNE_SAFE) {
        return LINE_TRACE_ERROR_OK;
    }
    if ((current_alg == NULL) || (current_sensor == NULL)) {
        return LINE_TRACE_ERROR_BAD_CONTROL_PARAM;
    }

    if ((next_sensor->channel_count != current_sensor->channel_count) ||
        (next_sensor->active_level != current_sensor->active_level) ||
        (next_sensor->threshold_mode != current_sensor->threshold_mode) ||
        (next_sensor->i2c_addr != current_sensor->i2c_addr) ||
        (next_sensor->sensor_enable_mask != current_sensor->sensor_enable_mask)) {
        return LINE_TRACE_ERROR_DANGEROUS_JUMP;
    }

    for (uint8_t i = 0u; i < LINE_TRACE_MAX_CHANNELS; ++i) {
        if (next_sensor->channel_order[i] != current_sensor->channel_order[i]) {
            return LINE_TRACE_ERROR_DANGEROUS_JUMP;
        }
        if (abs_diff_i16_gt(next_alg->weights[i], current_alg->weights[i],
                            LINE_TRACE_RUNNING_WEIGHT_STEP_MAX) != 0u) {
            return LINE_TRACE_ERROR_DANGEROUS_JUMP;
        }
        if (abs_diff_u16_gt(next_sensor->threshold[i], current_sensor->threshold[i],
                            LINE_TRACE_RUNNING_THRESHOLD_STEP_MAX) != 0u) {
            return LINE_TRACE_ERROR_DANGEROUS_JUMP;
        }
    }

    if ((abs_diff_i16_gt(next_alg->kp, current_alg->kp, LINE_TRACE_RUNNING_K_STEP_MAX) != 0u) ||
        (abs_diff_i16_gt(next_alg->kd, current_alg->kd, LINE_TRACE_RUNNING_K_STEP_MAX) != 0u) ||
        (abs_diff_i16_gt(next_alg->base_speed, current_alg->base_speed,
                         LINE_TRACE_RUNNING_SPEED_STEP_MAX) != 0u) ||
        (abs_diff_i16_gt(next_alg->max_correction, current_alg->max_correction,
                         LINE_TRACE_RUNNING_CORRECTION_STEP_MAX) != 0u) ||
        (abs_diff_i16_gt(next_alg->search_speed, current_alg->search_speed,
                         LINE_TRACE_RUNNING_SPEED_STEP_MAX) != 0u) ||
        (abs_diff_i16_gt(next_alg->ramp_step, current_alg->ramp_step,
                         LINE_TRACE_RUNNING_RAMP_STEP_MAX) != 0u) ||
        (next_alg->corner_slow_error != current_alg->corner_slow_error) ||
        (next_alg->min_speed_percent != current_alg->min_speed_percent)) {
        return LINE_TRACE_ERROR_DANGEROUS_JUMP;
    }

    return LINE_TRACE_ERROR_OK;
}

void LineTrace_Init(line_trace_t *trace, const line_trace_config_t *cfg)
{
    if ((trace == NULL) || (cfg == NULL)) {
        return;
    }

    memset(trace, 0, sizeof(*trace));
    trace->cfg = *cfg;
    if (trace->cfg.channel_count > LINE_TRACE_MAX_CHANNELS) {
        trace->cfg.channel_count = LINE_TRACE_MAX_CHANNELS;
    }
    trace->cfg.sensor_enable_mask =
        normalized_enable_mask(trace->cfg.sensor_enable_mask, trace->cfg.channel_count);
}

uint8_t LineTrace_Update(line_trace_t *trace, line_trace_result_t *out)
{
    uint8_t raw_bits = 0u;
    line_trace_sample_frame_t frame;

    if ((trace == NULL) || (out == NULL) || (trace->cfg.read_level == NULL) ||
        (trace->cfg.weights == NULL) || (trace->cfg.channel_count == 0u)) {
        return 0u;
    }

    for (uint8_t i = 0u; i < trace->cfg.channel_count; ++i) {
        uint8_t high = trace->cfg.read_level(i, trace->cfg.user) ? 1u : 0u;
        if (high != 0u) {
            raw_bits = (uint8_t)(raw_bits | (uint8_t)(1u << i));
        }
    }

    memset(&frame, 0, sizeof(frame));
    frame.raw_bits = raw_bits;
    frame.channel_count = trace->cfg.channel_count;
    frame.active_level = trace->cfg.active_level;
    frame.sensor_enable_mask = trace->cfg.sensor_enable_mask;
    frame.sensor_status = LINE_TRACE_SENSOR_OK;

    return LineTrace_UpdateFromFrame(trace, &frame, out);
}

uint8_t LineTrace_BuildSampleFrame(const line_trace_sensor_params_t *sensor,
                                   uint8_t raw_physical_bits,
                                   uint8_t threshold_status,
                                   line_trace_sensor_status_t sensor_status,
                                   line_trace_sample_frame_t *out)
{
    uint8_t raw_logical_bits = 0u;

    if ((sensor == NULL) || (out == NULL) ||
        (validate_sensor(sensor) != LINE_TRACE_ERROR_OK)) {
        return 0u;
    }

    if (channel_order_is_default(sensor) != 0u) {
        raw_logical_bits = (uint8_t)(raw_physical_bits & channel_mask(sensor->channel_count));
    } else {
        for (uint8_t logical = 0u; logical < sensor->channel_count; ++logical) {
            uint8_t physical = sensor->channel_order[logical];
            if (((raw_physical_bits >> physical) & 1u) != 0u) {
                raw_logical_bits = (uint8_t)(raw_logical_bits | (uint8_t)(1u << logical));
            }
        }
    }

    memset(out, 0, sizeof(*out));
    out->raw_bits = raw_logical_bits;
    out->channel_count = sensor->channel_count;
    out->active_level = sensor->active_level;
    out->sensor_enable_mask = normalized_enable_mask(sensor->sensor_enable_mask,
                                                     sensor->channel_count);
    out->threshold_status = threshold_status;
    out->sensor_status = sensor_status;
    return 1u;
}

uint8_t LineTrace_UpdateFromFrame(line_trace_t *trace,
                                  const line_trace_sample_frame_t *frame,
                                  line_trace_result_t *out)
{
    uint8_t active_bits;
    uint8_t active_count;
    uint8_t count;
    uint8_t enable_mask;
    int32_t weighted_sum = 0;
    int16_t position = 0;

    if ((trace == NULL) || (frame == NULL) || (out == NULL) ||
        (trace->cfg.weights == NULL) || (frame->channel_count == 0u) ||
        (frame->channel_count > LINE_TRACE_MAX_CHANNELS) ||
        ((frame->active_level != LINE_TRACE_ACTIVE_LOW) &&
         (frame->active_level != LINE_TRACE_ACTIVE_HIGH)) ||
        (frame->sensor_status == LINE_TRACE_SENSOR_FAULT)) {
        return 0u;
    }

    memset(out, 0, sizeof(*out));
    trace->last_frame = *frame;
    count = frame->channel_count;
    enable_mask = normalized_enable_mask(frame->sensor_enable_mask, count);

    if (frame->active_level == LINE_TRACE_ACTIVE_HIGH) {
        active_bits = (uint8_t)(frame->raw_bits & enable_mask);
    } else {
        active_bits = (uint8_t)((uint8_t)~frame->raw_bits & enable_mask);
    }
    active_count = popcount8(active_bits);
    out->raw_bits = (uint8_t)(frame->raw_bits & channel_mask(count));
    out->active_bits = active_bits;
    out->active_count = active_count;
    out->threshold_status = frame->threshold_status;
    out->sensor_status = frame->sensor_status;

    if (active_count == 0u) {
        out->detected = 0u;
        out->position = trace->last_position;
        out->error = trace->last_error;
        out->lost_dir = trace->last_dir;
        out->pattern = LINE_TRACE_PATTERN_NO_LINE;
        return 1u;
    }

    for (uint8_t i = 0u; i < count; ++i) {
        if (((active_bits >> i) & 1u) != 0u) {
            weighted_sum += trace->cfg.weights[i];
        }
    }

    position = (int16_t)(weighted_sum / active_count);
    out->detected = 1u;
    out->position = position;
    out->error = position;
    out->confidence = (uint8_t)((active_count * 100u) / count);
    out->lost_dir = LINE_TRACE_LOST_NONE;

    if (active_count == popcount8(enable_mask)) {
        out->pattern = LINE_TRACE_PATTERN_ALL_ACTIVE;
    } else if (bits_are_contiguous(active_bits, count) == 0u) {
        out->pattern = LINE_TRACE_PATTERN_SPLIT;
    } else {
        out->pattern = LINE_TRACE_PATTERN_NORMAL;
    }

    trace->last_position = out->position;
    trace->last_error = out->error;
    trace->has_last = 1u;
    if (out->error < 0) {
        trace->last_dir = LINE_TRACE_LOST_LEFT;
    } else if (out->error > 0) {
        trace->last_dir = LINE_TRACE_LOST_RIGHT;
    }

    return 1u;
}

void LineTrace_ControllerInit(line_trace_controller_t *ctrl,
                              const line_trace_controller_config_t *cfg)
{
    if ((ctrl == NULL) || (cfg == NULL)) {
        return;
    }
    memset(ctrl, 0, sizeof(*ctrl));
    ctrl->cfg = *cfg;
}

void LineTrace_ControllerReset(line_trace_controller_t *ctrl)
{
    if (ctrl == NULL) {
        return;
    }
    ctrl->last_error = 0;
    ctrl->current_left = 0;
    ctrl->current_right = 0;
    ctrl->has_last_error = 0u;
}

void LineTrace_ControllerStep(line_trace_controller_t *ctrl,
                              const line_trace_result_t *line,
                              int16_t base_speed,
                              line_trace_motor_cmd_t *out)
{
    int16_t target_left = base_speed;
    int16_t target_right = base_speed;
    int16_t correction = 0;
    uint8_t searching = 0u;

    if ((ctrl == NULL) || (line == NULL) || (out == NULL)) {
        return;
    }

    if (line->detected != 0u) {
        int16_t derivative = ctrl->has_last_error ? (int16_t)(line->error - ctrl->last_error) : 0;
        int32_t raw_correction = ((int32_t)ctrl->cfg.kp * line->error) +
                                 ((int32_t)ctrl->cfg.kd * derivative);
        int16_t abs_error = (line->error < 0) ? (int16_t)(-line->error) : line->error;
        int16_t speed = base_speed;

        correction = clamp_i16(raw_correction, (int16_t)-ctrl->cfg.max_correction,
                               ctrl->cfg.max_correction);

        if ((ctrl->cfg.corner_slow_error > 0) && (abs_error > ctrl->cfg.corner_slow_error)) {
            int16_t min_speed = (int16_t)(((int32_t)base_speed * ctrl->cfg.min_speed_percent) / 100);
            speed = min_speed;
        }

        target_left = clamp_i16((int32_t)speed + correction, (int16_t)-32767, 32767);
        target_right = clamp_i16((int32_t)speed - correction, (int16_t)-32767, 32767);
        ctrl->last_error = line->error;
        ctrl->has_last_error = 1u;
    } else {
        searching = 1u;
        if (line->lost_dir == LINE_TRACE_LOST_LEFT) {
            target_left = (int16_t)-ctrl->cfg.search_speed;
            target_right = ctrl->cfg.search_speed;
        } else if (line->lost_dir == LINE_TRACE_LOST_RIGHT) {
            target_left = ctrl->cfg.search_speed;
            target_right = (int16_t)-ctrl->cfg.search_speed;
        } else {
            target_left = (int16_t)(base_speed / 2);
            target_right = (int16_t)(base_speed / 2);
        }
    }

    ctrl->current_left = ramp_i16(ctrl->current_left, target_left, ctrl->cfg.ramp_step);
    ctrl->current_right = ramp_i16(ctrl->current_right, target_right, ctrl->cfg.ramp_step);

    out->target_left = ctrl->current_left;
    out->target_right = ctrl->current_right;
    out->correction = correction;
    out->searching = searching;
}

void LineTrace_TuningBlockInit(volatile line_trace_tuning_block_t *block,
                               const line_trace_algorithm_params_t *algorithm,
                               const line_trace_sensor_params_t *sensor)
{
    if ((block == NULL) || (algorithm == NULL) || (sensor == NULL)) {
        return;
    }

    memset((void *)block, 0, sizeof(*block));
    block->magic = LINE_TRACE_TUNING_MAGIC;
    block->version = LINE_TRACE_TUNING_VERSION;
    block->size = (uint16_t)sizeof(*block);
    block->seq = 0u;
    block->algorithm = *algorithm;
    block->sensor = *sensor;
    block->status = LINE_TRACE_TUNING_IDLE;
    block->error_code = LINE_TRACE_ERROR_OK;
}

uint8_t LineTrace_ApplyTuningBlock(volatile line_trace_tuning_block_t *block,
                                   line_trace_t *trace,
                                   line_trace_controller_t *ctrl,
                                   line_trace_algorithm_params_t *runtime_algorithm,
                                   line_trace_sensor_params_t *runtime_sensor,
                                   line_trace_safety_state_t safety_state)
{
    line_trace_algorithm_params_t next_alg;
    line_trace_sensor_params_t next_sensor;
    line_trace_error_code_t err;

    if ((block == NULL) || (trace == NULL) || (ctrl == NULL) ||
        (runtime_algorithm == NULL) || (runtime_sensor == NULL)) {
        return 0u;
    }
    if (block->apply_flag != LINE_TRACE_APPLY_REQUEST) {
        return 0u;
    }

    block->status = LINE_TRACE_TUNING_PENDING;

    if (block->magic != LINE_TRACE_TUNING_MAGIC) {
        err = LINE_TRACE_ERROR_BAD_MAGIC;
    } else if (block->version != LINE_TRACE_TUNING_VERSION) {
        err = LINE_TRACE_ERROR_BAD_VERSION;
    } else {
        copy_algorithm_from_volatile(&block->algorithm, &next_alg);
        copy_sensor_from_volatile(&block->sensor, &next_sensor);
        err = validate_algorithm(&next_alg);
        if (err == LINE_TRACE_ERROR_OK) {
            err = validate_sensor(&next_sensor);
        }
        if (err == LINE_TRACE_ERROR_OK) {
            err = validate_running_change(&next_alg, &next_sensor, runtime_algorithm,
                                          runtime_sensor, safety_state);
        }
    }

    if (err != LINE_TRACE_ERROR_OK) {
        block->error_code = err;
        block->rejected_seq = block->seq;
        block->apply_flag = 0u;
        block->status = (err == LINE_TRACE_ERROR_DANGEROUS_JUMP)
                            ? LINE_TRACE_TUNING_ROLLED_BACK
                            : LINE_TRACE_TUNING_REJECTED;
        return 0u;
    }

    *runtime_algorithm = next_alg;
    *runtime_sensor = next_sensor;

    trace->cfg.channel_count = runtime_sensor->channel_count;
    trace->cfg.active_level = runtime_sensor->active_level;
    trace->cfg.weights = runtime_algorithm->weights;
    trace->cfg.sensor_enable_mask =
        normalized_enable_mask(runtime_sensor->sensor_enable_mask, runtime_sensor->channel_count);

    ctrl->cfg.kp = runtime_algorithm->kp;
    ctrl->cfg.kd = runtime_algorithm->kd;
    ctrl->cfg.max_correction = runtime_algorithm->max_correction;
    ctrl->cfg.corner_slow_error = runtime_algorithm->corner_slow_error;
    ctrl->cfg.min_speed_percent = runtime_algorithm->min_speed_percent;
    ctrl->cfg.search_speed = runtime_algorithm->search_speed;
    ctrl->cfg.ramp_step = runtime_algorithm->ramp_step;

    block->error_code = LINE_TRACE_ERROR_OK;
    block->applied_seq = block->seq;
    block->apply_flag = 0u;
    block->status = LINE_TRACE_TUNING_APPLIED;
    return 1u;
}

void LineTrace_FillTelemetry(const line_trace_result_t *line,
                             const line_trace_motor_cmd_t *cmd,
                             const volatile line_trace_tuning_block_t *block,
                             line_trace_safety_state_t safety_state,
                             uint32_t loop_counter,
                             line_trace_telemetry_t *out)
{
    if (out == NULL) {
        return;
    }

    memset(out, 0, sizeof(*out));
    if (line != NULL) {
        out->raw_bits = line->raw_bits;
        out->active_bits = line->active_bits;
        out->active_count = line->active_count;
        out->position = line->position;
        out->error = line->error;
        out->confidence = line->confidence;
        out->detected = line->detected;
        out->pattern = line->pattern;
        out->lost_dir = line->lost_dir;
        out->threshold_status = line->threshold_status;
        out->sensor_status = line->sensor_status;
    } else {
        out->sensor_status = LINE_TRACE_SENSOR_FAULT;
    }

    if (cmd != NULL) {
        out->target_left = cmd->target_left;
        out->target_right = cmd->target_right;
        out->correction = cmd->correction;
        out->searching = cmd->searching;
    }

    if (block != NULL) {
        out->tuning_status = (line_trace_tuning_status_t)block->status;
        out->error_code = (line_trace_error_code_t)block->error_code;
    } else {
        out->tuning_status = LINE_TRACE_TUNING_IDLE;
        out->error_code = LINE_TRACE_ERROR_OK;
    }
    out->safety_state = safety_state;
    out->loop_counter = loop_counter;
}
