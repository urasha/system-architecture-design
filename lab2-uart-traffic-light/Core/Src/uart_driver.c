#include "uart_driver.h"

#include "custom_gpio.h"
#include "stm32f4xx_hal.h"

#define RX_MASK (UART_RX_BUFFER_SIZE - 1U)
#define TX_MASK (UART_TX_BUFFER_SIZE - 1U)
#define RX_ERRORS (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)

_Static_assert((UART_RX_BUFFER_SIZE & RX_MASK) == 0U, "RX size must be a power of two");
_Static_assert((UART_TX_BUFFER_SIZE & TX_MASK) == 0U, "TX size must be a power of two");

static uint8_t rx_data[UART_RX_BUFFER_SIZE];
static uint8_t tx_data[UART_TX_BUFFER_SIZE];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;
static volatile uint32_t tx_head;
static volatile uint32_t tx_tail;
static volatile bool rx_error;
static volatile bool use_interrupts;

static uint32_t lock(void)
{
    const uint32_t previous = __get_PRIMASK();
    __disable_irq();
    return previous;
}

static void unlock(uint32_t previous)
{
    __set_PRIMASK(previous);
}

/* Called only from the USART ISR or a short critical section in main. */
static void service_hardware(void)
{
    const uint32_t status = USART6->SR;
    if ((status & (USART_SR_RXNE | RX_ERRORS)) != 0U) {
        /* RM0090: SR read followed by DR read clears RXNE and RX errors. */
        const uint8_t byte = (uint8_t)USART6->DR;
        if ((status & RX_ERRORS) != 0U) {
            rx_error = true;
        }
        if ((status & USART_SR_RXNE) != 0U
            && (status & (USART_SR_NE | USART_SR_FE | USART_SR_PE)) == 0U) {
            const uint32_t next = (rx_head + 1U) & RX_MASK;
            if (next != rx_tail) {
                rx_data[rx_head] = byte;
                __DMB();
                rx_head = next;
            } else {
                rx_error = true;
            }
        }
    }

    if ((status & USART_SR_TXE) != 0U) {
        if (tx_tail != tx_head) {
            USART6->DR = tx_data[tx_tail];
            tx_tail = (tx_tail + 1U) & TX_MASK;
        }
        if (tx_tail == tx_head) {
            USART6->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

bool uart_init(void)
{
    NVIC_DisableIRQ(USART6_IRQn);
    RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    (void)RCC->APB2ENR;
    RCC->APB2RSTR |= RCC_APB2RSTR_USART6RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART6RST;

    rx_head = rx_tail = tx_head = tx_tail = 0U;
    rx_error = false;
    use_interrupts = false;

    /* USART6 is on APB2, oversampling by 16, 8 data bits, no parity, 1 stop bit. */
    USART6->CR1 = 0U;
    USART6->CR2 = 0U;
    USART6->CR3 = 0U;
    USART6->BRR = (HAL_RCC_GetPCLK2Freq() + UART_BAUD_RATE / 2U) / UART_BAUD_RATE;
    USART6->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    const CustomGpioConfig tx_config = {
        .mode = CUSTOM_GPIO_MODE_ALTERNATE,
        .output_type = CUSTOM_GPIO_PUSH_PULL,
        .speed = CUSTOM_GPIO_SPEED_LOW,
        .pull = CUSTOM_GPIO_NO_PULL,
        .alternate_function = 8U,
        .output_high = true
    };
    CustomGpioConfig rx_config = tx_config;
    rx_config.pull = CUSTOM_GPIO_PULL_UP;
    if (!custom_gpio_init(GPIOC, (uint16_t)(1U << 6U), &tx_config)
        || !custom_gpio_init(GPIOC, (uint16_t)(1U << 7U), &rx_config)) {
        USART6->CR1 = 0U;
        return false;
    }

    NVIC_SetPriority(USART6_IRQn, 5U);
    NVIC_ClearPendingIRQ(USART6_IRQn);
    uart_set_interrupts(true);
    return true;
}

void uart_poll(void)
{
    if (!use_interrupts) {
        const uint32_t previous = lock();
        service_hardware();
        unlock(previous);
    }
}

bool uart_read_byte(uint8_t *byte)
{
    if (byte == NULL) {
        return false;
    }
    const uint32_t previous = lock();
    const bool available = rx_tail != rx_head;
    if (available) {
        *byte = rx_data[rx_tail];
        rx_tail = (rx_tail + 1U) & RX_MASK;
    }
    unlock(previous);
    return available;
}

size_t uart_tx_free(void)
{
    /* Only main produces TX data. ISR can only increase the free space. */
    return (size_t)((tx_tail - tx_head - 1U) & TX_MASK);
}

bool uart_write(const char *data, size_t length)
{
    if (data == NULL || length > uart_tx_free()) {
        return false;
    }
    uint32_t head = tx_head;
    for (size_t i = 0U; i < length; ++i) {
        tx_data[head] = (uint8_t)data[i];
        head = (head + 1U) & TX_MASK;
    }
    /* Publish only after copying. Keep interrupts enabled during the copy. */
    const uint32_t previous = lock();
    __DMB();
    tx_head = head;
    if (use_interrupts && tx_head != tx_tail) {
        USART6->CR1 |= USART_CR1_TXEIE;
    }
    unlock(previous);
    return true;
}

void uart_set_interrupts(bool enabled)
{
    const uint32_t previous = lock();
    NVIC_DisableIRQ(USART6_IRQn);
    USART6->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE | USART_CR1_PEIE | USART_CR1_TCIE);
    USART6->CR3 &= ~USART_CR3_EIE;
    /* Preserve both rings and consume a possible pending RX byte before switching. */
    service_hardware();
    use_interrupts = enabled;
    NVIC_ClearPendingIRQ(USART6_IRQn);
    if (enabled) {
        USART6->CR3 |= USART_CR3_EIE;
        USART6->CR1 |= USART_CR1_RXNEIE;
        if (tx_tail != tx_head) {
            USART6->CR1 |= USART_CR1_TXEIE;
        }
        NVIC_EnableIRQ(USART6_IRQn);
    }
    unlock(previous);
}

bool uart_interrupts_enabled(void)
{
    return use_interrupts;
}

bool uart_take_rx_error(void)
{
    const uint32_t previous = lock();
    const bool error = rx_error;
    rx_error = false;
    unlock(previous);
    return error;
}

void USART6_IRQHandler(void)
{
    if (use_interrupts) {
        service_hardware();
    }
}
