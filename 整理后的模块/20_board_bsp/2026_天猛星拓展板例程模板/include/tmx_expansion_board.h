#ifndef TMX_EXPANSION_BOARD_H
#define TMX_EXPANSION_BOARD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TMX_BOARD_PIN_MAP_VERSION 1u
#define TMX_BOARD_MOTOR_ARM_COOKIE 0x544D584Du
#define TMX_DUTY_PERMILLE_MAX 1000u

typedef enum {
    TMX_RESULT_OK = 0,
    TMX_RESULT_BAD_ARG = 1,
    TMX_RESULT_MISSING_OP = 2,
    TMX_RESULT_UNSAFE = 3,
    TMX_RESULT_RANGE = 4,
    TMX_RESULT_NOT_FOUND = 5,
    TMX_RESULT_HW_FAIL = 6
} tmx_result_t;

typedef enum {
    TMX_PIN_CONFIRMED_FROM_SCHEMATIC = 0,
    TMX_PIN_NEEDS_BOARD_CHECK = 1,
    TMX_PIN_CONFLICT_NEEDS_DECISION = 2
} tmx_pin_status_t;

typedef enum {
    TMX_MODULE_POWER = 0,
    TMX_MODULE_MOTOR_TB6612 = 1,
    TMX_MODULE_ENCODER = 2,
    TMX_MODULE_BUZZER = 3,
    TMX_MODULE_OLED = 4,
    TMX_MODULE_SERVO = 5,
    TMX_MODULE_UART = 6,
    TMX_MODULE_IMU = 7,
    TMX_MODULE_RGB = 8,
    TMX_MODULE_EXTRA_IO = 9
} tmx_module_t;

typedef enum {
    TMX_PORT_NONE = 0,
    TMX_PORT_PA = 1,
    TMX_PORT_PB = 2,
    TMX_PORT_POWER = 3,
    TMX_PORT_GND = 4,
    TMX_PORT_AGND = 5,
    TMX_PORT_VCC = 6
} tmx_pin_port_t;

typedef enum {
    TMX_SIG_NONE = 0,

    TMX_SIG_POWER_12V_IN,
    TMX_SIG_POWER_12V_POWER,
    TMX_SIG_POWER_5V,
    TMX_SIG_POWER_GND,

    TMX_SIG_MOTOR_A_PWM,
    TMX_SIG_MOTOR_A_IN1,
    TMX_SIG_MOTOR_A_IN2,
    TMX_SIG_MOTOR_B_PWM,
    TMX_SIG_MOTOR_B_IN1,
    TMX_SIG_MOTOR_B_IN2,
    TMX_SIG_MOTOR_STBY_5V,

    TMX_SIG_ENCODER_A_CHA,
    TMX_SIG_ENCODER_A_CHB,
    TMX_SIG_ENCODER_B_CHA,
    TMX_SIG_ENCODER_B_CHB,

    TMX_SIG_BUZZER,

    TMX_SIG_OLED_SCL,
    TMX_SIG_OLED_SDA,

    TMX_SIG_SERVO_1_PWM,
    TMX_SIG_SERVO_2_PWM,
    TMX_SIG_SERVO_3_PWM,
    TMX_SIG_SERVO_4_PWM,

    TMX_SIG_UART0_TX,
    TMX_SIG_UART0_RX,
    TMX_SIG_UART1_TX,
    TMX_SIG_UART1_RX,
    TMX_SIG_UART2_TX,
    TMX_SIG_UART2_RX,
    TMX_SIG_UART3_TX,
    TMX_SIG_UART3_RX,

    TMX_SIG_IMU_MPU_SCL_OR_UART0_RX,
    TMX_SIG_IMU_MPU_SDA_OR_UART0_TX,
    TMX_SIG_IMU_INT,
    TMX_SIG_IMU_AD0_GND,

    TMX_SIG_RGB_PB22,
    TMX_SIG_RGB_PB5,
    TMX_SIG_RGB_PA18
} tmx_signal_id_t;

typedef enum {
    TMX_MOTOR_A = 0,
    TMX_MOTOR_B = 1
} tmx_motor_id_t;

typedef enum {
    TMX_MOTOR_COAST = 0,
    TMX_MOTOR_FORWARD = 1,
    TMX_MOTOR_REVERSE = 2,
    TMX_MOTOR_BRAKE = 3
} tmx_motor_dir_t;

