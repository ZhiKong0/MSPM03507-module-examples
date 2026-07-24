#include "rotary_menu.h"

#include <string.h>

#ifndef ROTARY_MENU_BUTTON_ACTIVE_LOW
#define ROTARY_MENU_BUTTON_ACTIVE_LOW 1
#endif

#ifndef ROTARY_MENU_DEBOUNCE_MS
#define ROTARY_MENU_DEBOUNCE_MS 20u
#endif

#ifndef ROTARY_MENU_LONG_PRESS_MS
#define ROTARY_MENU_LONG_PRESS_MS 800u
#endif

#ifndef ROTARY_MENU_ENCODER_REVERSED
#define ROTARY_MENU_ENCODER_REVERSED 0
#endif

typedef struct {
    const char *label;
    rotary_menu_action_t action;
} rotary_menu_item_t;

static const rotary_menu_item_t g_menu_items[] = {
    { "ARM / DISARM", ROTARY_MENU_ACTION_ARM_TOGGLE },
    { "STOP", ROTARY_MENU_ACTION_STOP },
    { "SELF TEST", ROTARY_MENU_ACTION_SELF_TEST },
    { "H1 A-B", ROTARY_MENU_ACTION_H_BASIC_AB },
    { "H2 A-B-C-D-A", ROTARY_MENU_ACTION_H_BASIC_LOOP },
    { "H3 FIG8", ROTARY_MENU_ACTION_H_FIGURE8 },
    { "H4 FIG8 X4", ROTARY_MENU_ACTION_H_FIGURE8_4LAP },
    { "SENSOR CHECK", ROTARY_MENU_ACTION_SENSOR_CHECK },
    { "MOTOR CHECK", ROTARY_MENU_ACTION_MOTOR_CHECK },
};

static uint8_t array_count(void)
{
    return (uint8_t)(sizeof(g_menu_items) / sizeof(g_menu_items[0]));
}

static void move_selection(rotary_menu_t *menu, int8_t delta)
{
    uint8_t count;

    if ((menu == 0) || (delta == 0)) {
        return;
    }

    count = menu->item_count;
    if (count == 0u) {
        return;
    }

    if (delta > 0) {
        menu->selected = (uint8_t)((menu->selected + 1u) % count);
    } else {
        menu->selected = (menu->selected == 0u) ? (uint8_t)(count - 1u)
                                                : (uint8_t)(menu->selected - 1u);
    }
    menu->detent_count += delta;
    menu->last_delta = delta;
    menu->redraw_requested = 1u;
}

void RotaryMenu_Init(rotary_menu_t *menu)
{
    if (menu == 0) {
        return;
    }

    memset(menu, 0, sizeof(*menu));
    menu->item_count = array_count();
    menu->redraw_requested = 1u;
}

void RotaryMenu_Update(rotary_menu_t *menu,
                       uint8_t phase_a_high,
                       uint8_t phase_b_high,
                       uint8_t button_high)
{
    static const int8_t decode_table[16] = {
        0, -1, 1, 0,
        1, 0, 0, -1,
        -1, 0, 0, 1,
        0, 1, -1, 0,
    };
    uint8_t ab;
    uint8_t transition;
    int8_t step;
    uint8_t raw_pressed;

    if (menu == 0) {
        return;
    }

    ab = (uint8_t)(((phase_a_high != 0u) ? 2u : 0u) |
                   ((phase_b_high != 0u) ? 1u : 0u));
    transition = (uint8_t)((menu->last_ab << 2u) | ab);
    step = decode_table[transition];
    menu->last_ab = ab;

#if ROTARY_MENU_ENCODER_REVERSED
    step = (int8_t)(-step);
#endif

    if (step != 0) {
        menu->quarter_steps = (int8_t)(menu->quarter_steps + step);
        if (menu->quarter_steps >= 4) {
            menu->quarter_steps = 0;
            move_selection(menu, 1);
        } else if (menu->quarter_steps <= -4) {
            menu->quarter_steps = 0;
            move_selection(menu, -1);
        }
    }

#if ROTARY_MENU_BUTTON_ACTIVE_LOW
    raw_pressed = (button_high == 0u) ? 1u : 0u;
#else
    raw_pressed = (button_high != 0u) ? 1u : 0u;
#endif

    if (raw_pressed != menu->last_raw_pressed) {
        menu->last_raw_pressed = raw_pressed;
        menu->debounce_ms = 0u;
    } else if (menu->debounce_ms < ROTARY_MENU_DEBOUNCE_MS) {
        menu->debounce_ms++;
    }

    if ((menu->debounce_ms >= ROTARY_MENU_DEBOUNCE_MS) &&
        (raw_pressed != menu->stable_pressed)) {
        menu->stable_pressed = raw_pressed;
        menu->button_down = raw_pressed;
        menu->redraw_requested = 1u;

        if (raw_pressed != 0u) {
            menu->press_ms = 0u;
            menu->long_press_sent = 0u;
        } else if (menu->long_press_sent == 0u) {
            menu->pending_action = RotaryMenu_Action(menu->selected);
            menu->action_counter++;
        }
    }

    if (menu->stable_pressed != 0u) {
        if (menu->press_ms < UINT16_MAX) {
            menu->press_ms++;
        }
        if ((menu->press_ms >= ROTARY_MENU_LONG_PRESS_MS) &&
            (menu->long_press_sent == 0u)) {
            menu->pending_action = ROTARY_MENU_ACTION_STOP;
            menu->action_counter++;
            menu->long_press_sent = 1u;
            menu->redraw_requested = 1u;
        }
    }
}

rotary_menu_action_t RotaryMenu_ConsumeAction(rotary_menu_t *menu)
{
    rotary_menu_action_t action;

    if (menu == 0) {
        return ROTARY_MENU_ACTION_NONE;
    }

    action = menu->pending_action;
    menu->pending_action = ROTARY_MENU_ACTION_NONE;
    return action;
}

const char *RotaryMenu_Label(uint8_t index)
{
    if (index >= array_count()) {
        return "";
    }
    return g_menu_items[index].label;
}

rotary_menu_action_t RotaryMenu_Action(uint8_t index)
{
    if (index >= array_count()) {
        return ROTARY_MENU_ACTION_NONE;
    }
    return g_menu_items[index].action;
}

uint8_t RotaryMenu_Count(void)
{
    return array_count();
}
