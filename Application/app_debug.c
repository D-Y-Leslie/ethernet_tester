#include "app_debug.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>
#include <stdint.h>

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

static void APP_Debug_PrintMaskName(const char *tag, uint8_t mask)
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

void APP_Debug_Init(void)
{
    UART_SendString("\r\n==== app debug start ====\r\n");
}

void APP_Debug_PrintInputEvent(const BSP_InputEvent_t *event)
{
    char buf[32];

    if (event == 0) {
        return;
    }

    if (event->key_short_mask != 0U) {
        APP_Debug_PrintMaskName("SHORT: ", event->key_short_mask);
    }

    if (event->key_long_mask != 0U) {
        APP_Debug_PrintMaskName("LONG : ", event->key_long_mask);
    }

    if (event->enc_delta != 0) {
        snprintf(buf, sizeof(buf), "ENC  : %d\r\n", event->enc_delta);
        UART_SendString(buf);
    }
}