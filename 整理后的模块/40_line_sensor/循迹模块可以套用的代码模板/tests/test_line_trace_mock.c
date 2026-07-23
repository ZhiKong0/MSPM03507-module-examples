#include "line_trace_template.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int16_t k_weights8[LINE_TRACE_MAX_CHANNELS] = {
    -35, -25, -15, -5, 5, 15, 25, 35
};

static void require_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void init_trace(line_trace_t *trace)
{
    const line_trace_config_t cfg = {
        .channel_count = 8u,
        .active_level = LINE_TRACE_ACTIVE_HIGH,
        .weights = k_weights8,
        .read_level = 0,
        .user = 0,
        .sensor_enable_mask = 0xffu,
    };

    LineTrace_Init(trace, &cfg);
}

static line_trace_sample_frame_t make_frame(uint8_t raw_bits,
                                            line_trace_active_level_t active_level)
{
    line_trace_sample_frame_t frame;

    memset(&frame, 0, sizeof(frame));
    frame.raw_bits = raw_bits;
    frame.channel_count = 8u;
    frame.active_level = active_level;
    frame.sensor_enable_mask = 0xffu;
    frame.sensor_status = LINE_TRACE_SENSOR_OK;
    return frame;
}

static void test_algorithm_cases(void)
{
    line_trace_t trace;
    line_trace_result_t line;
    line_trace_sample_frame_t frame;

    init_trace(&trace);

    frame = make_frame(0x00u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "no_line update");
    require_true(line.detected == 0u, "no_line detected");
    require_true(line.pattern == LINE_TRACE_PATTERN_NO_LINE, "no_line pattern");

    frame = make_frame(0x01u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "single_left update");
    require_true(line.error < 0, "single_left negative error");
    require_true(line.lost_dir == LINE_TRACE_LOST_NONE, "single_left current lost none");

    frame = make_frame(0x80u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "single_right update");
    require_true(line.error > 0, "single_right positive error");

    frame = make_frame(0x18u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "center_pair update");
    require_true(line.error == 0, "center_pair zero error");
    require_true(line.pattern == LINE_TRACE_PATTERN_NORMAL, "center_pair normal pattern");

    frame = make_frame(0xffu, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "all_active update");
    require_true(line.pattern == LINE_TRACE_PATTERN_ALL_ACTIVE, "all_active pattern");

    frame = make_frame(0x81u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "split update");
    require_true(line.pattern == LINE_TRACE_PATTERN_SPLIT, "split pattern");

    frame = make_frame(0x80u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "edge before lost");
    frame = make_frame(0x00u, LINE_TRACE_ACTIVE_HIGH);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "edge lost");
    require_true(line.detected == 0u, "edge_lost detected false");
    require_true(line.lost_dir == LINE_TRACE_LOST_RIGHT, "edge_lost keeps right direction");
    require_true(line.error > 0, "edge_lost keeps last error");

    frame = make_frame((uint8_t)(0xffu & (uint8_t)~(1u << 3u)), LINE_TRACE_ACTIVE_LOW);
    require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u, "active_low update");
    require_true(line.active_bits == (uint8_t)(1u << 3u), "active_low maps channel 3");

    {
        line_trace_sensor_params_t sensor;

        memset(&sensor, 0, sizeof(sensor));
        sensor.channel_count = 8u;
        sensor.active_level = LINE_TRACE_ACTIVE_HIGH;
        sensor.threshold_mode = LINE_TRACE_THRESHOLD_DIGITAL_GPIO;
        sensor.sensor_enable_mask = 0xffu;
        sensor.channel_order[0] = 7u;
        sensor.channel_order[1] = 6u;
        sensor.channel_order[2] = 5u;
        sensor.channel_order[3] = 4u;
        sensor.channel_order[4] = 3u;
        sensor.channel_order[5] = 2u;
        sensor.channel_order[6] = 1u;
        sensor.channel_order[7] = 0u;
        require_true(LineTrace_BuildSampleFrame(&sensor, 0x80u, 0u,
                                                LINE_TRACE_SENSOR_OK, &frame) == 1u,
                     "channel_order build frame");
        require_true(frame.raw_bits == 0x01u, "channel_order maps physical 7 to logical 0");
        require_true(LineTrace_UpdateFromFrame(&trace, &frame, &line) == 1u,
                     "channel_order update");
        require_true(line.error < 0, "channel_order left after remap");
    }
}

