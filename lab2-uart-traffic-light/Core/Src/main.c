#include "main.h"

/* USER CODE BEGIN Includes */
#include "button.h"
#include "console.h"
#include "sdk_board.h"
#include "traffic_light.h"
#include "uart_driver.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
static Button button;
static TrafficLight traffic_light;
static Console console;
/* USER CODE END PV */

void SystemClock_Config(void);

/* USER CODE BEGIN 0 */
static void set_traffic_lights(bool red, bool yellow, bool green)
{
    uint8_t mask = SDK_LED_NONE;
    mask |= red ? SDK_LED_RED : 0U;
    mask |= yellow ? SDK_LED_YELLOW : 0U;
    mask |= green ? SDK_LED_GREEN : 0U;
    sdk_board_write_leds(mask);
}
/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* USER CODE BEGIN 2 */
    sdk_board_init();
    const uint32_t now_ms = HAL_GetTick();
    button_init(&button, sdk_board_button_pressed(), now_ms, 30U);
    traffic_light_init(&traffic_light, now_ms, set_traffic_lights);
    if (!uart_init()) {
        Error_Handler();
    }
    console_init(&console);
    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
        uart_poll();
        const uint32_t current_ms = HAL_GetTick();
        traffic_light_update(&traffic_light, current_ms);
        const ButtonEvent event = button_update(&button, sdk_board_button_pressed(), current_ms);
        if (event.type == BUTTON_EVENT_PRESSED) {
            traffic_light_on_button_pressed(&traffic_light, current_ms);
        }
        console_update(&console, &traffic_light, current_ms);
    }
    /* USER CODE END 3 */
}

/* Same HSI 16 MHz configuration as the existing light project. No LSE on PC15. */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    oscillator.HSIState = RCC_HSI_ON;
    oscillator.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    oscillator.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
        Error_Handler();
    }
    clocks.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV1;
    clocks.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}
