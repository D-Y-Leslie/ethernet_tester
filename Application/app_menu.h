#ifndef APP_MENU_H
#define APP_MENU_H

#include <stdint.h>
#include "bsp_input.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    APP_MENU_ITEM_HOME = 0,
    APP_MENU_ITEM_CABLE_TEST,
    APP_MENU_ITEM_SIGNAL_GEN,
    APP_MENU_ITEM_DETECTOR,
    APP_MENU_ITEM_ABOUT,
    APP_MENU_ITEM_MAX
} APP_MenuItem_t;

typedef enum
{
    APP_MENU_STATE_BROWSE = 0,
    APP_MENU_STATE_ENTER
} APP_MenuState_t;

void APP_Menu_Init(void);
void APP_Menu_Process(const BSP_InputEvent_t *event);

APP_MenuItem_t APP_Menu_GetCurrentItem(void);
APP_MenuState_t APP_Menu_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MENU_H */