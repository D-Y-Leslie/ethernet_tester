#include "bsp_led.h"
#include "ti_msp_dl_config.h"
#include "ti/driverlib/dl_gpio.h"

/* 已确认：
 * PA2 -> R
 * PA3 -> B
 * PA4 -> G
 * 且为高电平点亮
 */
#define LED_R_PORT      GPIOA
#define LED_R_PIN       DL_GPIO_PIN_2

#define LED_B_PORT      GPIOA
#define LED_B_PIN       DL_GPIO_PIN_3

#define LED_G_PORT      GPIOA
#define LED_G_PIN       DL_GPIO_PIN_4

static void BSP_LED_SetRGB(uint8_t r_on, uint8_t g_on, uint8_t b_on)
{
    if (r_on) {
        DL_GPIO_setPins(LED_R_PORT, LED_R_PIN);
    } else {
        DL_GPIO_clearPins(LED_R_PORT, LED_R_PIN);
    }

    if (g_on) {
        DL_GPIO_setPins(LED_G_PORT, LED_G_PIN);
    } else {
        DL_GPIO_clearPins(LED_G_PORT, LED_G_PIN);
    }

    if (b_on) {
        DL_GPIO_setPins(LED_B_PORT, LED_B_PIN);
    } else {
        DL_GPIO_clearPins(LED_B_PORT, LED_B_PIN);
    }
}

void BSP_LED_Init(void)
{
    BSP_LED_Off();
}

void BSP_LED_Off(void)
{
    BSP_LED_SetRGB(0, 0, 0);
}

void BSP_LED_SetColor(BSP_LED_Color_t color)
{
    switch (color) {
    case BSP_LED_COLOR_OFF:
        BSP_LED_SetRGB(0, 0, 0);
        break;

    case BSP_LED_COLOR_RED:
        BSP_LED_SetRGB(1, 0, 0);
        break;

    case BSP_LED_COLOR_GREEN:
        BSP_LED_SetRGB(0, 1, 0);
        break;

    case BSP_LED_COLOR_BLUE:
        BSP_LED_SetRGB(0, 0, 1);
        break;

    case BSP_LED_COLOR_YELLOW:
        BSP_LED_SetRGB(1, 1, 0);
        break;

    case BSP_LED_COLOR_CYAN:
        BSP_LED_SetRGB(0, 1, 1);
        break;

    case BSP_LED_COLOR_MAGENTA:
        BSP_LED_SetRGB(1, 0, 1);
        break;

    case BSP_LED_COLOR_WHITE:
        BSP_LED_SetRGB(1, 1, 1);
        break;

    default:
        BSP_LED_SetRGB(0, 0, 0);
        break;
    }
}