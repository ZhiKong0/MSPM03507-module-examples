#include "board_app.h"
#include "ti_msp_dl_config.h"

int main(void)
{
    SYSCFG_DL_init();
    BoardApp_Init();

    while (1) {
        BoardApp_Poll();
        delay_cycles(CPUCLK_FREQ / 1000u);
    }
}
