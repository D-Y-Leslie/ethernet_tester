#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include "bsp_input.h"

#ifdef __cplusplus
extern "C" {
#endif

void APP_Debug_Init(void);
void APP_Debug_PrintInputEvent(const BSP_InputEvent_t *event);

#ifdef __cplusplus
}
#endif

#endif /* APP_DEBUG_H */