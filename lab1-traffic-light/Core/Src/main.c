#include "main.h"

#include "button.h"
#include "sdk_board.h"
#include "traffic_light.h"

static void SystemClock_Config(void);

static void set_traffic_lights(bool red, bool yellow, bool green)
{
    uint8_t mask = SDK_LED_NONE;
    mask |= red ? SDK_LED_RED : 0U;
    mask |= yellow ? SDK_LED_YELLOW : 0U;
    mask |= green ? SDK_LED_GREEN : 0U;
    sdk_board_write_leds(mask);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    sdk_board_init();

    const uint32_t now_ms = HAL_GetTick();
    Button button;
    TrafficLight traffic_light;
    button_init(&button, sdk_board_button_pressed(), now_ms, 30U);
    traffic_light_init(&traffic_light, now_ms, set_traffic_lights);

    while (1) {
        const uint32_t current_ms = HAL_GetTick();
        const ButtonEvent event = button_update(
            &button,
            sdk_board_button_pressed(),
            current_ms);

        if (event.type == BUTTON_EVENT_PRESSED) {
            traffic_light_on_button_pressed(&traffic_light, current_ms);
        }
        traffic_light_update(&traffic_light, current_ms);
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscillator.HSEState = RCC_HSE_ON;
    oscillator.PLL.PLLState = RCC_PLL_ON;
    oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscillator.PLL.PLLM = 25U;
    oscillator.PLL.PLLN = 336U;
    oscillator.PLL.PLLP = RCC_PLLP_DIV2;
    oscillator.PLL.PLLQ = 7U;
    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
        Error_Handler();
    }

    clocks.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV4;
    clocks.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    sdk_board_write_leds(SDK_LED_RED);
    while (1) {
    }
}
