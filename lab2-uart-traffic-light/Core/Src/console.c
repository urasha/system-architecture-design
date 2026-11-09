#include "console.h"

#include <stdio.h>
#include <string.h>

#include "uart_driver.h"

/* Enough space for echo, an error and the longest status response in one call. */
enum { CONSOLE_TX_RESERVE = 256U };

static void reply(const char *message)
{
    (void)uart_write("\r\n", 2U);
    (void)uart_write(message, strlen(message));
    (void)uart_write("\r\n", 2U);
}

static bool parse_seconds(const char *text, uint32_t *seconds)
{
    uint32_t value = 0U;
    if (*text == '\0') {
        return false;
    }
    while (*text != '\0') {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const uint32_t digit = (uint32_t)(*text - '0');
        if (value > (TRAFFIC_MAX_TIMEOUT_SECONDS - digit) / 10U) {
            return false;
        }
        value = value * 10U + digit;
        ++text;
    }
    if (value == 0U) {
        return false;
    }
    *seconds = value;
    return true;
}

static void execute(char *line, TrafficLight *light, uint32_t now_ms)
{
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    size_t length = strlen(line);
    while (length > 0U && (line[length - 1U] == ' ' || line[length - 1U] == '\t')) {
        line[--length] = '\0';
    }
    if (length == 0U) {
        return;
    }

    if (strcmp(line, "?") == 0) {
        char status[128];
        (void)snprintf(status, sizeof status, "%s; mode %u; timeout %lu; %c",
            traffic_light_state_name(light), light->button_enabled ? 1U : 2U,
            (unsigned long)(light->red_ms / 1000U), uart_interrupts_enabled() ? 'I' : 'P');
        reply(status);
    } else if (strcmp(line, "set mode 1") == 0) {
        light->button_enabled = true;
        reply("OK");
    } else if (strcmp(line, "set mode 2") == 0) {
        light->button_enabled = false;
        reply("OK");
    } else if (strcmp(line, "set interrupts on") == 0) {
        uart_set_interrupts(true);
        reply("OK");
    } else if (strcmp(line, "set interrupts off") == 0) {
        uart_set_interrupts(false);
        reply("OK");
    } else if (strncmp(line, "set timeout ", 12U) == 0) {
        uint32_t seconds;
        if (parse_seconds(line + 12U, &seconds)
            && traffic_light_set_timeout(light, seconds, now_ms)) {
            reply("OK");
        } else {
            reply("ERROR: timeout must be an integer from 1 to 2147483 seconds");
        }
    } else {
        reply("ERROR: unknown command");
    }
}

void console_init(Console *console)
{
    console->length = 0U;
    console->discard_line = false;
    console->previous_cr = false;
    console->rx_failed = false;
    reply("SDK-1.1M lab 2: 57600 8N1; mode 1; timeout 4; I. Enter ? + Enter.");
}

void console_update(Console *console, TrafficLight *light, uint32_t now_ms)
{
    /* Leave RX queued until there is room for the echo and a full response. */
    if (uart_tx_free() < CONSOLE_TX_RESERVE) {
        return;
    }
    if (uart_take_rx_error()) {
        console->discard_line = true;
        console->rx_failed = true;
    }
    uint8_t byte;
    if (!uart_read_byte(&byte)) {
        return;
    }
    const char character = (char)byte;
    (void)uart_write(&character, 1U);

    const bool paired_lf = console->previous_cr && byte == '\n';
    console->previous_cr = byte == '\r';
    if (paired_lf) {
        return;
    }
    if (byte == '\r' || byte == '\n') {
        if (console->discard_line) {
            reply(console->rx_failed ? "ERROR: UART receive error; command discarded"
                                    : "ERROR: command too long or invalid character");
        } else {
            console->line[console->length] = '\0';
            execute(console->line, light, now_ms);
        }
        console->length = 0U;
        console->discard_line = false;
        console->rx_failed = false;
    } else if (byte == '\b' || byte == 127U) {
        if (!console->discard_line && console->length > 0U) {
            --console->length;
            (void)uart_write(" \b", 2U);
        }
    } else if (!console->discard_line) {
        if ((byte < 32U && byte != '\t') || byte > 126U
            || console->length >= sizeof console->line - 1U) {
            console->discard_line = true;
        } else {
            console->line[console->length++] = character;
        }
    }
}
