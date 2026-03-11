#include "ti_msp_dl_config.h"
#include "bsp_led.h"
#include "bsp_input.h"
#include "app_menu.h"

#define CPU_CLOCK_HZ            (32000000U)
#define MAIN_LOOP_PERIOD_MS     (5U)
#define MAIN_LOOP_DELAY_CYCLES  ((CPU_CLOCK_HZ / 1000U) * MAIN_LOOP_PERIOD_MS)

int main(void)
{
    BSP_InputEvent_t inputEvent;

    SYSCFG_DL_init();

    BSP_LED_Init();
    BSP_Input_Init();
    APP_Menu_Init();

    while (1) {
        BSP_Input_Scan();

        if (BSP_Input_FetchEvents(&inputEvent)) {
            APP_Menu_Process(&inputEvent);
        }

        delay_cycles(MAIN_LOOP_DELAY_CYCLES);
    }
}