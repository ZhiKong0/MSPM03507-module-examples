#include "tmx_expansion_board.h"

/*
 * Define TMX_EXAMPLE_WITH_TI_DRIVERLIB inside a real MSPM0 SysConfig project
 * after ti_msp_dl_config.h exists. This file maps the portable BSP ops to
 * DriverLib calls without tying src/tmx_expansion_board.c to TI headers.
 */
#if defined(TMX_EXAMPLE_WITH_TI_DRIVERLIB)

#include "ti_msp_dl_config.h"

#ifndef TMX_SYSCFG_MOTOR_PWM_PERIOD_TICKS
#define TMX_SYSCFG_MOTOR_PWM_PERIOD_TICKS (3200u)
#endif

#ifndef TMX_SYSCFG_SERVO_PWM_PERIOD_US
#define TMX_SYSCFG_SERVO_PWM_PERIOD_US (20000u)
#endif

#ifndef TMX_SYSCFG_SERVO_PWM_PERIOD_TICKS
#define TMX_SYSCFG_SERVO_PWM_PERIOD_TICKS (64000u)
#endif

#ifndef TMX_SYSCFG_I2C_TIMEOUT
#define TMX_SYSCFG_I2C_TIMEOUT (80000u)
#endif

#ifndef TMX_SYSCFG_I2C_PROBE_SEND_OLED_CONTROL_BYTE
#define TMX_SYSCFG_I2C_PROBE_SEND_OLED_CONTROL_BYTE (0u)
#endif

#ifndef TMX_SYSCFG_PWM_COMPARE_INVERTED
#define TMX_SYSCFG_PWM_COMPARE_INVERTED (0u)
#endif

#if !defined(TMX_SYSCFG_MOTOR_PWM_INST) && defined(TIMG0)
#define TMX_SYSCFG_MOTOR_PWM_INST TIMG0
#endif

#ifndef TMX_SYSCFG_MOTOR_A_PWM_CC_INDEX
#define TMX_SYSCFG_MOTOR_A_PWM_CC_INDEX DL_TIMER_CC_0_INDEX
#endif

#ifndef TMX_SYSCFG_MOTOR_B_PWM_CC_INDEX
#define TMX_SYSCFG_MOTOR_B_PWM_CC_INDEX DL_TIMER_CC_1_INDEX
#endif

#if !defined(TMX_SYSCFG_SERVO_PWM_INST) && defined(TIMA0)
#define TMX_SYSCFG_SERVO_PWM_INST TIMA0
#endif

#ifndef TMX_SYSCFG_SERVO1_CC_INDEX
#define TMX_SYSCFG_SERVO1_CC_INDEX DL_TIMER_CC_1_INDEX
#endif

#ifndef TMX_SYSCFG_SERVO2_CC_INDEX
#define TMX_SYSCFG_SERVO2_CC_INDEX DL_TIMER_CC_2_INDEX
#endif

#ifndef TMX_SYSCFG_SERVO3_CC_INDEX
#define TMX_SYSCFG_SERVO3_CC_INDEX DL_TIMER_CC_3_INDEX
#endif

#ifndef TMX_SYSCFG_SERVO4_CC_INDEX
#define TMX_SYSCFG_SERVO4_CC_INDEX DL_TIMER_CC_0_INDEX
#endif

#if !defined(TMX_SYSCFG_OLED_I2C_INST)
#if defined(I2C_OLED_INST)
#define TMX_SYSCFG_OLED_I2C_INST I2C_OLED_INST
#elif defined(I2C_0_INST)
#define TMX_SYSCFG_OLED_I2C_INST I2C_0_INST
#endif
#endif

#if !defined(TMX_SYSCFG_UART0_INST) && defined(UART_0_INST)
#define TMX_SYSCFG_UART0_INST UART_0_INST
#endif
#if !defined(TMX_SYSCFG_UART1_INST) && defined(UART_1_INST)
#define TMX_SYSCFG_UART1_INST UART_1_INST
#endif
#if !defined(TMX_SYSCFG_UART2_INST) && defined(UART_2_INST)
#define TMX_SYSCFG_UART2_INST UART_2_INST
#endif
#if !defined(TMX_SYSCFG_UART3_INST) && defined(UART_3_INST)
#define TMX_SYSCFG_UART3_INST UART_3_INST
#endif

