#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

enum {
    TRAFFIC_DEFAULT_RED_MS = 4000U,
    TRAFFIC_GREEN_MS = 1000U,
    TRAFFIC_BLINK_HALF_PERIOD_MS = 250U,
    TRAFFIC_BLINK_EDGE_COUNT = 8U,
    TRAFFIC_YELLOW_MS = 1000U,
    /* Keep every deadline interval below INT32_MAX milliseconds. */
    TRAFFIC_MAX_TIMEOUT_SECONDS = 2147483U
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
    uint32_t red_ms;
    uint8_t blink_edge;
    bool shorten_next_red;
    bool red_shortened;
    bool button_enabled;
    TrafficLightOutput output;
} TrafficLight;

void traffic_light_init(TrafficLight *light, uint32_t now_ms, TrafficLightOutput output);
void traffic_light_on_button_pressed(TrafficLight *light, uint32_t now_ms);
void traffic_light_update(TrafficLight *light, uint32_t now_ms);
bool traffic_light_set_timeout(TrafficLight *light, uint32_t seconds, uint32_t now_ms);
const char *traffic_light_state_name(const TrafficLight *light);

#endif
