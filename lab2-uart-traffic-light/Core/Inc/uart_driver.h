#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    UART_BAUD_RATE = 57600U,
    UART_RX_BUFFER_SIZE = 512U,
    UART_TX_BUFFER_SIZE = 2048U
};

bool uart_init(void);
/* Polling mode: one bounded RX/TX service pass. Interrupt mode: no-op. */
void uart_poll(void);
bool uart_read_byte(uint8_t *byte);
/* Enqueue the entire message or return false; never wait for transmission. */
bool uart_write(const char *data, size_t length);
size_t uart_tx_free(void);
void uart_set_interrupts(bool enabled);
bool uart_interrupts_enabled(void);
/* Report and clear a hardware RX error or a software RX buffer overflow. */
bool uart_take_rx_error(void);
void USART6_IRQHandler(void);

#endif
