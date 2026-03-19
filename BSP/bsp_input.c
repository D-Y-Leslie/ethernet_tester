#include "bsp_input.h"
#include "ti_msp_dl_config.h"
#include "ti/driverlib/dl_gpio.h"

#define KEY_PORT            GPIOB
#define KEY_LEFT_PIN        DL_GPIO_PIN_6
#define KEY_DOWN_PIN        DL_GPIO_PIN_7
#define KEY_RIGHT_PIN       DL_GPIO_PIN_8
#define KEY_UP_PIN          DL_GPIO_PIN_9
#define KEY_MID_PIN         DL_GPIO_PIN_14

#define ENC_PORT            GPIOB
#define ENC_A_PIN           DL_GPIO_PIN_15
#define ENC_B_PIN           DL_GPIO_PIN_16

typedef struct
{
    uint8_t stable_level;     /* 当前稳定电平：1=松开，0=按下 */
    uint8_t filter_level;     /* 当前去抖观察电平 */
    uint8_t debounce_cnt;     /* 去抖计数 */
    uint16_t hold_cnt;        /* 稳定按下持续计数 */
    uint8_t short_armed;      /* 释放后允许下一次 short */
    uint8_t long_sent;        /* 本次按下已经发过 long */
} BSP_KeyState_t;

static BSP_KeyState_t g_keyState[BSP_KEY_MAX];
static BSP_InputEvent_t g_inputEvent;

static uint8_t g_encPrevAB = 0U;
static int8_t  g_encAcc = 0;

static uint32_t BSP_Input_GetKeyPin(BSP_Key_t key)
{
    switch (key) {
    case BSP_KEY_LEFT:  return KEY_LEFT_PIN;
    case BSP_KEY_DOWN:  return KEY_DOWN_PIN;
    case BSP_KEY_RIGHT: return KEY_RIGHT_PIN;
    case BSP_KEY_UP:    return KEY_UP_PIN;
    case BSP_KEY_MID:   return KEY_MID_PIN;
    default:            return 0U;
    }
}

/* 返回值：1=松开，0=按下 */
static uint8_t BSP_Input_ReadKeyRaw(BSP_Key_t key)
{
    uint32_t pin = BSP_Input_GetKeyPin(key);

    if (pin == 0U) {
        return 1U;
    }

    return (DL_GPIO_readPins(KEY_PORT, pin) != 0U) ? 1U : 0U;
}

static uint8_t BSP_Input_ReadEncAB(void)
{
    uint8_t a = (DL_GPIO_readPins(ENC_PORT, ENC_A_PIN) != 0U) ? 1U : 0U;
    uint8_t b = (DL_GPIO_readPins(ENC_PORT, ENC_B_PIN) != 0U) ? 1U : 0U;

    return (uint8_t)((a << 1) | b);
}

void BSP_Input_ClearAllEvents(void)
{
    g_inputEvent.key_short_mask = 0U;
    g_inputEvent.key_long_mask  = 0U;
    g_inputEvent.enc_delta      = 0;
}

void BSP_Input_Init(void)
{
    uint32_t i;

    for (i = 0; i < BSP_KEY_MAX; i++) {
        uint8_t raw = BSP_Input_ReadKeyRaw((BSP_Key_t)i);

        g_keyState[i].stable_level = raw;
        g_keyState[i].filter_level = raw;
        g_keyState[i].debounce_cnt = 0U;
        g_keyState[i].hold_cnt     = 0U;
        g_keyState[i].long_sent    = 0U;
        g_keyState[i].short_armed  = (raw != 0U) ? 1U : 0U;
    }

    BSP_Input_ClearAllEvents();

    g_encPrevAB = BSP_Input_ReadEncAB();
    g_encAcc    = 0;
}

static void BSP_Input_ScanOneKey(BSP_Key_t key)
{
    uint8_t raw;
    BSP_KeyState_t *ks;

    raw = BSP_Input_ReadKeyRaw(key);
    ks  = &g_keyState[key];

    /* 去抖观察阶段 */
    if (raw != ks->filter_level) {
        ks->filter_level = raw;
        ks->debounce_cnt = 0U;
    } else {
        if (ks->debounce_cnt < BSP_KEY_DEBOUNCE_COUNT) {
            ks->debounce_cnt++;
        }
    }

    /* 尚未达到稳定判定 */
    if (ks->debounce_cnt < BSP_KEY_DEBOUNCE_COUNT) {
        return;
    }

    /* 稳定边沿 */
    if (ks->stable_level != ks->filter_level) {
        ks->stable_level = ks->filter_level;

        if (ks->stable_level == 0U) {
            /* 稳定按下 */
            ks->hold_cnt  = 0U;
            ks->long_sent = 0U;

            if (ks->short_armed != 0U) {
                g_inputEvent.key_short_mask |= (uint8_t)(1U << key);
                ks->short_armed = 0U;
            }
        } else {
            /* 稳定释放 */
            ks->hold_cnt    = 0U;
            ks->long_sent   = 0U;
            ks->short_armed = 1U;
        }

        return;
    }

    /* 稳定保持按下 */
    if (ks->stable_level == 0U) {
        if (ks->hold_cnt < 0xFFFFU) {
            ks->hold_cnt++;
        }

        if ((ks->hold_cnt >= BSP_KEY_LONG_PRESS_COUNT) &&
            (ks->long_sent == 0U)) {
            g_inputEvent.key_long_mask |= (uint8_t)(1U << key);
            ks->long_sent = 1U;
        }
    }
}

static void BSP_Input_ScanKeys(void)
{
    uint32_t i;

    for (i = 0; i < BSP_KEY_MAX; i++) {
        BSP_Input_ScanOneKey((BSP_Key_t)i);
    }
}

static void BSP_Input_ScanEncoder(void)
{
    static const int8_t transTable[16] = {
         0, -1, +1,  0,
        +1,  0,  0, -1,
        -1,  0,  0, +1,
         0, +1, -1,  0
    };

    uint8_t currAB;
    uint8_t idx;
    int8_t step;

    currAB = BSP_Input_ReadEncAB();
    idx = (uint8_t)((g_encPrevAB << 2) | currAB);
    step = transTable[idx];

    if (step != 0) {
        g_encAcc += step;

        if (g_encAcc >= 4) {
            if (g_inputEvent.enc_delta < 127) {
                g_inputEvent.enc_delta++;
            }
            g_encAcc = 0;
        } else if (g_encAcc <= -4) {
            if (g_inputEvent.enc_delta > -128) {
                g_inputEvent.enc_delta--;
            }
            g_encAcc = 0;
        }
    }

    g_encPrevAB = currAB;
}

void BSP_Input_Scan(void)
{
    BSP_Input_ScanKeys();
    BSP_Input_ScanEncoder();
}

bool BSP_Input_FetchEvents(BSP_InputEvent_t *event)
{
    if (event == 0) {
        return false;
    }

    if ((g_inputEvent.key_short_mask == 0U) &&
        (g_inputEvent.key_long_mask  == 0U) &&
        (g_inputEvent.enc_delta      == 0)) {
        return false;
    }

    *event = g_inputEvent;
    BSP_Input_ClearAllEvents();
    return true;
}