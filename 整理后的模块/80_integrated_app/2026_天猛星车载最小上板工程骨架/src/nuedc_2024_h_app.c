#include "nuedc_2024_h_app.h"

#include <stddef.h>
#include <string.h>

#define H_BASE_SPEED_DEFAULT 140
#define H_BASE_SPEED_MOTOR_CHECK 60
#define H_HEADING_KP_DIVISOR 40
#define H_CHECKPOINT_MIN_MS 400u

typedef struct {
    nuedc_2024_h_point_t from;
    nuedc_2024_h_point_t to;
    nuedc_2024_h_drive_mode_t drive_mode;
    uint16_t min_ms;
    uint16_t timeout_ms;
    int16_t heading_cdeg;
} h_segment_t;

static const h_segment_t g_route_ab[] = {
    { NUEDC_2024_H_POINT_A, NUEDC_2024_H_POINT_B,
      NUEDC_2024_H_DRIVE_STRAIGHT, 800u, 15000u, 0 },
};

static const h_segment_t g_route_loop[] = {
    { NUEDC_2024_H_POINT_A, NUEDC_2024_H_POINT_B,
      NUEDC_2024_H_DRIVE_STRAIGHT, 800u, 15000u, 0 },
    { NUEDC_2024_H_POINT_B, NUEDC_2024_H_POINT_C,
      NUEDC_2024_H_DRIVE_LINE_FOLLOW, 800u, 9000u, 0 },
    { NUEDC_2024_H_POINT_C, NUEDC_2024_H_POINT_D,
      NUEDC_2024_H_DRIVE_STRAIGHT, 800u, 15000u, 0 },
    { NUEDC_2024_H_POINT_D, NUEDC_2024_H_POINT_A,
      NUEDC_2024_H_DRIVE_LINE_FOLLOW, 800u, 9000u, 0 },
};

static const h_segment_t g_route_figure8[] = {
    { NUEDC_2024_H_POINT_A, NUEDC_2024_H_POINT_C,
      NUEDC_2024_H_DRIVE_LINE_FOLLOW, 800u, 12000u, 0 },
    { NUEDC_2024_H_POINT_C, NUEDC_2024_H_POINT_B,
      NUEDC_2024_H_DRIVE_LINE_FOLLOW, 800u, 12000u, 0 },
    { NUEDC_2024_H_POINT_B, NUEDC_2024_H_POINT_D,
      NUEDC_2024_H_DRIVE_LINE_FOLLOW, 800u, 12000u, 0 },
    { NUEDC_2024_H_POINT_D, NUEDC_2024_H_POINT_A,
      NUEDC_2024_H_DRIVE_LINE_FOLLOW, 800u, 12000u, 0 },
};

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

static int16_t heading_error_cdeg(int16_t target, int16_t current)
{
    int32_t err = (int32_t)target - current;

    while (err > 18000) {
        err -= 36000;
    }
    while (err < -18000) {
        err += 36000;
    }
    return (int16_t)err;
}

static const h_segment_t *route_for_mode(nuedc_2024_h_mode_t mode, uint8_t *count)
{
    if (count == 0) {
        return 0;
    }

    switch (mode) {
    case NUEDC_2024_H_MODE_BASIC_AB:
        *count = (uint8_t)(sizeof(g_route_ab) / sizeof(g_route_ab[0]));
        return g_route_ab;
    case NUEDC_2024_H_MODE_BASIC_LOOP:
        *count = (uint8_t)(sizeof(g_route_loop) / sizeof(g_route_loop[0]));
        return g_route_loop;
    case NUEDC_2024_H_MODE_FIGURE8:
    case NUEDC_2024_H_MODE_FIGURE8_4LAP:
        *count = (uint8_t)(sizeof(g_route_figure8) / sizeof(g_route_figure8[0]));
        return g_route_figure8;
    default:
        *count = 0u;
        return 0;
    }
}

static uint32_t deadline_for_mode(nuedc_2024_h_mode_t mode)
{
    switch (mode) {
    case NUEDC_2024_H_MODE_BASIC_AB:
        return 15000u;
    case NUEDC_2024_H_MODE_BASIC_LOOP:
        return 30000u;
    case NUEDC_2024_H_MODE_FIGURE8:
        return 40000u;
    case NUEDC_2024_H_MODE_FIGURE8_4LAP:
        return 160000u;
    case NUEDC_2024_H_MODE_MOTOR_CHECK:
        return 1200u;
    default:
        return 0u;
    }
}

static uint8_t lap_count_for_mode(nuedc_2024_h_mode_t mode)
{
    return (mode == NUEDC_2024_H_MODE_FIGURE8_4LAP) ? 4u : 1u;
}

