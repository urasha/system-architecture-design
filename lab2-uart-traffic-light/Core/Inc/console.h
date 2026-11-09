#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stddef.h>

#include "traffic_light.h"

typedef struct {
    char line[96];
    size_t length;
    bool discard_line;
    bool previous_cr;
    bool rx_failed;
} Console;

void console_init(Console *console);
void console_update(Console *console, TrafficLight *light, uint32_t now_ms);

#endif
