#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} ButtonEventType;

typedef struct {
    ButtonEventType type;
    uint32_t held_ms;
} ButtonEvent;

typedef struct {
    bool candidate_pressed;
    bool stable_pressed;
    uint32_t candidate_since_ms;
    uint32_t pressed_since_ms;
    uint32_t debounce_ms;
} Button;

void button_init(Button *button, bool initially_pressed, uint32_t now_ms, uint32_t debounce_ms);
ButtonEvent button_update(Button *button, bool raw_pressed, uint32_t now_ms);

#endif
