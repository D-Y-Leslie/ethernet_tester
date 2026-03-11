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
    uint8_t stable_level;     /* 1=松开, 0=按下 */
    uint8_t last_raw_level;
    uint8_t debounce_cnt;
    uint16_t press_cnt;
    uint8_t long_sent;
} BSP_KeyState_t;

static BSP_KeyState_t g_keyState[BSP_KEY_MAX];
static BSP_InputEvent_t g_inputEvent;

static uint8_t g_encPrevAB = 0;
static int8_t  g_encAcc = 0;

static uint8_t BSP_Input_ReadKeyRaw(BSP_Key_t key)
{
    uint32_t pin = 0U;

    switch (key) {
    case BSP_KEY_LEFT:  pin = KEY_LEFT_PIN;  break;
    case BSP_KEY_DOWN:  pin = KEY_DOWN_PIN;  break;
    case BSP_KEY_RIGHT: pin = KEY_RIGHT_PIN; break;
    case BSP_KEY_UP:    pin = KEY_UP_PIN;    break;
    case BSP_KEY_MID:   pin = KEY_MID_PIN;   break;
    default:            pin = 0U;            break;
    }

    return (DL_GPIO_readPins(KEY_PORT, pin) != 0U) ? 1U : 0U;
}

static uint8_t BSP_Input_ReadEncAB(void)
{
    uint8_t a = (DL_GPIO_readPins(ENC_PORT, ENC_A_PIN) != 0U) ? 1U : 0U;
    uint8_t b = (DL_GPIO_readPins(ENC_PORT, ENC_B_PIN) != 0U) ? 1U : 0U;
    return (uint8_t)((a << 1) | b);
}

void BSP_Input_Init(void)
{
    uint32_t i;

    for (i = 0; i < BSP_KEY_MAX; i++) {
        uint8_t raw = BSP_Input_ReadKeyRaw((BSP_Key_t)i);
        g_keyState[i].stable_level   = raw;
        g_keyState[i].last_raw_level = raw;
        g_keyState[i].debounce_cnt   = 0U;
        g_keyState[i].press_cnt      = 0U;
        g_keyState[i].long_sent      = 0U;
    }

    g_inputEvent.key_short_mask = 0U;
    g_inputEvent.key_long_mask  = 0U;
    g_inputEvent.enc_delta      = 0;

    g_encPrevAB = BSP_Input_ReadEncAB();
    g_encAcc = 0;
}

static void BSP_Input_ScanKeys(void)
{
    uint32_t i;

    for (i = 0; i < BSP_KEY_MAX; i++) {
        uint8_t raw = BSP_Input_ReadKeyRaw((BSP_Key_t)i);
        BSP_KeyState_t *ks = &g_keyState[i];

        if (raw != ks->last_raw_level) {
            ks->last_raw_level = raw;
            ks->debounce_cnt = 0U;
        } else {
            if (ks->debounce_cnt < BSP_KEY_DEBOUNCE_COUNT) {
                ks->debounce_cnt++;
            }
        }

        if (ks->debounce_cnt >= BSP_KEY_DEBOUNCE_COUNT) {
            if (ks->stable_level != raw) {
                /* 出现稳定边沿 */
                ks->stable_level = raw;

                if (raw == 0U) {
                    /* 稳定按下：立刻触发 short */
                    g_inputEvent.key_short_mask |= (1U << i);
                    ks->press_cnt = 0U;
                    ks->long_sent = 0U;
                } else {
                    /* 稳定释放：只复位计数 */
                    ks->press_cnt = 0U;
                    ks->long_sent = 0U;
                }
            } else {
                /* 稳定保持 */
                if (ks->stable_level == 0U) {
                    if (ks->press_cnt < 0xFFFFU) {
                        ks->press_cnt++;
                    }

                    if ((ks->press_cnt >= BSP_KEY_LONG_PRESS_COUNT) &&
                        (ks->long_sent == 0U)) {
                        g_inputEvent.key_long_mask |= (1U << i);
                        ks->long_sent = 1U;
                    }
                }
            }
        }
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

    uint8_t currAB = BSP_Input_ReadEncAB();
    uint8_t idx = (uint8_t)((g_encPrevAB << 2) | currAB);
    int8_t step = transTable[idx];

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

    g_inputEvent.key_short_mask = 0U;
    g_inputEvent.key_long_mask  = 0U;
    g_inputEvent.enc_delta      = 0;

    return true;
}