#ifndef CUSTOM_GPIO_H
#define CUSTOM_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx.h"

typedef enum {
    CUSTOM_GPIO_MODE_INPUT = 0U,
    CUSTOM_GPIO_MODE_OUTPUT = 1U,
    CUSTOM_GPIO_MODE_ALTERNATE = 2U,
    CUSTOM_GPIO_MODE_ANALOG = 3U
} CustomGpioMode;

typedef enum {
    CUSTOM_GPIO_PUSH_PULL = 0U,
    CUSTOM_GPIO_OPEN_DRAIN = 1U
} CustomGpioOutputType;

typedef enum {
    CUSTOM_GPIO_SPEED_LOW = 0U,
    CUSTOM_GPIO_SPEED_MEDIUM = 1U,
    CUSTOM_GPIO_SPEED_HIGH = 2U,
    CUSTOM_GPIO_SPEED_VERY_HIGH = 3U
} CustomGpioSpeed;

typedef enum {
    CUSTOM_GPIO_NO_PULL = 0U,
    CUSTOM_GPIO_PULL_UP = 1U,
    CUSTOM_GPIO_PULL_DOWN = 2U
} CustomGpioPull;

typedef struct {
    CustomGpioMode mode;
    CustomGpioOutputType output_type;
    CustomGpioSpeed speed;
    CustomGpioPull pull;
    uint8_t alternate_function;
    bool output_high;
} CustomGpioConfig;

bool custom_gpio_enable_clock(GPIO_TypeDef *port);
bool custom_gpio_init(GPIO_TypeDef *port, uint16_t pin_mask, const CustomGpioConfig *config);
bool custom_gpio_get_config(const GPIO_TypeDef *port, uint8_t pin, CustomGpioConfig *config);

bool custom_gpio_read_pin(const GPIO_TypeDef *port, uint8_t pin);
uint16_t custom_gpio_read_mask(const GPIO_TypeDef *port, uint16_t pin_mask);
void custom_gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, bool high);
void custom_gpio_write_mask(GPIO_TypeDef *port, uint16_t pin_mask, uint16_t high_mask);

#endif
