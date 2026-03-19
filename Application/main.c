#include "ti_msp_dl_config.h"
#include "bsp_led.h"
#include "bsp_input.h"
#include <stdio.h>
#include <stdint.h>

#define CPU_CLOCK_HZ            (32000000U)
#define MAIN_LOOP_PERIOD_MS     (5U)
#define MAIN_LOOP_DELAY_CYCLES  ((CPU_CLOCK_HZ / 1000U) * MAIN_LOOP_PERIOD_MS)

#define KEY_VALID_MASK  (BSP_KEY_MASK_LEFT  | \
                         BSP_KEY_MASK_DOWN  | \
                         BSP_KEY_MASK_RIGHT | \
                         BSP_KEY_MASK_UP    | \
                         BSP_KEY_MASK_MID)

static void UART_SendChar(char ch)
{
    DL_UART_Main_transmitDataBlocking(UART_DEBUG_INST, (uint8_t)ch);
}

static void UART_SendString(const char *str)
{
    while (*str != '\0') {
        UART_SendChar(*str++);
    }
}

static void PrintKeyNames(const char *tag, uint8_t mask)
{
    UART_SendString(tag);

    if (mask == 0U) {
        UART_SendString("NONE\r\n");
        return;
    }

    if (mask & BSP_KEY_MASK_LEFT)  UART_SendString("LEFT ");
    if (mask & BSP_KEY_MASK_DOWN)  UART_SendString("DOWN ");
    if (mask & BSP_KEY_MASK_RIGHT) UART_SendString("RIGHT ");
    if (mask & BSP_KEY_MASK_UP)    UART_SendString("UP ");
    if (mask & BSP_KEY_MASK_MID)   UART_SendString("MID ");

    UART_SendString("\r\n");
}

int main(void)
{
    BSP_InputEvent_t inputEvent;
    uint8_t shortMask, longMask;
    uint8_t shortUnknown, longUnknown;
    uint8_t lastUnknownShort = 0xFF;
    uint8_t lastUnknownLong  = 0xFF;
    char buf[80];

    SYSCFG_DL_init();

    BSP_LED_Init();
    BSP_Input_Init();

    UART_SendString("\r\n==== key debug start ====\r\n");

    while (1) {

        BSP_Input_Scan();

        if (BSP_Input_FetchEvents(&inputEvent)) {

            /* 只保留 5 个已知按键 */
            shortMask = (uint8_t)(inputEvent.key_short_mask & KEY_VALID_MASK);
            longMask  = (uint8_t)(inputEvent.key_long_mask  & KEY_VALID_MASK);

            /* 把未知位单独抓出来，例如你现在一直出现的 0x20 */
            shortUnknown = (uint8_t)(inputEvent.key_short_mask & (uint8_t)(~KEY_VALID_MASK));
            longUnknown  = (uint8_t)(inputEvent.key_long_mask  & (uint8_t)(~KEY_VALID_MASK));

            /* 未知位只在变化时提示一次，避免刷屏 */
            if ((shortUnknown != lastUnknownShort) || (longUnknown != lastUnknownLong)) {
                if ((shortUnknown != 0U) || (longUnknown != 0U)) {
                    snprintf(buf, sizeof(buf),
                             "WARN unknown short=0x%02X long=0x%02X\r\n",
                             shortUnknown, longUnknown);
                    UART_SendString(buf);
                }
                lastUnknownShort = shortUnknown;
                lastUnknownLong  = longUnknown;
            }

            /* 只输出已知按键 */
            if (shortMask != 0U) {
                PrintKeyNames("SHORT: ", shortMask);
            }
            if (longMask != 0U) {
                PrintKeyNames("LONG : ", longMask);
            }
        }

        delay_cycles(MAIN_LOOP_DELAY_CYCLES);
    }
}