typedef enum {
    TMX_SMOKE_SAFE_DISARM = 0,
    TMX_SMOKE_BUZZER_TICK = 1,
    TMX_SMOKE_OLED_PROBE = 2,
    TMX_SMOKE_UART0_PING = 3,
    TMX_SMOKE_RGB_CYCLE = 4,
    TMX_SMOKE_SERVO_CENTER_IF_ENABLED = 5,
    TMX_SMOKE_MOTOR_STATIC_SAFE = 6
} tmx_smoke_step_t;

typedef struct {
    tmx_signal_id_t signal;
    tmx_module_t module;
    const char *name;
    tmx_pin_port_t port;
    uint8_t pin;
    const char *mspm0_pin;
    const char *syscfg_hint;
    const char *schematic_net;
    tmx_pin_status_t status;
    const char *note;
} tmx_pin_map_entry_t;

typedef void (*tmx_write_signal_fn)(tmx_signal_id_t signal, uint8_t level, void *user);
typedef uint8_t (*tmx_read_signal_fn)(tmx_signal_id_t signal, void *user);
typedef void (*tmx_pwm_write_fn)(tmx_signal_id_t signal, uint16_t duty_permille, void *user);
typedef void (*tmx_servo_pulse_fn)(tmx_signal_id_t signal, uint16_t pulse_us, void *user);
typedef uint8_t (*tmx_i2c_probe_fn)(uint8_t bus_index, uint8_t address_7bit, void *user);
typedef void (*tmx_uart_write_fn)(uint8_t uart_index, const uint8_t *data, uint16_t len, void *user);
typedef void (*tmx_delay_ms_fn)(uint16_t delay_ms, void *user);

typedef struct {
    tmx_write_signal_fn write_signal;
    tmx_read_signal_fn read_signal;
    tmx_pwm_write_fn write_pwm;
    tmx_servo_pulse_fn write_servo_pulse;
    tmx_i2c_probe_fn i2c_probe;
    tmx_uart_write_fn uart_write;
    tmx_delay_ms_fn delay_ms;
    void *user;
} tmx_board_ops_t;

typedef struct {
    uint16_t max_motor_duty_permille;
    uint16_t servo_min_us;
    uint16_t servo_center_us;
    uint16_t servo_max_us;
    uint8_t oled_i2c_bus;
    uint8_t oled_i2c_addr;
} tmx_board_config_t;

typedef struct {
    tmx_board_ops_t ops;
    tmx_board_config_t cfg;
    uint8_t initialized;
    uint8_t motors_armed;
    uint8_t servos_enabled;
} tmx_board_t;

void TmxBoard_DefaultConfig(tmx_board_config_t *out);

tmx_result_t TmxBoard_Init(tmx_board_t *board,
                           const tmx_board_ops_t *ops,
                           const tmx_board_config_t *cfg);
tmx_result_t TmxBoard_DisarmActuators(tmx_board_t *board);
tmx_result_t TmxBoard_ArmMotors(tmx_board_t *board, uint32_t safety_cookie);
tmx_result_t TmxBoard_DisarmMotors(tmx_board_t *board);
tmx_result_t TmxBoard_SetMotorRaw(tmx_board_t *board,
                                  tmx_motor_id_t motor,
                                  tmx_motor_dir_t dir,
                                  uint16_t duty_permille);
tmx_result_t TmxBoard_EnableServos(tmx_board_t *board, uint8_t enable);
tmx_result_t TmxBoard_SetServoPulseUs(tmx_board_t *board,
                                      uint8_t servo_index,
                                      uint16_t pulse_us);
tmx_result_t TmxBoard_SetBuzzer(tmx_board_t *board, uint8_t on);
tmx_result_t TmxBoard_SetRgb(tmx_board_t *board, uint8_t channel, uint8_t on);
tmx_result_t TmxBoard_OledProbe(tmx_board_t *board);
tmx_result_t TmxBoard_UartPing(tmx_board_t *board, uint8_t uart_index);
tmx_result_t TmxBoard_SmokeStep(tmx_board_t *board, tmx_smoke_step_t step);

const tmx_pin_map_entry_t *TmxBoard_PinMap(size_t *count);
const tmx_pin_map_entry_t *TmxBoard_FindSignal(tmx_signal_id_t signal);
const char *TmxBoard_SignalName(tmx_signal_id_t signal);
const char *TmxBoard_ResultName(tmx_result_t result);

#ifdef __cplusplus
}
#endif

#endif
