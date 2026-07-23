#include "line_trace_template.h"
#include "ti_msp_dl_config.h"

#include <string.h>

typedef struct {
    GPIO_Regs *port;
    uint32_t pin;
} board_line_pin_t;

static const board_line_pin_t g_line_pins[8] = {
    { GPIOA, DL_GPIO_PIN_15 },
    { GPIOA, DL_GPIO_PIN_16 },
    { GPIOA, DL_GPIO_PIN_17 },
    { GPIOA, DL_GPIO_PIN_7  },
    { GPIOB, DL_GPIO_PIN_2  },
    { GPIOB, DL_GPIO_PIN_3  },
    { GPIOB, DL_GPIO_PIN_4  },
    { GPIOB, DL_GPIO_PIN_5  },
};

uint8_t Board_LineReadLevel(uint8_t channel, void *user)
{
    (void)user;

    if (channel >= 8u) {
        return 1u;
    }

    return (DL_GPIO_readPins(g_line_pins[channel].port,
                             g_line_pins[channel].pin) != 0u) ? 1u : 0u;
}

uint8_t Board_LineReadRawBits(void)
{
    uint8_t raw_bits = 0u;

    for (uint8_t i = 0u; i < 8u; ++i) {
        if (Board_LineReadLevel(i, 0) != 0u) {
            raw_bits = (uint8_t)(raw_bits | (uint8_t)(1u << i));
        }
    }

    return raw_bits;
}

uint8_t Board_LineReadSampleFrame(line_trace_sample_frame_t *out)
{
    const line_trace_sensor_params_t default_sensor = {
        .channel_count = 8u,
        .active_level = LINE_TRACE_ACTIVE_LOW,
        .threshold_mode = LINE_TRACE_THRESHOLD_DIGITAL_GPIO,
        .i2c_addr = 0u,
        .sensor_enable_mask = 0xffu,
    };

    return LineTrace_BuildSampleFrame(&default_sensor, Board_LineReadRawBits(), 0u,
                                      LINE_TRACE_SENSOR_OK, out);
}

uint8_t Board_LineBuildSampleFrame(const line_trace_sensor_params_t *sensor,
                                   line_trace_sample_frame_t *out)
{
    if (out == 0) {
        return 0u;
    }

    /*
     * Comparator/threshold modules and I2C line sensors should also end here:
     * fill raw_bits after digitizing, set threshold_status/sensor_status, then
     * keep the algorithm layer unchanged.
     */
    return LineTrace_BuildSampleFrame(sensor, Board_LineReadRawBits(), 0u,
                                      LINE_TRACE_SENSOR_OK, out);
}

void Board_LinePinsInit(void)
{
    /*
     * Put your SysConfig-generated GPIO init here, or initialize each IOMUX
     * with pull-up and hysteresis before reading the line sensor.
     */
}
