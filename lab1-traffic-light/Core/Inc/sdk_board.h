#ifndef SDK_BOARD_H
#define SDK_BOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SDK_LED_NONE = 0U,
    SDK_LED_GREEN = 1U << 0,
    SDK_LED_YELLOW = 1U << 1,
    SDK_LED_RED = 1U << 2
} SdkLedMask;

void sdk_board_init(void);
bool sdk_board_button_pressed(void);
void sdk_board_write_leds(uint8_t led_mask);

#endif
