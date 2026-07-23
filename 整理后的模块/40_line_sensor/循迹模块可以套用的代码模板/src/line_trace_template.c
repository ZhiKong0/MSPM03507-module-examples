#include "line_trace_template.h"

#include <stddef.h>
#include <string.h>

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
}

uint8_t LineTrace_Update(line_trace_t *trace, line_trace_result_t *out)
{
    uint8_t raw_bits = 0u;
    uint8_t active_bits = 0u;
    uint8_t active_count;
    int32_t weighted_sum = 0;
    int16_t position = 0;

    if ((trace == NULL) || (out == NULL) || (trace->cfg.read_level == NULL) ||
        (trace->cfg.weights == NULL) || (trace->cfg.channel_count == 0u)) {
        return 0u;
    }

    memset(out, 0, sizeof(*out));

    for (uint8_t i = 0u; i < trace->cfg.channel_count; ++i) {
        uint8_t high = trace->cfg.read_level(i, trace->cfg.user) ? 1u : 0u;
        uint8_t active;

        if (high != 0u) {
            raw_bits = (uint8_t)(raw_bits | (uint8_t)(1u << i));
        }

        active = (trace->cfg.active_level == LINE_TRACE_ACTIVE_HIGH) ? high : (uint8_t)!high;
        if (active != 0u) {
            active_bits = (uint8_t)(active_bits | (uint8_t)(1u << i));
            weighted_sum += trace->cfg.weights[i];
        }
    }

    active_count = popcount8(active_bits);
    out->raw_bits = raw_bits;
    out->active_bits = active_bits;
    out->active_count = active_count;

    if (active_count == 0u) {
        out->detected = 0u;
        out->position = trace->last_position;
        out->error = trace->last_error;
        out->lost_dir = trace->last_dir;
        out->pattern = LINE_TRACE_PATTERN_NO_LINE;
        return 1u;
    }

    position = (int16_t)(weighted_sum / active_count);
    out->detected = 1u;
    out->position = position;
    out->error = position;
    out->confidence = (uint8_t)((active_count * 100u) / trace->cfg.channel_count);
    out->lost_dir = LINE_TRACE_LOST_NONE;

    if (active_count == trace->cfg.channel_count) {
        out->pattern = LINE_TRACE_PATTERN_ALL_ACTIVE;
    } else if (bits_are_contiguous(active_bits, trace->cfg.channel_count) == 0u) {
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
