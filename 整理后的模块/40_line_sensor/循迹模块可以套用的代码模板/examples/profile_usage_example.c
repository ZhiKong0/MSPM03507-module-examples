#include "line_trace_template.h"

#ifndef LINE_TRACE_PROFILE_HEADER
#define LINE_TRACE_PROFILE_HEADER "line_trace_profile_generated.h"
#endif

#include LINE_TRACE_PROFILE_HEADER

typedef struct {
    line_trace_t trace;
    line_trace_controller_t controller;
    line_trace_algorithm_params_t algorithm;
    line_trace_sensor_params_t sensor;
} line_trace_profile_runtime_t;

void LineTrace_ProfileRuntimeInit(line_trace_profile_runtime_t *runtime)
{
    line_trace_config_t line_cfg;
    line_trace_controller_config_t ctrl_cfg;

    if (runtime == 0) {
        return;
    }

    runtime->algorithm = kLineTraceProfileAlgorithm;
    runtime->sensor = kLineTraceProfileSensor;

    line_cfg.channel_count = runtime->sensor.channel_count;
    line_cfg.active_level = runtime->sensor.active_level;
    line_cfg.weights = runtime->algorithm.weights;
    line_cfg.read_level = 0;
    line_cfg.user = 0;
    line_cfg.sensor_enable_mask = runtime->sensor.sensor_enable_mask;
    LineTrace_Init(&runtime->trace, &line_cfg);

    ctrl_cfg.kp = runtime->algorithm.kp;
    ctrl_cfg.kd = runtime->algorithm.kd;
    ctrl_cfg.max_correction = runtime->algorithm.max_correction;
    ctrl_cfg.corner_slow_error = runtime->algorithm.corner_slow_error;
    ctrl_cfg.min_speed_percent = runtime->algorithm.min_speed_percent;
    ctrl_cfg.search_speed = runtime->algorithm.search_speed;
    ctrl_cfg.ramp_step = runtime->algorithm.ramp_step;
    LineTrace_ControllerInit(&runtime->controller, &ctrl_cfg);
}
