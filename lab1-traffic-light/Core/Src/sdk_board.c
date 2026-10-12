#include "sdk_board.h"

#include "custom_gpio.h"

enum {
    BUTTON_PIN = 15U,
    GREEN_LED_PIN = 13U,
    YELLOW_LED_PIN = 14U,
    RED_LED_PIN = 15U
};

static const uint16_t GREEN_LED_MASK = (uint16_t)(1U << GREEN_LED_PIN);
static const uint16_t YELLOW_LED_MASK = (uint16_t)(1U << YELLOW_LED_PIN);
static const uint16_t RED_LED_MASK = (uint16_t)(1U << RED_LED_PIN);
static const uint16_t ALL_LED_MASK = (uint16_t)(
    (1U << GREEN_LED_PIN) | (1U << YELLOW_LED_PIN) | (1U << RED_LED_PIN));

void sdk_board_init(void)
{
    const CustomGpioConfig led_config = {
        .mode = CUSTOM_GPIO_MODE_OUTPUT,
        .output_type = CUSTOM_GPIO_PUSH_PULL,
        .speed = CUSTOM_GPIO_SPEED_LOW,
        .pull = CUSTOM_GPIO_NO_PULL,
        .alternate_function = 0U,
        .output_high = false
    };
    const CustomGpioConfig button_config = {
        .mode = CUSTOM_GPIO_MODE_INPUT,
        .output_type = CUSTOM_GPIO_PUSH_PULL,
        .speed = CUSTOM_GPIO_SPEED_LOW,
        .pull = CUSTOM_GPIO_NO_PULL,
        .alternate_function = 0U,
        .output_high = false
    };

    (void)custom_gpio_init(GPIOD, ALL_LED_MASK, &led_config);
    (void)custom_gpio_init(GPIOC, (uint16_t)(1U << BUTTON_PIN), &button_config);
}

bool sdk_board_button_pressed(void)
{
    return !custom_gpio_read_pin(GPIOC, BUTTON_PIN);
}

void sdk_board_write_leds(uint8_t led_mask)
{
    uint16_t high_mask = 0U;

    if ((led_mask & SDK_LED_GREEN) != 0U) {
        high_mask |= GREEN_LED_MASK;
    }

    const bool red = (led_mask & SDK_LED_RED) != 0U;
    const bool yellow = (led_mask & SDK_LED_YELLOW) != 0U;
    if (red && !yellow) {
        high_mask |= RED_LED_MASK;
    } else if (yellow && !red) {
        high_mask |= YELLOW_LED_MASK;
    }

    custom_gpio_write_mask(GPIOD, ALL_LED_MASK, high_mask);
}
