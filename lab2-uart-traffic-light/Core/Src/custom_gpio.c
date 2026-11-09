#include "custom_gpio.h"

static bool is_valid_pin(uint8_t pin)
{
    return pin < 16U;
}

static uint32_t field_mask_for_pins(uint16_t pin_mask)
{
    uint32_t field_mask = 0U;

    for (uint32_t pin = 0U; pin < 16U; ++pin) {
        if ((pin_mask & (uint16_t)(1U << pin)) != 0U) {
            field_mask |= 3UL << (pin * 2U);
        }
    }
    return field_mask;
}

static uint32_t field_value_for_pins(uint16_t pin_mask, uint32_t value)
{
    uint32_t field_value = 0U;

    for (uint32_t pin = 0U; pin < 16U; ++pin) {
        if ((pin_mask & (uint16_t)(1U << pin)) != 0U) {
            field_value |= value << (pin * 2U);
        }
    }
    return field_value;
}

bool custom_gpio_enable_clock(GPIO_TypeDef *port)
{
    uint32_t enable_mask;

    if (port == GPIOA) {
        enable_mask = RCC_AHB1ENR_GPIOAEN;
    } else if (port == GPIOB) {
        enable_mask = RCC_AHB1ENR_GPIOBEN;
    } else if (port == GPIOC) {
        enable_mask = RCC_AHB1ENR_GPIOCEN;
    } else if (port == GPIOD) {
        enable_mask = RCC_AHB1ENR_GPIODEN;
    } else if (port == GPIOE) {
        enable_mask = RCC_AHB1ENR_GPIOEEN;
    } else if (port == GPIOF) {
        enable_mask = RCC_AHB1ENR_GPIOFEN;
    } else if (port == GPIOG) {
        enable_mask = RCC_AHB1ENR_GPIOGEN;
    } else if (port == GPIOH) {
        enable_mask = RCC_AHB1ENR_GPIOHEN;
    } else if (port == GPIOI) {
        enable_mask = RCC_AHB1ENR_GPIOIEN;
    } else {
        return false;
    }

    RCC->AHB1ENR |= enable_mask;
    (void)RCC->AHB1ENR;
    return true;
}

bool custom_gpio_init(GPIO_TypeDef *port, uint16_t pin_mask, const CustomGpioConfig *config)
{
    if ((port == NULL) || (config == NULL) || (pin_mask == 0U)
        || ((uint32_t)config->mode > (uint32_t)CUSTOM_GPIO_MODE_ANALOG)
        || ((uint32_t)config->output_type > (uint32_t)CUSTOM_GPIO_OPEN_DRAIN)
        || ((uint32_t)config->speed > (uint32_t)CUSTOM_GPIO_SPEED_VERY_HIGH)
        || ((uint32_t)config->pull > (uint32_t)CUSTOM_GPIO_PULL_DOWN)
        || (config->alternate_function > 15U)) {
        return false;
    }

    if (!custom_gpio_enable_clock(port)) {
        return false;
    }

    if ((config->mode == CUSTOM_GPIO_MODE_OUTPUT)
        || (config->mode == CUSTOM_GPIO_MODE_ALTERNATE)) {
        custom_gpio_write_mask(port, pin_mask, config->output_high ? pin_mask : 0U);
    }

    const uint32_t two_bit_mask = field_mask_for_pins(pin_mask);

    port->OTYPER = (port->OTYPER & ~(uint32_t)pin_mask)
        | (config->output_type == CUSTOM_GPIO_OPEN_DRAIN ? pin_mask : 0U);
    port->OSPEEDR = (port->OSPEEDR & ~two_bit_mask)
        | field_value_for_pins(pin_mask, (uint32_t)config->speed);
    port->PUPDR = (port->PUPDR & ~two_bit_mask)
        | field_value_for_pins(pin_mask, (uint32_t)config->pull);

    for (uint32_t pin = 0U; pin < 16U; ++pin) {
        if ((pin_mask & (uint16_t)(1U << pin)) != 0U) {
            const uint32_t index = pin / 8U;
            const uint32_t shift = (pin % 8U) * 4U;
            port->AFR[index] = (port->AFR[index] & ~(0xFUL << shift))
                | ((uint32_t)config->alternate_function << shift);
        }
    }

    port->MODER = (port->MODER & ~two_bit_mask)
        | field_value_for_pins(pin_mask, (uint32_t)config->mode);
    return true;
}

bool custom_gpio_get_config(const GPIO_TypeDef *port, uint8_t pin, CustomGpioConfig *config)
{
    if ((port == NULL) || (config == NULL) || !is_valid_pin(pin)) {
        return false;
    }

    const uint32_t shift = (uint32_t)pin * 2U;
    const uint32_t af_shift = ((uint32_t)pin % 8U) * 4U;

    config->mode = (CustomGpioMode)((port->MODER >> shift) & 3U);
    config->output_type = (CustomGpioOutputType)((port->OTYPER >> pin) & 1U);
    config->speed = (CustomGpioSpeed)((port->OSPEEDR >> shift) & 3U);
    config->pull = (CustomGpioPull)((port->PUPDR >> shift) & 3U);
    config->alternate_function = (uint8_t)((port->AFR[pin / 8U] >> af_shift) & 0xFU);
    config->output_high = ((port->ODR >> pin) & 1U) != 0U;
    return true;
}

bool custom_gpio_read_pin(const GPIO_TypeDef *port, uint8_t pin)
{
    return (port != NULL) && is_valid_pin(pin) && (((port->IDR >> pin) & 1U) != 0U);
}

uint16_t custom_gpio_read_mask(const GPIO_TypeDef *port, uint16_t pin_mask)
{
    return port == NULL ? 0U : (uint16_t)(port->IDR & pin_mask);
}

void custom_gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, bool high)
{
    if ((port != NULL) && is_valid_pin(pin)) {
        const uint16_t pin_mask = (uint16_t)(1U << pin);
        custom_gpio_write_mask(port, pin_mask, high ? pin_mask : 0U);
    }
}

void custom_gpio_write_mask(GPIO_TypeDef *port, uint16_t pin_mask, uint16_t high_mask)
{
    if (port == NULL) {
        return;
    }

    const uint32_t set_mask = (uint32_t)(pin_mask & high_mask);
    const uint32_t reset_mask = (uint32_t)(pin_mask & (uint16_t)~high_mask) << 16U;
    port->BSRR = set_mask | reset_mask;
}
