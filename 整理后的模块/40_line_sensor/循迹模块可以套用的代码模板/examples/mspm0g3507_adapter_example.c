#include "line_trace_template.h"
#include "ti_msp_dl_config.h"

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

void Board_LinePinsInit(void)
{
    /*
     * Put your SysConfig-generated GPIO init here, or initialize each IOMUX
     * with pull-up and hysteresis before reading the line sensor.
     */
}
