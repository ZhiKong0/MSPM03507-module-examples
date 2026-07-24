#include "tmx_expansion_board.h"

/*
 * The real project should provide these functions in its board adapter layer.
 * Keep this example small: it shows call order and safety gates only.
 */
extern void Board_PlatformInit(void);
extern tmx_board_ops_t Board_BuildTmxOps(void);
extern void Board_ReportSmokeResult(tmx_smoke_step_t step, tmx_result_t result);

static void run_step(tmx_board_t *board, tmx_smoke_step_t step)
{
    tmx_result_t result = TmxBoard_SmokeStep(board, step);
    Board_ReportSmokeResult(step, result);
}

int main(void)
{
    tmx_board_t board;
    tmx_board_config_t cfg;
    tmx_board_ops_t ops;
    tmx_result_t result;

    Board_PlatformInit();
    ops = Board_BuildTmxOps();
    TmxBoard_DefaultConfig(&cfg);

    /*
     * Default cap is intentionally low. Raise it only after wheel direction,
     * encoder polarity, current limit, and emergency stop have been checked.
     */
    cfg.max_motor_duty_permille = 250u;

    result = TmxBoard_Init(&board, &ops, &cfg);
    Board_ReportSmokeResult(TMX_SMOKE_SAFE_DISARM, result);
    if (result != TMX_RESULT_OK) {
        for (;;) {
        }
    }

    run_step(&board, TMX_SMOKE_SAFE_DISARM);
    run_step(&board, TMX_SMOKE_BUZZER_TICK);
    run_step(&board, TMX_SMOKE_OLED_PROBE);
    run_step(&board, TMX_SMOKE_UART0_PING);
    run_step(&board, TMX_SMOKE_RGB_CYCLE);

    /*
     * Servo and motor movement are deliberate opt-in actions.
     *
     * TmxBoard_EnableServos(&board, 1u);
     * run_step(&board, TMX_SMOKE_SERVO_CENTER_IF_ENABLED);
     *
     * TmxBoard_ArmMotors(&board, TMX_BOARD_MOTOR_ARM_COOKIE);
     * TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_FORWARD, 120u);
     */
    run_step(&board, TMX_SMOKE_MOTOR_STATIC_SAFE);

    for (;;) {
    }
}
