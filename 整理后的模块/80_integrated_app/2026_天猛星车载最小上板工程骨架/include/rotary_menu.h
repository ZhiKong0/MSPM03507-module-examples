#ifndef ROTARY_MENU_H
#define ROTARY_MENU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ROTARY_MENU_ACTION_NONE = 0,
    ROTARY_MENU_ACTION_ARM_TOGGLE,
    ROTARY_MENU_ACTION_STOP,
    ROTARY_MENU_ACTION_SELF_TEST,
    ROTARY_MENU_ACTION_H_BASIC_AB,
    ROTARY_MENU_ACTION_H_BASIC_LOOP,
    ROTARY_MENU_ACTION_H_FIGURE8,
    ROTARY_MENU_ACTION_H_FIGURE8_4LAP,
    ROTARY_MENU_ACTION_SENSOR_CHECK,
    ROTARY_MENU_ACTION_MOTOR_CHECK
} rotary_menu_action_t;

typedef struct {
    uint8_t selected;
    uint8_t item_count;
    uint8_t button_down;
    uint8_t redraw_requested;
    uint8_t last_ab;
    int8_t quarter_steps;
    int8_t last_delta;
    int32_t detent_count;
    uint16_t debounce_ms;
    uint16_t press_ms;
    uint8_t last_raw_pressed;
    uint8_t stable_pressed;
    uint8_t long_press_sent;
    rotary_menu_action_t pending_action;
    uint32_t action_counter;
} rotary_menu_t;

void RotaryMenu_Init(rotary_menu_t *menu);
void RotaryMenu_Update(rotary_menu_t *menu,
                       uint8_t phase_a_high,
                       uint8_t phase_b_high,
                       uint8_t button_high);
rotary_menu_action_t RotaryMenu_ConsumeAction(rotary_menu_t *menu);
const char *RotaryMenu_Label(uint8_t index);
rotary_menu_action_t RotaryMenu_Action(uint8_t index);
uint8_t RotaryMenu_Count(void);

#ifdef __cplusplus
}
#endif

#endif