static void load_segment(nuedc_2024_h_app_t *app)
{
    uint8_t count = 0u;
    const h_segment_t *route = route_for_mode(app->rt.mode, &count);
    const h_segment_t *seg;

    app->rt.segment_count = count;
    app->rt.segment_elapsed_ms = 0u;

    if ((route == 0) || (count == 0u) || (app->rt.segment_index >= count)) {
        app->rt.drive_mode = NUEDC_2024_H_DRIVE_IDLE;
        app->rt.from_point = NUEDC_2024_H_POINT_NONE;
        app->rt.to_point = NUEDC_2024_H_POINT_NONE;
        return;
    }

    seg = &route[app->rt.segment_index];
    app->rt.drive_mode = seg->drive_mode;
    app->rt.from_point = seg->from;
    app->rt.to_point = seg->to;
    app->rt.heading_setpoint_cdeg = seg->heading_cdeg;
}

static uint8_t current_segment_finished(const nuedc_2024_h_app_t *app,
                                        const nuedc_2024_h_inputs_t *inputs)
{
    uint8_t count = 0u;
    const h_segment_t *route = route_for_mode(app->rt.mode, &count);
    uint8_t line_rise;
    uint8_t pattern_checkpoint;

    if ((route == 0) || (inputs == 0) || (app->rt.segment_index >= count)) {
        return 0u;
    }
    if (app->rt.segment_elapsed_ms < route[app->rt.segment_index].min_ms) {
        return 0u;
    }
    if (app->rt.segment_elapsed_ms < H_CHECKPOINT_MIN_MS) {
        return 0u;
    }

    line_rise = ((inputs->line_detected != 0u) &&
                 (app->rt.last_line_detected == 0u)) ? 1u : 0u;
    pattern_checkpoint = ((inputs->line_pattern == LINE_TRACE_PATTERN_ALL_ACTIVE) ||
                          (inputs->line_pattern == LINE_TRACE_PATTERN_SPLIT)) ? 1u : 0u;

    return ((inputs->checkpoint_event != 0u) ||
            (line_rise != 0u) ||
            (pattern_checkpoint != 0u)) ? 1u : 0u;
}

static uint8_t current_segment_timed_out(const nuedc_2024_h_app_t *app)
{
    uint8_t count = 0u;
    const h_segment_t *route = route_for_mode(app->rt.mode, &count);

    if ((route == 0) || (app->rt.segment_index >= count)) {
        return 0u;
    }
    return (app->rt.segment_elapsed_ms > route[app->rt.segment_index].timeout_ms) ? 1u : 0u;
}

static void advance_segment(nuedc_2024_h_app_t *app)
{
    app->rt.checkpoint_count++;
    app->rt.segment_index++;
    if (app->rt.segment_index >= app->rt.segment_count) {
        app->rt.segment_index = 0u;
        app->rt.lap_index++;
        if (app->rt.lap_index >= app->rt.lap_count) {
            app->rt.running = 0u;
            app->rt.done = 1u;
            app->rt.drive_mode = NUEDC_2024_H_DRIVE_STOP;
            app->rt.target_left_permille = 0;
            app->rt.target_right_permille = 0;
            return;
        }
    }
    load_segment(app);
}

static void update_drive_command(nuedc_2024_h_app_t *app,
                                 const nuedc_2024_h_inputs_t *inputs)
{
    int16_t base = app->rt.base_speed_permille;
    int16_t correction = 0;

    if (app->rt.running == 0u) {
        app->rt.target_left_permille = 0;
        app->rt.target_right_permille = 0;
        return;
    }

    if (app->rt.drive_mode == NUEDC_2024_H_DRIVE_STRAIGHT) {
        correction = (int16_t)(heading_error_cdeg(app->rt.heading_setpoint_cdeg,
                                                  inputs->yaw_cdeg) /
                               H_HEADING_KP_DIVISOR);
        app->rt.target_left_permille =
            clamp_i16((int32_t)base - correction, -250, 250);
        app->rt.target_right_permille =
            clamp_i16((int32_t)base + correction, -250, 250);
    } else if (app->rt.drive_mode == NUEDC_2024_H_DRIVE_LINE_FOLLOW) {
        app->rt.target_left_permille = base;
        app->rt.target_right_permille = base;
    } else {
        app->rt.target_left_permille = 0;
        app->rt.target_right_permille = 0;
    }
}

void Nuedc2024H_Init(nuedc_2024_h_app_t *app)
{
    if (app == 0) {
        return;
    }

    memset(app, 0, sizeof(*app));
    app->rt.magic = NUEDC_2024_H_MAGIC;
    app->rt.mode = NUEDC_2024_H_MODE_IDLE;
    app->rt.drive_mode = NUEDC_2024_H_DRIVE_IDLE;
}