#ifndef TMX_SYSCFG_MOTOR_A_IN1_PORT
#define TMX_SYSCFG_MOTOR_A_IN1_PORT GPIOB
#define TMX_SYSCFG_MOTOR_A_IN1_PIN DL_GPIO_PIN_17
#endif
#ifndef TMX_SYSCFG_MOTOR_A_IN2_PORT
#define TMX_SYSCFG_MOTOR_A_IN2_PORT GPIOB
#define TMX_SYSCFG_MOTOR_A_IN2_PIN DL_GPIO_PIN_19
#endif
#ifndef TMX_SYSCFG_MOTOR_B_IN1_PORT
#define TMX_SYSCFG_MOTOR_B_IN1_PORT GPIOA
#define TMX_SYSCFG_MOTOR_B_IN1_PIN DL_GPIO_PIN_16
#endif
#ifndef TMX_SYSCFG_MOTOR_B_IN2_PORT
#define TMX_SYSCFG_MOTOR_B_IN2_PORT GPIOB
#define TMX_SYSCFG_MOTOR_B_IN2_PIN DL_GPIO_PIN_24
#endif
#ifndef TMX_SYSCFG_BUZZER_PORT
#define TMX_SYSCFG_BUZZER_PORT GPIOA
#define TMX_SYSCFG_BUZZER_PIN DL_GPIO_PIN_7
#endif
#ifndef TMX_SYSCFG_RGB0_PORT
#define TMX_SYSCFG_RGB0_PORT GPIOB
#define TMX_SYSCFG_RGB0_PIN DL_GPIO_PIN_22
#endif
#ifndef TMX_SYSCFG_RGB1_PORT
#define TMX_SYSCFG_RGB1_PORT GPIOB
#define TMX_SYSCFG_RGB1_PIN DL_GPIO_PIN_5
#endif
#ifndef TMX_SYSCFG_RGB2_PORT
#define TMX_SYSCFG_RGB2_PORT GPIOA
#define TMX_SYSCFG_RGB2_PIN DL_GPIO_PIN_18
#endif

static void write_gpio(GPIO_Regs *port, uint32_t pin, uint8_t level)
{
    if (level != 0u) {
        DL_GPIO_setPins(port, pin);
    } else {
        DL_GPIO_clearPins(port, pin);
    }
}

static void TmxSyscfg_WriteSignal(tmx_signal_id_t signal, uint8_t level, void *user)
{
    (void)user;

    switch (signal) {
    case TMX_SIG_MOTOR_A_IN1:
        write_gpio(TMX_SYSCFG_MOTOR_A_IN1_PORT, TMX_SYSCFG_MOTOR_A_IN1_PIN, level);
        break;
    case TMX_SIG_MOTOR_A_IN2:
        write_gpio(TMX_SYSCFG_MOTOR_A_IN2_PORT, TMX_SYSCFG_MOTOR_A_IN2_PIN, level);
        break;
    case TMX_SIG_MOTOR_B_IN1:
        write_gpio(TMX_SYSCFG_MOTOR_B_IN1_PORT, TMX_SYSCFG_MOTOR_B_IN1_PIN, level);
        break;
    case TMX_SIG_MOTOR_B_IN2:
        write_gpio(TMX_SYSCFG_MOTOR_B_IN2_PORT, TMX_SYSCFG_MOTOR_B_IN2_PIN, level);
        break;
    case TMX_SIG_BUZZER:
        write_gpio(TMX_SYSCFG_BUZZER_PORT, TMX_SYSCFG_BUZZER_PIN, level);
        break;
    case TMX_SIG_RGB_PB22:
        write_gpio(TMX_SYSCFG_RGB0_PORT, TMX_SYSCFG_RGB0_PIN, level);
        break;
    case TMX_SIG_RGB_PB5:
        write_gpio(TMX_SYSCFG_RGB1_PORT, TMX_SYSCFG_RGB1_PIN, level);
        break;
    case TMX_SIG_RGB_PA18:
        write_gpio(TMX_SYSCFG_RGB2_PORT, TMX_SYSCFG_RGB2_PIN, level);
        break;
    default:
        break;
    }
}

