#include "button.h"

static bool elapsed(uint32_t now_ms, uint32_t since_ms, uint32_t interval_ms)
{
    return (uint32_t)(now_ms - since_ms) >= interval_ms;
}

void button_init(Button *button, bool initially_pressed, uint32_t now_ms, uint32_t debounce_ms)
{
    button->candidate_pressed = initially_pressed;
    button->stable_pressed = initially_pressed;
    button->candidate_since_ms = now_ms;
    button->pressed_since_ms = now_ms;
    button->debounce_ms = debounce_ms;
}

ButtonEvent button_update(Button *button, bool raw_pressed, uint32_t now_ms)
{
    ButtonEvent event = { .type = BUTTON_EVENT_NONE, .held_ms = 0U };

    if (raw_pressed != button->candidate_pressed) {
        button->candidate_pressed = raw_pressed;
        button->candidate_since_ms = now_ms;
        return event;
    }

    if ((button->stable_pressed == button->candidate_pressed)
        || !elapsed(now_ms, button->candidate_since_ms, button->debounce_ms)) {
        return event;
    }

    button->stable_pressed = button->candidate_pressed;
    if (button->stable_pressed) {
        button->pressed_since_ms = now_ms;
        event.type = BUTTON_EVENT_PRESSED;
    } else {
        event.type = BUTTON_EVENT_RELEASED;
        event.held_ms = (uint32_t)(now_ms - button->pressed_since_ms);
    }
    return event;
}