void Nuedc2024H_Start(nuedc_2024_h_app_t *app, nuedc_2024_h_mode_t mode)
{
    uint8_t count = 0u;

    if (app == 0) {
        return;
    }

    memset(&app->rt, 0, sizeof(app->rt));
    app->rt.magic = NUEDC_2024_H_MAGIC;
    app->rt.mode = mode;
    app->rt.deadline_ms = deadline_for_mode(mode);
    app->rt.lap_count = lap_count_for_mode(mode);
    app->rt.base_speed_permille =
        (mode == NUEDC_2024_H_MODE_MOTOR_CHECK) ? H_BASE_SPEED_MOTOR_CHECK
                                                : H_BASE_SPEED_DEFAULT;

    if (mode == NUEDC_2024_H_MODE_SENSOR_CHECK) {
        app->rt.running = 1u;
        app->rt.drive_mode = NUEDC_2024_H_DRIVE_IDLE;
        return;
    }

    if (mode == NUEDC_2024_H_MODE_MOTOR_CHECK) {
        app->rt.running = 1u;
        app->rt.drive_mode = NUEDC_2024_H_DRIVE_STRAIGHT;
        app->rt.segment_count = 1u;
        app->rt.deadline_ms = 1200u;
        return;
    }

    (void)route_for_mode(mode, &count);
    if (count == 0u) {
        app->rt.mode = NUEDC_2024_H_MODE_IDLE;
        app->rt.drive_mode = NUEDC_2024_H_DRIVE_IDLE;
        return;
    }

    app->rt.running = 1u;
    load_segment(app);
}

void Nuedc2024H_Stop(nuedc_2024_h_app_t *app)
{
    if (app == 0) {
        return;
    }
    app->rt.running = 0u;
    app->rt.drive_mode = NUEDC_2024_H_DRIVE_STOP;
    app->rt.target_left_permille = 0;
    app->rt.target_right_permille = 0;
}

void Nuedc2024H_Update(nuedc_2024_h_app_t *app,
                       const nuedc_2024_h_inputs_t *inputs)
{
    nuedc_2024_h_inputs_t empty_inputs;

    if (app == 0) {
        return;
    }
    if (inputs == 0) {
        memset(&empty_inputs, 0, sizeof(empty_inputs));
        inputs = &empty_inputs;
    }

    app->rt.loop_counter++;

    if (app->rt.running == 0u) {
        update_drive_command(app, inputs);
        app->rt.last_line_detected = inputs->line_detected;
        return;
    }

    app->rt.elapsed_ms++;
    app->rt.segment_elapsed_ms++;

    if (inputs->stop_event != 0u) {
        Nuedc2024H_Stop(app);
        app->rt.last_line_detected = inputs->line_detected;
        return;
    }

    if ((app->rt.deadline_ms != 0u) &&
        (app->rt.elapsed_ms > app->rt.deadline_ms)) {
        app->rt.running = 0u;
        app->rt.fault = 1u;
        app->rt.drive_mode = NUEDC_2024_H_DRIVE_STOP;
    } else if (app->rt.mode == NUEDC_2024_H_MODE_MOTOR_CHECK) {
        if (app->rt.elapsed_ms >= app->rt.deadline_ms) {
            app->rt.running = 0u;
            app->rt.done = 1u;
            app->rt.drive_mode = NUEDC_2024_H_DRIVE_STOP;
        }
    } else if ((app->rt.mode != NUEDC_2024_H_MODE_SENSOR_CHECK) &&
               (current_segment_timed_out(app) != 0u)) {
        app->rt.running = 0u;
        app->rt.fault = 1u;
        app->rt.drive_mode = NUEDC_2024_H_DRIVE_STOP;
    } else if ((app->rt.mode != NUEDC_2024_H_MODE_SENSOR_CHECK) &&
               (current_segment_finished(app, inputs) != 0u)) {
        advance_segment(app);
    }

    update_drive_command(app, inputs);
    app->rt.last_line_detected = inputs->line_detected;
}

const nuedc_2024_h_runtime_t *Nuedc2024H_Runtime(const nuedc_2024_h_app_t *app)
{
    if (app == 0) {
        return 0;
    }
    return &app->rt;
}

const char *Nuedc2024H_ModeName(nuedc_2024_h_mode_t mode)
{
    switch (mode) {
    case NUEDC_2024_H_MODE_BASIC_AB:
        return "H1 A-B";
    case NUEDC_2024_H_MODE_BASIC_LOOP:
        return "H2 LOOP";
    case NUEDC_2024_H_MODE_FIGURE8:
        return "H3 FIG8";
    case NUEDC_2024_H_MODE_FIGURE8_4LAP:
        return "H4 FIG8 X4";
    case NUEDC_2024_H_MODE_SENSOR_CHECK:
        return "SENSOR";
    case NUEDC_2024_H_MODE_MOTOR_CHECK:
        return "MOTOR";
    default:
        return "IDLE";
    }
}

const char *Nuedc2024H_DriveName(nuedc_2024_h_drive_mode_t drive_mode)
{
    switch (drive_mode) {
    case NUEDC_2024_H_DRIVE_STRAIGHT:
        return "STRAIGHT";
    case NUEDC_2024_H_DRIVE_LINE_FOLLOW:
        return "LINE";
    case NUEDC_2024_H_DRIVE_STOP:
        return "STOP";
    default:
        return "IDLE";
    }
}

const char *Nuedc2024H_PointName(nuedc_2024_h_point_t point)
{
    switch (point) {
    case NUEDC_2024_H_POINT_A:
        return "A";
    case NUEDC_2024_H_POINT_B:
        return "B";
    case NUEDC_2024_H_POINT_C:
        return "C";
    case NUEDC_2024_H_POINT_D:
        return "D";
    default:
        return "-";
    }
}