static uint32_t permille_to_compare(uint16_t duty_permille, uint32_t period_ticks)
{
    uint32_t compare;

    if (period_ticks == 0u) {
        return 0u;
    }

    compare = ((uint32_t)duty_permille * period_ticks) / 1000u;
#if TMX_SYSCFG_PWM_COMPARE_INVERTED
    compare = (compare >= period_ticks) ? 0u : (period_ticks - compare);
#endif
    return (compare >= period_ticks) ? (period_ticks - 1u) : compare;
}

static uint32_t pulse_us_to_compare(uint16_t pulse_us)
{
    uint32_t compare = ((uint32_t)pulse_us * TMX_SYSCFG_SERVO_PWM_PERIOD_TICKS) /
                       TMX_SYSCFG_SERVO_PWM_PERIOD_US;

    if (compare >= TMX_SYSCFG_SERVO_PWM_PERIOD_TICKS) {
        compare = TMX_SYSCFG_SERVO_PWM_PERIOD_TICKS - 1u;
    }
    return compare;
}

static void TmxSyscfg_WritePwm(tmx_signal_id_t signal, uint16_t duty_permille, void *user)
{
    uint32_t compare = permille_to_compare(duty_permille, TMX_SYSCFG_MOTOR_PWM_PERIOD_TICKS);

    (void)user;

#if defined(TMX_SYSCFG_MOTOR_PWM_INST)
    switch (signal) {
    case TMX_SIG_MOTOR_A_PWM:
        DL_Timer_setCaptureCompareValue(TMX_SYSCFG_MOTOR_PWM_INST, compare,
                                        TMX_SYSCFG_MOTOR_A_PWM_CC_INDEX);
        break;
    case TMX_SIG_MOTOR_B_PWM:
        DL_Timer_setCaptureCompareValue(TMX_SYSCFG_MOTOR_PWM_INST, compare,
                                        TMX_SYSCFG_MOTOR_B_PWM_CC_INDEX);
        break;
    default:
        break;
    }
#else
    (void)signal;
    (void)compare;
#endif
}

static void TmxSyscfg_WriteServo(tmx_signal_id_t signal, uint16_t pulse_us, void *user)
{
    uint32_t compare = (pulse_us == 0u) ? 0u : pulse_us_to_compare(pulse_us);

    (void)user;

#if defined(TMX_SYSCFG_SERVO_PWM_INST)
    switch (signal) {
    case TMX_SIG_SERVO_1_PWM:
        DL_Timer_setCaptureCompareValue(TMX_SYSCFG_SERVO_PWM_INST, compare,
                                        TMX_SYSCFG_SERVO1_CC_INDEX);
        break;
    case TMX_SIG_SERVO_2_PWM:
        DL_Timer_setCaptureCompareValue(TMX_SYSCFG_SERVO_PWM_INST, compare,
                                        TMX_SYSCFG_SERVO2_CC_INDEX);
        break;
    case TMX_SIG_SERVO_3_PWM:
        DL_Timer_setCaptureCompareValue(TMX_SYSCFG_SERVO_PWM_INST, compare,
                                        TMX_SYSCFG_SERVO3_CC_INDEX);
        break;
    case TMX_SIG_SERVO_4_PWM:
        DL_Timer_setCaptureCompareValue(TMX_SYSCFG_SERVO_PWM_INST, compare,
                                        TMX_SYSCFG_SERVO4_CC_INDEX);
        break;
    default:
        break;
    }
#else
    (void)signal;
    (void)compare;
#endif
}

#if defined(TMX_SYSCFG_OLED_I2C_INST)
static uint8_t wait_i2c_status_clear(I2C_Regs *i2c, uint32_t mask)
{
    uint32_t timeout = TMX_SYSCFG_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(i2c) & mask) != 0u) {
        if (timeout == 0u) {
            return 0u;
        }
        --timeout;
    }
    return 1u;
}

