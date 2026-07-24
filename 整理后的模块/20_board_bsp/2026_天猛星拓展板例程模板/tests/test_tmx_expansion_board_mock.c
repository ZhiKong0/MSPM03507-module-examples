#include "tmx_expansion_board.h"

#include <stdio.h>
#include <string.h>

#define MOCK_SIGNAL_COUNT ((unsigned)TMX_SIG_RGB_PA18 + 1u)

typedef struct {
    uint8_t signal_level[MOCK_SIGNAL_COUNT];
    uint16_t pwm_duty[MOCK_SIGNAL_COUNT];
    uint16_t servo_pulse[MOCK_SIGNAL_COUNT];
    uint8_t oled_ack;
    uint8_t uart_count;
    uint8_t failures;
} mock_hw_t;

static void expect_true(mock_hw_t *hw, int cond, const char *name)
{
    if (!cond) {
        ++hw->failures;
        printf("FAIL: %s\n", name);
    }
}

static void mock_write_signal(tmx_signal_id_t signal, uint8_t level, void *user)
{
    mock_hw_t *hw = (mock_hw_t *)user;
    if ((unsigned)signal < MOCK_SIGNAL_COUNT) {
        hw->signal_level[(unsigned)signal] = level;
    }
}

static void mock_write_pwm(tmx_signal_id_t signal, uint16_t duty, void *user)
{
    mock_hw_t *hw = (mock_hw_t *)user;
    if ((unsigned)signal < MOCK_SIGNAL_COUNT) {
        hw->pwm_duty[(unsigned)signal] = duty;
    }
}

static void mock_write_servo(tmx_signal_id_t signal, uint16_t pulse_us, void *user)
{
    mock_hw_t *hw = (mock_hw_t *)user;
    if ((unsigned)signal < MOCK_SIGNAL_COUNT) {
        hw->servo_pulse[(unsigned)signal] = pulse_us;
    }
}

static uint8_t mock_i2c_probe(uint8_t bus_index, uint8_t addr, void *user)
{
    mock_hw_t *hw = (mock_hw_t *)user;
    return ((bus_index == 0u) && (addr == 0x3cu)) ? hw->oled_ack : 0u;
}

static void mock_uart_write(uint8_t uart_index,
                            const uint8_t *data,
                            uint16_t len,
                            void *user)
{
    mock_hw_t *hw = (mock_hw_t *)user;
    if ((uart_index == 0u) && (data != 0) && (len != 0u)) {
        ++hw->uart_count;
    }
}

static void mock_delay(uint16_t delay_ms, void *user)
{
    (void)delay_ms;
    (void)user;
}

int main(void)
{
    mock_hw_t hw;
    tmx_board_t board;
    tmx_board_ops_t ops;
    tmx_result_t result;
    size_t pin_count = 0u;

    memset(&hw, 0, sizeof(hw));
    hw.oled_ack = 1u;

    memset(&ops, 0, sizeof(ops));
    ops.write_signal = mock_write_signal;
    ops.write_pwm = mock_write_pwm;
    ops.write_servo_pulse = mock_write_servo;
    ops.i2c_probe = mock_i2c_probe;
    ops.uart_write = mock_uart_write;
    ops.delay_ms = mock_delay;
    ops.user = &hw;

    result = TmxBoard_Init(&board, &ops, 0);
    expect_true(&hw, result == TMX_RESULT_OK, "init");
    expect_true(&hw, board.motors_armed == 0u, "init disarms motors");
    expect_true(&hw, board.servos_enabled == 0u, "init disables servos");

    result = TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_FORWARD, 100u);
    expect_true(&hw, result == TMX_RESULT_UNSAFE, "motor forward needs arm");

    result = TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_FORWARD, 0u);
    expect_true(&hw, result == TMX_RESULT_UNSAFE, "zero duty forward still needs arm");

    result = TmxBoard_ArmMotors(&board, 0u);
    expect_true(&hw, result == TMX_RESULT_UNSAFE, "bad arm cookie rejected");

    result = TmxBoard_ArmMotors(&board, TMX_BOARD_MOTOR_ARM_COOKIE);
    expect_true(&hw, result == TMX_RESULT_OK, "arm cookie accepted");

    result = TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_FORWARD, 200u);
    expect_true(&hw, result == TMX_RESULT_OK, "armed motor forward");
    expect_true(&hw, hw.signal_level[TMX_SIG_MOTOR_A_IN1] == 1u, "motor A IN1 high");
    expect_true(&hw, hw.signal_level[TMX_SIG_MOTOR_A_IN2] == 0u, "motor A IN2 low");
    expect_true(&hw, hw.pwm_duty[TMX_SIG_MOTOR_A_PWM] == 200u, "motor A pwm duty");

    result = TmxBoard_SetMotorRaw(&board, TMX_MOTOR_B, TMX_MOTOR_REVERSE, 301u);
    expect_true(&hw, result == TMX_RESULT_RANGE, "default duty cap");

    result = TmxBoard_SetMotorRaw(&board, TMX_MOTOR_A, TMX_MOTOR_COAST, 999u);
    expect_true(&hw, result == TMX_RESULT_OK, "coast ignores duty argument");
    expect_true(&hw, hw.pwm_duty[TMX_SIG_MOTOR_A_PWM] == 0u, "coast pwm zero");

    result = TmxBoard_SetServoPulseUs(&board, 1u, 1500u);
    expect_true(&hw, result == TMX_RESULT_UNSAFE, "servo center needs enable");

    result = TmxBoard_EnableServos(&board, 1u);
    expect_true(&hw, result == TMX_RESULT_OK, "enable servos");
    result = TmxBoard_SetServoPulseUs(&board, 1u, 1500u);
    expect_true(&hw, result == TMX_RESULT_OK, "servo center after enable");
    expect_true(&hw, hw.servo_pulse[TMX_SIG_SERVO_1_PWM] == 1500u, "servo pulse stored");

    result = TmxBoard_SmokeStep(&board, TMX_SMOKE_OLED_PROBE);
    expect_true(&hw, result == TMX_RESULT_OK, "oled probe ack");

    result = TmxBoard_SmokeStep(&board, TMX_SMOKE_UART0_PING);
    expect_true(&hw, result == TMX_RESULT_OK, "uart ping result");
    expect_true(&hw, hw.uart_count == 1u, "uart ping write count");

    (void)TmxBoard_PinMap(&pin_count);
    expect_true(&hw, pin_count >= 30u, "pin map count");
    expect_true(&hw, TmxBoard_FindSignal(TMX_SIG_MOTOR_A_PWM) != 0, "find PWMA");
    expect_true(&hw, TmxBoard_FindSignal(TMX_SIG_IMU_MPU_SCL_OR_UART0_RX)->status ==
                     TMX_PIN_CONFLICT_NEEDS_DECISION,
                "imu conflict marked");

    result = TmxBoard_DisarmActuators(&board);
    expect_true(&hw, result == TMX_RESULT_OK, "final disarm");
    expect_true(&hw, board.motors_armed == 0u, "final motors disarmed");
    expect_true(&hw, hw.pwm_duty[TMX_SIG_MOTOR_A_PWM] == 0u, "final motor A pwm zero");

    if (hw.failures != 0u) {
        return 1;
    }

    printf("PASS: tmx expansion board mock test\n");
    return 0;
}
