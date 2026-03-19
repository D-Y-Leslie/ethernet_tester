#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include "bsp_input.h"

void APP_Debug_Init(void);
void APP_Debug_PrintInputEvent(const BSP_InputEvent_t *event);

#endif