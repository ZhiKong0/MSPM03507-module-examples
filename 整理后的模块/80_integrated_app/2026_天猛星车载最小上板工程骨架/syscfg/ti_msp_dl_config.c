#include "ti_msp_dl_config.h"

SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_0_init();
    SYSCFG_DL_UART_0_init();
    SYSCFG_DL_I2C_OLED_init();
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerG_reset(PWM_0_INST);
    DL_UART_Main_reset(UART_0_INST);
    DL_I2C_reset(I2C_OLED_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_enablePower(PWM_0_INST);
    DL_UART_Main_enablePower(UART_0_INST);
    DL_I2C_enablePower(I2C_OLED_INST);

    delay_cycles(POWER_STARTUP_DELAY);
}

static void init_line_input(uint32_t iomux)
{
    DL_GPIO_initDigitalInputFeatures(iomux,
                                     DL_GPIO_INVERSION_DISABLE,
                                     DL_GPIO_RESISTOR_PULL_UP,
                                     DL_GPIO_HYSTERESIS_ENABLE,
                                     DL_GPIO_WAKEUP_DISABLE);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_0_C0_IOMUX,
                                         GPIO_PWM_0_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_0_C0_PORT, GPIO_PWM_0_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_0_C1_IOMUX,
                                         GPIO_PWM_0_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_0_C1_PORT, GPIO_PWM_0_C1_PIN);

    DL_GPIO_initPeripheralOutputFunction(GPIO_UART_0_IOMUX_TX,
                                         GPIO_UART_0_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_UART_0_IOMUX_RX,
                                        GPIO_UART_0_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_OLED_IOMUX_SDA,
                                                GPIO_I2C_OLED_IOMUX_SDA_FUNC,
                                                DL_GPIO_INVERSION_DISABLE,
                                                DL_GPIO_RESISTOR_NONE,
                                                DL_GPIO_HYSTERESIS_DISABLE,
                                                DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_OLED_IOMUX_SCL,
                                                GPIO_I2C_OLED_IOMUX_SCL_FUNC,
                                                DL_GPIO_INVERSION_DISABLE,
                                                DL_GPIO_RESISTOR_NONE,
                                                DL_GPIO_HYSTERESIS_DISABLE,
                                                DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_OLED_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_OLED_IOMUX_SCL);

    DL_GPIO_initDigitalOutput(MOTOR_A_IN1_IOMUX);
    DL_GPIO_initDigitalOutput(MOTOR_A_IN2_IOMUX);
    DL_GPIO_initDigitalOutput(MOTOR_B_IN1_IOMUX);
    DL_GPIO_initDigitalOutput(MOTOR_B_IN2_IOMUX);
    DL_GPIO_initDigitalOutput(BUZZER_IOMUX);
    DL_GPIO_initDigitalOutput(RGB0_IOMUX);
    DL_GPIO_initDigitalOutput(RGB1_IOMUX);
    DL_GPIO_initDigitalOutput(RGB2_IOMUX);

    DL_GPIO_clearPins(GPIOA, MOTOR_B_IN1_PIN | BUZZER_PIN | RGB2_PIN);
    DL_GPIO_enableOutput(GPIOA, MOTOR_B_IN1_PIN | BUZZER_PIN | RGB2_PIN);
    DL_GPIO_clearPins(GPIOB,
                      MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN | MOTOR_B_IN2_PIN |
                          RGB0_PIN | RGB1_PIN);
    DL_GPIO_enableOutput(GPIOB,
                         MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN | MOTOR_B_IN2_PIN |
                             RGB0_PIN | RGB1_PIN);

    init_line_input(LINE_TRACE_CH0_IOMUX);
    init_line_input(LINE_TRACE_CH1_IOMUX);
    init_line_input(LINE_TRACE_CH2_IOMUX);
    init_line_input(LINE_TRACE_CH3_IOMUX);
    init_line_input(LINE_TRACE_CH4_IOMUX);
    init_line_input(LINE_TRACE_CH5_IOMUX);
    init_line_input(LINE_TRACE_CH6_IOMUX);
    init_line_input(LINE_TRACE_CH7_IOMUX);
}

SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_disableHFXT();
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
}

static const DL_TimerG_ClockConfig gPWM0ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0u,
};

static const DL_TimerG_PWMConfig gPWM0Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = PWM_0_PERIOD_TICKS,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_0_init(void)
{
    DL_TimerG_setClockConfig(PWM_0_INST, (DL_TimerG_ClockConfig *)&gPWM0ClockConfig);
    DL_TimerG_initPWMMode(PWM_0_INST, (DL_TimerG_PWMConfig *)&gPWM0Config);

    DL_TimerG_setCounterControl(PWM_0_INST,
                                DL_TIMER_CZC_CCCTL0_ZCOND,
                                DL_TIMER_CAC_CCCTL0_ACOND,
                                DL_TIMER_CLC_CCCTL0_LCOND);
    DL_TimerG_setCaptureCompareOutCtl(PWM_0_INST,
                                      DL_TIMER_CC_OCTL_INIT_VAL_LOW,
                                      DL_TIMER_CC_OCTL_INV_OUT_DISABLED,
                                      DL_TIMER_CC_OCTL_SRC_FUNCVAL,
                                      DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptCompUpdateMethod(PWM_0_INST,
                                      DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE,
                                      DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0u, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareOutCtl(PWM_0_INST,
                                      DL_TIMER_CC_OCTL_INIT_VAL_LOW,
                                      DL_TIMER_CC_OCTL_INV_OUT_DISABLED,
                                      DL_TIMER_CC_OCTL_SRC_FUNCVAL,
                                      DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptCompUpdateMethod(PWM_0_INST,
                                      DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE,
                                      DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0u, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(PWM_0_INST);
    DL_TimerG_setCCPDirection(PWM_0_INST, DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT);
    DL_TimerG_startCounter(PWM_0_INST);
}

static const DL_UART_Main_ClockConfig gUART0ClockConfig = {
    .clockSel = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1,
};

static const DL_UART_Main_Config gUART0Config = {
    .mode = DL_UART_MAIN_MODE_NORMAL,
    .direction = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity = DL_UART_MAIN_PARITY_NONE,
    .wordLength = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits = DL_UART_MAIN_STOP_BITS_ONE,
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_0_init(void)
{
    DL_UART_Main_setClockConfig(UART_0_INST, (DL_UART_Main_ClockConfig *)&gUART0ClockConfig);
    DL_UART_Main_init(UART_0_INST, (DL_UART_Main_Config *)&gUART0Config);
    DL_UART_Main_setOversampling(UART_0_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_0_INST,
                                    UART_0_IBRD_32_MHZ_115200_BAUD,
                                    UART_0_FBRD_32_MHZ_115200_BAUD);
    DL_UART_Main_setRXFIFOThreshold(UART_0_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_enable(UART_0_INST);
}

static const DL_I2C_ClockConfig gI2COledClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_I2C_OLED_init(void)
{
    DL_I2C_setClockConfig(I2C_OLED_INST, (DL_I2C_ClockConfig *)&gI2COledClockConfig);
    DL_I2C_disableAnalogGlitchFilter(I2C_OLED_INST);
    DL_I2C_resetControllerTransfer(I2C_OLED_INST);
    DL_I2C_setTimerPeriod(I2C_OLED_INST, 7u);
    DL_I2C_setControllerTXFIFOThreshold(I2C_OLED_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(I2C_OLED_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(I2C_OLED_INST);
    DL_I2C_enableController(I2C_OLED_INST);
}
