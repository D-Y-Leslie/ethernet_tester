#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint.h>

typedef enum
{
    BSP_LED_COLOR_OFF = 0,
    BSP_LED_COLOR_RED,
    BSP_LED_COLOR_GREEN,
    BSP_LED_COLOR_BLUE,
    BSP_LED_COLOR_YELLOW,
    BSP_LED_COLOR_CYAN,
    BSP_LED_COLOR_MAGENTA,
    BSP_LED_COLOR_WHITE
} BSP_LED_Color_t;

void BSP_LED_Init(void);
void BSP_LED_Off(void);
void BSP_LED_SetColor(BSP_LED_Color_t color);

#endif /* BSP_LED_H */