static void init_runtime(line_trace_algorithm_params_t *alg,
                         line_trace_sensor_params_t *sensor,
                         line_trace_t *trace,
                         line_trace_controller_t *ctrl)
{
    line_trace_config_t line_cfg;
    line_trace_controller_config_t ctrl_cfg;

    memset(alg, 0, sizeof(*alg));
    memset(sensor, 0, sizeof(*sensor));

    for (uint8_t i = 0u; i < LINE_TRACE_MAX_CHANNELS; ++i) {
        alg->weights[i] = k_weights8[i];
    }
    alg->kp = 12;
    alg->kd = 4;
    alg->base_speed = 260;
    alg->max_correction = 500;
    alg->corner_slow_error = 25;
    alg->min_speed_percent = 65;
    alg->search_speed = 180;
    alg->ramp_step = 10;

    sensor->channel_count = 8u;
    sensor->active_level = LINE_TRACE_ACTIVE_LOW;
    sensor->threshold_mode = LINE_TRACE_THRESHOLD_DIGITAL_GPIO;
    sensor->sensor_enable_mask = 0xffu;

    memset(&line_cfg, 0, sizeof(line_cfg));
    line_cfg.channel_count = sensor->channel_count;
    line_cfg.active_level = sensor->active_level;
    line_cfg.weights = alg->weights;
    line_cfg.sensor_enable_mask = sensor->sensor_enable_mask;
    LineTrace_Init(trace, &line_cfg);

    memset(&ctrl_cfg, 0, sizeof(ctrl_cfg));
    ctrl_cfg.kp = alg->kp;
    ctrl_cfg.kd = alg->kd;
    ctrl_cfg.max_correction = alg->max_correction;
    ctrl_cfg.corner_slow_error = alg->corner_slow_error;
    ctrl_cfg.min_speed_percent = alg->min_speed_percent;
    ctrl_cfg.search_speed = alg->search_speed;
    ctrl_cfg.ramp_step = alg->ramp_step;
    LineTrace_ControllerInit(ctrl, &ctrl_cfg);
}

static void test_tuning_block(void)
{
    line_trace_algorithm_params_t alg;
    line_trace_sensor_params_t sensor;
    line_trace_t trace;
    line_trace_controller_t ctrl;
    volatile line_trace_tuning_block_t block;

    init_runtime(&alg, &sensor, &trace, &ctrl);
    LineTrace_TuningBlockInit(&block, &alg, &sensor);

    block.seq = 1u;
    block.algorithm.kp = 20;
    block.algorithm.base_speed = 300;
    block.apply_flag = LINE_TRACE_APPLY_REQUEST;
    require_true(LineTrace_ApplyTuningBlock(&block, &trace, &ctrl, &alg, &sensor,
                                            LINE_TRACE_SAFETY_ARMED) == 1u,
                 "valid tuning apply");
    require_true(block.status == LINE_TRACE_TUNING_APPLIED, "valid tuning status");
    require_true(block.error_code == LINE_TRACE_ERROR_OK, "valid tuning error");
    require_true(alg.kp == 20, "runtime kp updated");
    require_true(alg.base_speed == 300, "runtime base speed updated");

    block.seq = 2u;
    block.algorithm.kp = 3000;
    block.apply_flag = LINE_TRACE_APPLY_REQUEST;
    require_true(LineTrace_ApplyTuningBlock(&block, &trace, &ctrl, &alg, &sensor,
                                            LINE_TRACE_SAFETY_ARMED) == 0u,
                 "out of range tuning reject");
    require_true(block.status == LINE_TRACE_TUNING_REJECTED, "reject status");
    require_true(block.error_code == LINE_TRACE_ERROR_BAD_CONTROL_PARAM,
                 "reject error code");

    block.algorithm = alg;
    block.sensor = sensor;
    block.seq = 3u;
    block.algorithm.kp = 21;
    block.apply_flag = LINE_TRACE_APPLY_REQUEST;
    require_true(LineTrace_ApplyTuningBlock(&block, &trace, &ctrl, &alg, &sensor,
                                            LINE_TRACE_SAFETY_RUNNING) == 0u,
                 "running unsafe tuning reject");
    require_true(block.error_code == LINE_TRACE_ERROR_UNSAFE_STATE,
                 "running unsafe error code");

    block.algorithm = alg;
    block.sensor = sensor;
    block.seq = 4u;
    block.sensor.active_level = LINE_TRACE_ACTIVE_HIGH;
    block.apply_flag = LINE_TRACE_APPLY_REQUEST;
    require_true(LineTrace_ApplyTuningBlock(&block, &trace, &ctrl, &alg, &sensor,
                                            LINE_TRACE_SAFETY_RUNNING_TUNE_SAFE) == 0u,
                 "running cold param rollback");
    require_true(block.status == LINE_TRACE_TUNING_ROLLED_BACK,
                 "running cold param rollback status");
    require_true(block.error_code == LINE_TRACE_ERROR_DANGEROUS_JUMP,
                 "running cold param dangerous jump error");
}

int main(void)
{
    test_algorithm_cases();
    test_tuning_block();
    puts("line_trace_mock_tests: PASS");
    return 0;
}
