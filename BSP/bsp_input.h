#ifndef BSP_INPUT_H
#define BSP_INPUT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 主循环周期是 5ms */
#define BSP_INPUT_SCAN_PERIOD_MS        5U

/* 连续多少次采样一致，认为稳定 */
#define BSP_KEY_DEBOUNCE_COUNT          4U      /* 3 * 5ms = 15ms */

/* 长按门限 */
#define BSP_KEY_LONG_PRESS_COUNT        120U    /* 100 * 5ms = 500ms */

typedef enum
{
    BSP_KEY_LEFT = 0,
    BSP_KEY_DOWN,
    BSP_KEY_RIGHT,
    BSP_KEY_UP,
    BSP_KEY_MID,
    BSP_KEY_MAX
} BSP_Key_t;

#define BSP_KEY_MASK_LEFT     (1U << BSP_KEY_LEFT)
#define BSP_KEY_MASK_DOWN     (1U << BSP_KEY_DOWN)
#define BSP_KEY_MASK_RIGHT    (1U << BSP_KEY_RIGHT)
#define BSP_KEY_MASK_UP       (1U << BSP_KEY_UP)
#define BSP_KEY_MASK_MID      (1U << BSP_KEY_MID)

typedef struct
{
    uint8_t key_short_mask;   /* 稳定按下边沿触发一次 */
    uint8_t key_long_mask;    /* 长按触发一次 */
    int8_t  enc_delta;        /* >0 顺时针，<0 逆时针 */
} BSP_InputEvent_t;

void BSP_Input_Init(void);
void BSP_Input_Scan(void);
bool BSP_Input_FetchEvents(BSP_InputEvent_t *event);

#ifdef __cplusplus
}
#endif

#endif /* BSP_INPUT_H */