#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

enum {
    TRAFFIC_RED_MS = 4000U,
    TRAFFIC_GREEN_MS = 1000U,
    TRAFFIC_SHORT_RED_MS = TRAFFIC_RED_MS / 4U,
    TRAFFIC_BLINK_HALF_PERIOD_MS = 250U,
    TRAFFIC_BLINK_EDGE_COUNT = 8U,
    TRAFFIC_YELLOW_MS = 1000U
};

typedef enum {
    TRAFFIC_STATE_RED = 0,
    TRAFFIC_STATE_GREEN,
    TRAFFIC_STATE_BLINKING_GREEN,
    TRAFFIC_STATE_YELLOW
} TrafficLightState;

typedef void (*TrafficLightOutput)(bool red, bool yellow, bool green);

typedef struct {
    TrafficLightState state;
    uint32_t state_started_ms;
    uint32_t deadline_ms;
    uint8_t blink_edge;
    bool shorten_next_red;
    TrafficLightOutput output;
} TrafficLight;

void traffic_light_init(TrafficLight *traffic_light, uint32_t now_ms, TrafficLightOutput output);
void traffic_light_on_button_pressed(TrafficLight *traffic_light, uint32_t now_ms);
void traffic_light_update(TrafficLight *traffic_light, uint32_t now_ms);

#endif
