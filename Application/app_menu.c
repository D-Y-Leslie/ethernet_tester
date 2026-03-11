#include "app_menu.h"
#include "bsp_led.h"

static APP_MenuItem_t g_menuItem = APP_MENU_ITEM_HOME;
static APP_MenuState_t g_menuState = APP_MENU_STATE_BROWSE;

static void APP_Menu_ShowBrowseState(void)
{
    switch (g_menuItem) {
    case APP_MENU_ITEM_HOME:
        BSP_LED_SetColor(BSP_LED_COLOR_RED);
        break;
    case APP_MENU_ITEM_CABLE_TEST:
        BSP_LED_SetColor(BSP_LED_COLOR_GREEN);
        break;
    case APP_MENU_ITEM_SIGNAL_GEN:
        BSP_LED_SetColor(BSP_LED_COLOR_BLUE);
        break;
    case APP_MENU_ITEM_DETECTOR:
        BSP_LED_SetColor(BSP_LED_COLOR_YELLOW);
        break;
    case APP_MENU_ITEM_ABOUT:
        BSP_LED_SetColor(BSP_LED_COLOR_CYAN);
        break;
    default:
        BSP_LED_SetColor(BSP_LED_COLOR_OFF);
        break;
    }
}

static void APP_Menu_ShowEnterState(void)
{
    switch (g_menuItem) {
    case APP_MENU_ITEM_HOME:
        BSP_LED_SetColor(BSP_LED_COLOR_WHITE);
        break;
    case APP_MENU_ITEM_CABLE_TEST:
        BSP_LED_SetColor(BSP_LED_COLOR_MAGENTA);
        break;
    case APP_MENU_ITEM_SIGNAL_GEN:
        BSP_LED_SetColor(BSP_LED_COLOR_CYAN);
        break;
    case APP_MENU_ITEM_DETECTOR:
        BSP_LED_SetColor(BSP_LED_COLOR_WHITE);
        break;
    case APP_MENU_ITEM_ABOUT:
        BSP_LED_SetColor(BSP_LED_COLOR_MAGENTA);
        break;
    default:
        BSP_LED_SetColor(BSP_LED_COLOR_OFF);
        break;
    }
}

static void APP_Menu_RefreshLed(void)
{
    if (g_menuState == APP_MENU_STATE_BROWSE) {
        APP_Menu_ShowBrowseState();
    } else {
        APP_Menu_ShowEnterState();
    }
}

static void APP_Menu_Prev(void)
{
    if (g_menuItem == APP_MENU_ITEM_HOME) {
        g_menuItem = (APP_MenuItem_t)(APP_MENU_ITEM_MAX - 1);
    } else {
        g_menuItem = (APP_MenuItem_t)(g_menuItem - 1);
    }
}

static void APP_Menu_Next(void)
{
    g_menuItem = (APP_MenuItem_t)((g_menuItem + 1) % APP_MENU_ITEM_MAX);
}

void APP_Menu_Init(void)
{
    g_menuItem = APP_MENU_ITEM_HOME;
    g_menuState = APP_MENU_STATE_BROWSE;
    APP_Menu_RefreshLed();
}

void APP_Menu_Process(const BSP_InputEvent_t *event)
{
    int8_t i;

    if (event == 0) {
        return;
    }

    if (g_menuState == APP_MENU_STATE_BROWSE) {
        if (event->key_short_mask & BSP_KEY_MASK_LEFT) {
            APP_Menu_Prev();
        }

        if (event->key_short_mask & BSP_KEY_MASK_RIGHT) {
            APP_Menu_Next();
        }

        if (event->enc_delta > 0) {
            for (i = 0; i < event->enc_delta; i++) {
                APP_Menu_Next();
            }
        } else if (event->enc_delta < 0) {
            for (i = 0; i < -event->enc_delta; i++) {
                APP_Menu_Prev();
            }
        }

        if (event->key_short_mask & BSP_KEY_MASK_MID) {
            g_menuState = APP_MENU_STATE_ENTER;
        }
    } else {
        if (event->key_long_mask & BSP_KEY_MASK_MID) {
            g_menuState = APP_MENU_STATE_BROWSE;
        }

        if (event->key_short_mask & BSP_KEY_MASK_LEFT) {
            g_menuState = APP_MENU_STATE_BROWSE;
        }
    }

    APP_Menu_RefreshLed();
}

APP_MenuItem_t APP_Menu_GetCurrentItem(void)
{
    return g_menuItem;
}

APP_MenuState_t APP_Menu_GetState(void)
{
    return g_menuState;
}