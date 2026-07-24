#include "tmx_expansion_board.h"

#include <string.h>

#define TMX_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

static const tmx_pin_map_entry_t g_tmx_pin_map[] = {
    { TMX_SIG_POWER_12V_IN, TMX_MODULE_POWER, "12V input", TMX_PORT_POWER, 12u,
      "+12V", "", "12V", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "XT60/12V input area; verify polarity before powering motors." },
    { TMX_SIG_POWER_12V_POWER, TMX_MODULE_POWER, "12V_POWER switch net", TMX_PORT_POWER,
      12u, "12V_POWER", "", "12V_POWER", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Board-level 12V switched/distributed net." },
    { TMX_SIG_POWER_5V, TMX_MODULE_POWER, "5V rail", TMX_PORT_VCC, 5u,
      "+5V", "", "+5V", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Logic and servo/module 5V rail." },
    { TMX_SIG_POWER_GND, TMX_MODULE_POWER, "GND", TMX_PORT_GND, 0u,
      "GND", "", "GND", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Common ground for controller and power modules." },

    { TMX_SIG_MOTOR_A_PWM, TMX_MODULE_MOTOR_TB6612, "TB6612 PWMA", TMX_PORT_PA, 12u,
      "PA12", "TIMG0_C0", "TIMG0_C0_PA12", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Motor A speed PWM." },
    { TMX_SIG_MOTOR_A_IN1, TMX_MODULE_MOTOR_TB6612, "TB6612 AIN1", TMX_PORT_PB, 17u,
      "PB17", "GPIO output", "PB17", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Motor A direction input 1." },
    { TMX_SIG_MOTOR_A_IN2, TMX_MODULE_MOTOR_TB6612, "TB6612 AIN2", TMX_PORT_PB, 19u,
      "PB19", "GPIO output", "PB19", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Motor A direction input 2." },
    { TMX_SIG_MOTOR_B_PWM, TMX_MODULE_MOTOR_TB6612, "TB6612 PWMB", TMX_PORT_PA, 13u,
      "PA13", "TIMG0_C1", "TIMG0_C1_PA13", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Motor B speed PWM." },
    { TMX_SIG_MOTOR_B_IN1, TMX_MODULE_MOTOR_TB6612, "TB6612 BIN1", TMX_PORT_PA, 16u,
      "PA16", "GPIO output", "PA16", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Motor B direction input 1." },
    { TMX_SIG_MOTOR_B_IN2, TMX_MODULE_MOTOR_TB6612, "TB6612 BIN2", TMX_PORT_PB, 24u,
      "PB24", "GPIO output", "PB24", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Motor B direction input 2." },
    { TMX_SIG_MOTOR_STBY_5V, TMX_MODULE_MOTOR_TB6612, "TB6612 STBY", TMX_PORT_VCC,
      5u, "+5V", "", "+5V", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "STBY is tied high on schematic, so software must keep PWM at zero when idle." },

    { TMX_SIG_ENCODER_A_CHA, TMX_MODULE_ENCODER, "Encoder A channel A", TMX_PORT_PA,
      26u, "PA26", "TIMG8_C0", "TIMG8_C0_PA26", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "MOTOR2 encoder A phase in schematic." },
    { TMX_SIG_ENCODER_A_CHB, TMX_MODULE_ENCODER, "Encoder A channel B", TMX_PORT_PA,
      27u, "PA27", "TIMG8_C1", "TIMG8_C1_PA27", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "MOTOR2 encoder B phase in schematic." },
    { TMX_SIG_ENCODER_B_CHA, TMX_MODULE_ENCODER, "Encoder B channel A", TMX_PORT_PA,
      25u, "PA25", "TIMG12_C1", "TIMG12_C1_PA25", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "MOTOR1 encoder A phase in schematic." },
    { TMX_SIG_ENCODER_B_CHB, TMX_MODULE_ENCODER, "Encoder B channel B", TMX_PORT_PA,
      14u, "PA14", "TIMG12_C0", "TIMG12_C0_PA14", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "MOTOR1 encoder B phase in schematic." },

    { TMX_SIG_BUZZER, TMX_MODULE_BUZZER, "Buzzer", TMX_PORT_PA, 7u,
      "PA7", "GPIO output", "PA7", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Active level depends on the buzzer module; template assumes high=on." },

    { TMX_SIG_OLED_SCL, TMX_MODULE_OLED, "OLED I2C SCL", TMX_PORT_PA, 31u,
      "PA31", "I2C0_SCL", "I2C0_SCL_PA31", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "OLED header SCL." },
    { TMX_SIG_OLED_SDA, TMX_MODULE_OLED, "OLED I2C SDA", TMX_PORT_PA, 28u,
      "PA28", "I2C0_SDA", "I2C0_SDA_PA28", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "OLED header SDA." },

    { TMX_SIG_SERVO_1_PWM, TMX_MODULE_SERVO, "Servo 1 PWM", TMX_PORT_PA, 22u,
      "PA22", "TIMA0_C1", "TIMA0_C1_PA22", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "3-pin servo header with +5V/GND." },
    { TMX_SIG_SERVO_2_PWM, TMX_MODULE_SERVO, "Servo 2 PWM", TMX_PORT_PA, 15u,
      "PA15", "TIMA0_C2", "TIMA0_C2_PA15", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "3-pin servo header with +5V/GND." },
    { TMX_SIG_SERVO_3_PWM, TMX_MODULE_SERVO, "Servo 3 PWM", TMX_PORT_PA, 17u,
      "PA17", "TIMA0_C3", "TIMA0_C3_PA17", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "3-pin servo header with +5V/GND." },
    { TMX_SIG_SERVO_4_PWM, TMX_MODULE_SERVO, "Servo 4 PWM", TMX_PORT_PA, 21u,
      "PA21", "TIMA0_C0", "TIMA0_C0_PA21", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "3-pin servo header with +5V/GND." },

    { TMX_SIG_UART0_TX, TMX_MODULE_UART, "UART0 TX", TMX_PORT_PA, 0u,
      "PA0", "UART0_TX", "UART0_TX_PA0", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Also appears in the IMU/JY area." },
    { TMX_SIG_UART0_RX, TMX_MODULE_UART, "UART0 RX", TMX_PORT_PA, 1u,
      "PA1", "UART0_RX", "UART0_RX_PA1", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Also appears in the IMU/JY area." },
    { TMX_SIG_UART1_TX, TMX_MODULE_UART, "UART1 TX", TMX_PORT_PB, 6u,
      "PB6", "UART1_TX", "UART1_TX_PB6", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Available on serial/Zigbee headers." },
    { TMX_SIG_UART1_RX, TMX_MODULE_UART, "UART1 RX", TMX_PORT_PB, 7u,
      "PB7", "UART1_RX", "UART1_RX_PB7", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Available on serial/Zigbee headers." },
    { TMX_SIG_UART2_TX, TMX_MODULE_UART, "UART2 TX", TMX_PORT_PA, 23u,
      "PA23", "UART2_TX", "UART2_TX_PA23", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "UART2 header also exposes GND and 12V." },
    { TMX_SIG_UART2_RX, TMX_MODULE_UART, "UART2 RX", TMX_PORT_PA, 24u,
      "PA24", "UART2_RX", "UART2_RX_PA24", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "UART2 header also exposes GND and 12V." },
    { TMX_SIG_UART3_TX, TMX_MODULE_UART, "UART3 TX", TMX_PORT_PB, 2u,
      "PB2", "UART3_TX", "UART3_TX_PB2", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Shown near Zigbee/serial area." },
    { TMX_SIG_UART3_RX, TMX_MODULE_UART, "UART3 RX", TMX_PORT_PB, 3u,
      "PB3", "UART3_RX", "UART3_RX_PB3", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "Shown near Zigbee/serial area." },

    { TMX_SIG_IMU_MPU_SCL_OR_UART0_RX, TMX_MODULE_IMU, "MPU SCL or UART0 RX",
      TMX_PORT_PA, 1u, "PA1", "I2C/UART conflict", "UART0_RX_PA1",
      TMX_PIN_CONFLICT_NEEDS_DECISION,
      "MPU6050 SCL pad is labelled with UART0_RX_PA1 net in the picture." },
    { TMX_SIG_IMU_MPU_SDA_OR_UART0_TX, TMX_MODULE_IMU, "MPU SDA or UART0 TX",
      TMX_PORT_PA, 0u, "PA0", "I2C/UART conflict", "UART0_TX_PA0",
      TMX_PIN_CONFLICT_NEEDS_DECISION,
      "MPU6050 SDA pad is labelled with UART0_TX_PA0 net in the picture." },
    { TMX_SIG_IMU_INT, TMX_MODULE_IMU, "MPU INT", TMX_PORT_PB, 4u,
      "PB4", "GPIO interrupt input", "PB4", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "MPU6050 INT pad." },
    { TMX_SIG_IMU_AD0_GND, TMX_MODULE_IMU, "MPU AD0", TMX_PORT_GND, 0u,
      "GND", "", "GND", TMX_PIN_CONFIRMED_FROM_SCHEMATIC,
      "AD0 tied low if using MPU6050 address 0x68." },

    { TMX_SIG_RGB_PB22, TMX_MODULE_RGB, "RGB channel on PB22", TMX_PORT_PB, 22u,
      "PB22", "GPIO/PWM candidate", "PB22", TMX_PIN_NEEDS_BOARD_CHECK,
      "Three-pin RGB header; confirm LED type and active level." },
    { TMX_SIG_RGB_PB5, TMX_MODULE_RGB, "RGB channel on PB5", TMX_PORT_PB, 5u,
      "PB5", "GPIO/PWM candidate", "PB5", TMX_PIN_NEEDS_BOARD_CHECK,
      "Three-pin RGB header; confirm LED type and active level." },
    { TMX_SIG_RGB_PA18, TMX_MODULE_RGB, "RGB channel on PA18", TMX_PORT_PA, 18u,
      "PA18", "GPIO/PWM candidate", "PA18", TMX_PIN_NEEDS_BOARD_CHECK,
      "Three-pin RGB header; confirm LED type and active level." },
};

static tmx_result_t ensure_board(const tmx_board_t *board)
{
    if ((board == 0) || (board->initialized == 0u)) {
        return TMX_RESULT_BAD_ARG;
    }
    return TMX_RESULT_OK;
}

static void write_signal_if_present(tmx_board_t *board, tmx_signal_id_t signal, uint8_t level)
{
    if (board->ops.write_signal != 0) {
        board->ops.write_signal(signal, level, board->ops.user);
    }
}

static void write_pwm_if_present(tmx_board_t *board, tmx_signal_id_t signal, uint16_t duty)
{
    if (board->ops.write_pwm != 0) {
        board->ops.write_pwm(signal, duty, board->ops.user);
    }
}

static void write_servo_if_present(tmx_board_t *board, tmx_signal_id_t signal, uint16_t pulse_us)
{
    if (board->ops.write_servo_pulse != 0) {
        board->ops.write_servo_pulse(signal, pulse_us, board->ops.user);
    }
}

static tmx_signal_id_t servo_signal(uint8_t servo_index)
{
    static const tmx_signal_id_t signals[] = {
        TMX_SIG_SERVO_1_PWM,
        TMX_SIG_SERVO_2_PWM,
        TMX_SIG_SERVO_3_PWM,
        TMX_SIG_SERVO_4_PWM,
    };

    if ((servo_index == 0u) || (servo_index > TMX_ARRAY_COUNT(signals))) {
        return TMX_SIG_NONE;
    }
    return signals[servo_index - 1u];
}

static tmx_signal_id_t rgb_signal(uint8_t channel)
{
    static const tmx_signal_id_t signals[] = {
        TMX_SIG_RGB_PB22,
        TMX_SIG_RGB_PB5,
        TMX_SIG_RGB_PA18,
    };

    if (channel >= TMX_ARRAY_COUNT(signals)) {
        return TMX_SIG_NONE;
    }
    return signals[channel];
}

static void motor_signals(tmx_motor_id_t motor,
                          tmx_signal_id_t *pwm,
                          tmx_signal_id_t *in1,
                          tmx_signal_id_t *in2)
{
    if (motor == TMX_MOTOR_A) {
        *pwm = TMX_SIG_MOTOR_A_PWM;
        *in1 = TMX_SIG_MOTOR_A_IN1;
        *in2 = TMX_SIG_MOTOR_A_IN2;
    } else {
        *pwm = TMX_SIG_MOTOR_B_PWM;
        *in1 = TMX_SIG_MOTOR_B_IN1;
        *in2 = TMX_SIG_MOTOR_B_IN2;
    }
}

void TmxBoard_DefaultConfig(tmx_board_config_t *out)
{
    if (out == 0) {
        return;
    }

    out->max_motor_duty_permille = 300u;
    out->servo_min_us = 500u;
    out->servo_center_us = 1500u;
    out->servo_max_us = 2500u;
    out->oled_i2c_bus = 0u;
    out->oled_i2c_addr = 0x3cu;
}

tmx_result_t TmxBoard_Init(tmx_board_t *board,
                           const tmx_board_ops_t *ops,
                           const tmx_board_config_t *cfg)
{
    tmx_board_config_t default_cfg;

    if (board == 0) {
        return TMX_RESULT_BAD_ARG;
    }

    memset(board, 0, sizeof(*board));

    if (ops != 0) {
        board->ops = *ops;
    }

    if (cfg != 0) {
        board->cfg = *cfg;
    } else {
        TmxBoard_DefaultConfig(&default_cfg);
        board->cfg = default_cfg;
    }

    if ((board->cfg.max_motor_duty_permille == 0u) ||
        (board->cfg.max_motor_duty_permille > TMX_DUTY_PERMILLE_MAX) ||
        (board->cfg.servo_min_us > board->cfg.servo_center_us) ||
        (board->cfg.servo_center_us > board->cfg.servo_max_us)) {
        return TMX_RESULT_RANGE;
    }

    board->initialized = 1u;
    return TmxBoard_DisarmActuators(board);
}

tmx_result_t TmxBoard_DisarmActuators(tmx_board_t *board)
{
    tmx_result_t ready = ensure_board(board);
    if (ready != TMX_RESULT_OK) {
        return ready;
    }

    board->motors_armed = 0u;
    (void)TmxBoard_SetMotorRaw(board, TMX_MOTOR_A, TMX_MOTOR_COAST, 0u);
    (void)TmxBoard_SetMotorRaw(board, TMX_MOTOR_B, TMX_MOTOR_COAST, 0u);

    write_signal_if_present(board, TMX_SIG_BUZZER, 0u);
    for (uint8_t i = 0u; i < 3u; ++i) {
        (void)TmxBoard_SetRgb(board, i, 0u);
    }

    board->servos_enabled = 0u;
    for (uint8_t i = 1u; i <= 4u; ++i) {
        write_servo_if_present(board, servo_signal(i), 0u);
    }

    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_ArmMotors(tmx_board_t *board, uint32_t safety_cookie)
{
    tmx_result_t ready = ensure_board(board);
    if (ready != TMX_RESULT_OK) {
        return ready;
    }
    if (safety_cookie != TMX_BOARD_MOTOR_ARM_COOKIE) {
        return TMX_RESULT_UNSAFE;
    }

    board->motors_armed = 1u;
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_DisarmMotors(tmx_board_t *board)
{
    tmx_result_t ready = ensure_board(board);
    if (ready != TMX_RESULT_OK) {
        return ready;
    }

    board->motors_armed = 0u;
    (void)TmxBoard_SetMotorRaw(board, TMX_MOTOR_A, TMX_MOTOR_COAST, 0u);
    (void)TmxBoard_SetMotorRaw(board, TMX_MOTOR_B, TMX_MOTOR_COAST, 0u);
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_SetMotorRaw(tmx_board_t *board,
                                  tmx_motor_id_t motor,
                                  tmx_motor_dir_t dir,
                                  uint16_t duty_permille)
{
    tmx_signal_id_t pwm = TMX_SIG_NONE;
    tmx_signal_id_t in1 = TMX_SIG_NONE;
    tmx_signal_id_t in2 = TMX_SIG_NONE;
    uint8_t level1 = 0u;
    uint8_t level2 = 0u;
    uint16_t duty = duty_permille;
    tmx_result_t ready = ensure_board(board);

    if (ready != TMX_RESULT_OK) {
        return ready;
    }
    if ((motor != TMX_MOTOR_A) && (motor != TMX_MOTOR_B)) {
        return TMX_RESULT_BAD_ARG;
    }
    if ((dir == TMX_MOTOR_FORWARD) || (dir == TMX_MOTOR_REVERSE)) {
        if (board->motors_armed == 0u) {
            return TMX_RESULT_UNSAFE;
        }
        if (duty > board->cfg.max_motor_duty_permille) {
            return TMX_RESULT_RANGE;
        }
    } else {
        duty = 0u;
    }

    switch (dir) {
    case TMX_MOTOR_COAST:
        level1 = 0u;
        level2 = 0u;
        break;
    case TMX_MOTOR_FORWARD:
        level1 = 1u;
        level2 = 0u;
        break;
    case TMX_MOTOR_REVERSE:
        level1 = 0u;
        level2 = 1u;
        break;
    case TMX_MOTOR_BRAKE:
        level1 = 1u;
        level2 = 1u;
        break;
    default:
        return TMX_RESULT_BAD_ARG;
    }

    motor_signals(motor, &pwm, &in1, &in2);
    write_pwm_if_present(board, pwm, 0u);
    write_signal_if_present(board, in1, level1);
    write_signal_if_present(board, in2, level2);
    write_pwm_if_present(board, pwm, duty);

    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_EnableServos(tmx_board_t *board, uint8_t enable)
{
    tmx_result_t ready = ensure_board(board);
    if (ready != TMX_RESULT_OK) {
        return ready;
    }

    board->servos_enabled = (enable != 0u) ? 1u : 0u;
    if (board->servos_enabled == 0u) {
        for (uint8_t i = 1u; i <= 4u; ++i) {
            write_servo_if_present(board, servo_signal(i), 0u);
        }
    }
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_SetServoPulseUs(tmx_board_t *board,
                                      uint8_t servo_index,
                                      uint16_t pulse_us)
{
    tmx_signal_id_t signal;
    tmx_result_t ready = ensure_board(board);

    if (ready != TMX_RESULT_OK) {
        return ready;
    }
    signal = servo_signal(servo_index);
    if (signal == TMX_SIG_NONE) {
        return TMX_RESULT_BAD_ARG;
    }
    if (pulse_us == 0u) {
        write_servo_if_present(board, signal, 0u);
        return TMX_RESULT_OK;
    }
    if (board->servos_enabled == 0u) {
        return TMX_RESULT_UNSAFE;
    }
    if ((pulse_us < board->cfg.servo_min_us) || (pulse_us > board->cfg.servo_max_us)) {
        return TMX_RESULT_RANGE;
    }

    write_servo_if_present(board, signal, pulse_us);
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_SetBuzzer(tmx_board_t *board, uint8_t on)
{
    tmx_result_t ready = ensure_board(board);
    if (ready != TMX_RESULT_OK) {
        return ready;
    }

    write_signal_if_present(board, TMX_SIG_BUZZER, (on != 0u) ? 1u : 0u);
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_SetRgb(tmx_board_t *board, uint8_t channel, uint8_t on)
{
    tmx_signal_id_t signal;
    tmx_result_t ready = ensure_board(board);

    if (ready != TMX_RESULT_OK) {
        return ready;
    }
    signal = rgb_signal(channel);
    if (signal == TMX_SIG_NONE) {
        return TMX_RESULT_BAD_ARG;
    }

    write_signal_if_present(board, signal, (on != 0u) ? 1u : 0u);
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_OledProbe(tmx_board_t *board)
{
    tmx_result_t ready = ensure_board(board);
    if (ready != TMX_RESULT_OK) {
        return ready;
    }
    if (board->ops.i2c_probe == 0) {
        return TMX_RESULT_MISSING_OP;
    }

    return (board->ops.i2c_probe(board->cfg.oled_i2c_bus,
                                 board->cfg.oled_i2c_addr,
                                 board->ops.user) != 0u) ?
           TMX_RESULT_OK : TMX_RESULT_HW_FAIL;
}

tmx_result_t TmxBoard_UartPing(tmx_board_t *board, uint8_t uart_index)
{
    static const uint8_t ping[] = "TMX-EXPANSION-PING\r\n";
    tmx_result_t ready = ensure_board(board);

    if (ready != TMX_RESULT_OK) {
        return ready;
    }
    if (board->ops.uart_write == 0) {
        return TMX_RESULT_MISSING_OP;
    }
    if (uart_index > 3u) {
        return TMX_RESULT_BAD_ARG;
    }

    board->ops.uart_write(uart_index, ping, (uint16_t)(sizeof(ping) - 1u),
                          board->ops.user);
    return TMX_RESULT_OK;
}

tmx_result_t TmxBoard_SmokeStep(tmx_board_t *board, tmx_smoke_step_t step)
{
    tmx_result_t result;
    tmx_result_t ready = ensure_board(board);

    if (ready != TMX_RESULT_OK) {
        return ready;
    }

    switch (step) {
    case TMX_SMOKE_SAFE_DISARM:
    case TMX_SMOKE_MOTOR_STATIC_SAFE:
        return TmxBoard_DisarmActuators(board);
    case TMX_SMOKE_BUZZER_TICK:
        result = TmxBoard_SetBuzzer(board, 1u);
        if (result != TMX_RESULT_OK) {
            return result;
        }
        if (board->ops.delay_ms != 0) {
            board->ops.delay_ms(50u, board->ops.user);
        }
        return TmxBoard_SetBuzzer(board, 0u);
    case TMX_SMOKE_OLED_PROBE:
        return TmxBoard_OledProbe(board);
    case TMX_SMOKE_UART0_PING:
        return TmxBoard_UartPing(board, 0u);
    case TMX_SMOKE_RGB_CYCLE:
        for (uint8_t i = 0u; i < 3u; ++i) {
            (void)TmxBoard_SetRgb(board, i, 1u);
            if (board->ops.delay_ms != 0) {
                board->ops.delay_ms(80u, board->ops.user);
            }
            (void)TmxBoard_SetRgb(board, i, 0u);
        }
        return TMX_RESULT_OK;
    case TMX_SMOKE_SERVO_CENTER_IF_ENABLED:
        if (board->servos_enabled == 0u) {
            return TMX_RESULT_UNSAFE;
        }
        for (uint8_t i = 1u; i <= 4u; ++i) {
            result = TmxBoard_SetServoPulseUs(board, i, board->cfg.servo_center_us);
            if (result != TMX_RESULT_OK) {
                return result;
            }
        }
        return TMX_RESULT_OK;
    default:
        return TMX_RESULT_BAD_ARG;
    }
}

const tmx_pin_map_entry_t *TmxBoard_PinMap(size_t *count)
{
    if (count != 0) {
        *count = TMX_ARRAY_COUNT(g_tmx_pin_map);
    }
    return g_tmx_pin_map;
}

const tmx_pin_map_entry_t *TmxBoard_FindSignal(tmx_signal_id_t signal)
{
    for (size_t i = 0u; i < TMX_ARRAY_COUNT(g_tmx_pin_map); ++i) {
        if (g_tmx_pin_map[i].signal == signal) {
            return &g_tmx_pin_map[i];
        }
    }
    return 0;
}

const char *TmxBoard_SignalName(tmx_signal_id_t signal)
{
    const tmx_pin_map_entry_t *entry = TmxBoard_FindSignal(signal);
    if (entry == 0) {
        return "unknown";
    }
    return entry->name;
}

const char *TmxBoard_ResultName(tmx_result_t result)
{
    switch (result) {
    case TMX_RESULT_OK:
        return "ok";
    case TMX_RESULT_BAD_ARG:
        return "bad-arg";
    case TMX_RESULT_MISSING_OP:
        return "missing-op";
    case TMX_RESULT_UNSAFE:
        return "unsafe";
    case TMX_RESULT_RANGE:
        return "range";
    case TMX_RESULT_NOT_FOUND:
        return "not-found";
    case TMX_RESULT_HW_FAIL:
        return "hw-fail";
    default:
        return "unknown";
    }
}
