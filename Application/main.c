#include "ti_msp_dl_config.h"
#include "bsp_led.h"
#include "bsp_input.h"
#include "app_menu.h"
#include "app_debug.h"

#define CPU_CLOCK_HZ            (32000000U)
#define MAIN_LOOP_PERIOD_MS     (5U)
#define MAIN_LOOP_DELAY_CYCLES  ((CPU_CLOCK_HZ / 1000U) * MAIN_LOOP_PERIOD_MS)

/* 需要串口调试就改成 1 */
#define APP_ENABLE_DEBUG_UART   1

int main(void)
{
    BSP_InputEvent_t inputEvent;

    SYSCFG_DL_init();

    BSP_LED_Init();
    BSP_Input_Init();
    APP_Menu_Init();

#if APP_ENABLE_DEBUG_UART
    APP_Debug_Init();
#endif

    while (1) {

        BSP_Input_Scan();

        if (BSP_Input_FetchEvents(&inputEvent)) {

#if APP_ENABLE_DEBUG_UART
            APP_Debug_PrintInputEvent(&inputEvent);
#endif

            APP_Menu_Process(&inputEvent);
        }

        delay_cycles(MAIN_LOOP_DELAY_CYCLES);
    }
}