static uint8_t TmxSyscfg_I2cProbe(uint8_t bus_index, uint8_t address_7bit, void *user)
{
    (void)user;

    if ((bus_index != 0u) || (address_7bit == 0u) || (address_7bit > 0x7fu)) {
        return 0u;
    }

    DL_I2C_flushControllerTXFIFO(TMX_SYSCFG_OLED_I2C_INST);
    DL_I2C_resetControllerTransfer(TMX_SYSCFG_OLED_I2C_INST);

    if (wait_i2c_status_clear(TMX_SYSCFG_OLED_I2C_INST,
                              DL_I2C_CONTROLLER_STATUS_BUSY_BUS) == 0u) {
        return 0u;
    }

#if TMX_SYSCFG_I2C_PROBE_SEND_OLED_CONTROL_BYTE
    {
        uint8_t probe_byte = 0x00u;
        (void)DL_I2C_fillControllerTXFIFO(TMX_SYSCFG_OLED_I2C_INST, &probe_byte, 1u);
        DL_I2C_startControllerTransfer(TMX_SYSCFG_OLED_I2C_INST, address_7bit,
                                       DL_I2C_CONTROLLER_DIRECTION_TX, 1u);
    }
#else
    DL_I2C_startControllerTransfer(TMX_SYSCFG_OLED_I2C_INST, address_7bit,
                                   DL_I2C_CONTROLLER_DIRECTION_TX, 0u);
#endif

    if (wait_i2c_status_clear(TMX_SYSCFG_OLED_I2C_INST,
                              DL_I2C_CONTROLLER_STATUS_BUSY_BUS) == 0u) {
        DL_I2C_resetControllerTransfer(TMX_SYSCFG_OLED_I2C_INST);
        return 0u;
    }

    if ((DL_I2C_getControllerStatus(TMX_SYSCFG_OLED_I2C_INST) &
         DL_I2C_CONTROLLER_STATUS_ERROR) != 0u) {
        DL_I2C_resetControllerTransfer(TMX_SYSCFG_OLED_I2C_INST);
        return 0u;
    }

    return 1u;
}
#endif

static UART_Regs *uart_inst_for_index(uint8_t uart_index)
{
    switch (uart_index) {
#if defined(TMX_SYSCFG_UART0_INST)
    case 0u:
        return TMX_SYSCFG_UART0_INST;
#endif
#if defined(TMX_SYSCFG_UART1_INST)
    case 1u:
        return TMX_SYSCFG_UART1_INST;
#endif
#if defined(TMX_SYSCFG_UART2_INST)
    case 2u:
        return TMX_SYSCFG_UART2_INST;
#endif
#if defined(TMX_SYSCFG_UART3_INST)
    case 3u:
        return TMX_SYSCFG_UART3_INST;
#endif
    default:
        return 0;
    }
}

static void TmxSyscfg_UartWrite(uint8_t uart_index,
                                const uint8_t *data,
                                uint16_t len,
                                void *user)
{
    UART_Regs *uart = uart_inst_for_index(uart_index);

    (void)user;

    if ((uart == 0) || (data == 0)) {
        return;
    }

    for (uint16_t i = 0u; i < len; ++i) {
        DL_UART_Main_transmitDataBlocking(uart, data[i]);
    }
}

static void TmxSyscfg_DelayMs(uint16_t delay_ms, void *user)
{
    (void)user;
    delay_cycles((uint32_t)delay_ms * 32000u);
}

tmx_board_ops_t Board_BuildTmxOps(void)
{
    tmx_board_ops_t ops = {
        .write_signal = TmxSyscfg_WriteSignal,
        .read_signal = 0,
        .write_pwm = TmxSyscfg_WritePwm,
        .write_servo_pulse = TmxSyscfg_WriteServo,
#if defined(TMX_SYSCFG_OLED_I2C_INST)
        .i2c_probe = TmxSyscfg_I2cProbe,
#else
        .i2c_probe = 0,
#endif
        .uart_write = TmxSyscfg_UartWrite,
        .delay_ms = TmxSyscfg_DelayMs,
        .user = 0,
    };

    return ops;
}

#else

void TmxSyscfgAdapterExample_RequiresTiDriverlib(void)
{
}

#endif
