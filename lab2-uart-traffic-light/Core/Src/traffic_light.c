#include "traffic_light.h"

static bool deadline_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return (int32_t)(now_ms - deadline_ms) >= 0;
}

static void apply_output(const TrafficLight *light)
{
    const bool red = light->state == TRAFFIC_STATE_RED;
    const bool yellow = light->state == TRAFFIC_STATE_YELLOW;
    const bool green = light->state == TRAFFIC_STATE_GREEN
        || (light->state == TRAFFIC_STATE_BLINKING_GREEN && light->blink_edge % 2U == 0U);
    light->output(red, yellow, green);
}

static void enter_state(TrafficLight *light, TrafficLightState state, uint32_t now_ms)
{
    light->state = state;
    light->state_started_ms = now_ms;
    light->blink_edge = 0U;
    switch (state) {
        case TRAFFIC_STATE_RED:
            light->red_shortened = light->shorten_next_red;
            light->shorten_next_red = false;
            light->deadline_ms = now_ms + (light->red_shortened ? light->red_ms / 4U : light->red_ms);
            break;
        case TRAFFIC_STATE_GREEN:
            light->deadline_ms = now_ms + TRAFFIC_GREEN_MS;
            break;
        case TRAFFIC_STATE_BLINKING_GREEN:
            light->deadline_ms = now_ms + TRAFFIC_BLINK_HALF_PERIOD_MS;
            break;
        case TRAFFIC_STATE_YELLOW:
            light->deadline_ms = now_ms + TRAFFIC_YELLOW_MS;
            break;
        default:
            break;
    }
    apply_output(light);
}

void traffic_light_init(TrafficLight *light, uint32_t now_ms, TrafficLightOutput output)
{
    light->red_ms = TRAFFIC_DEFAULT_RED_MS;
    light->shorten_next_red = false;
    light->red_shortened = false;
    light->button_enabled = true;
    light->output = output;
    enter_state(light, TRAFFIC_STATE_RED, now_ms);
}

void traffic_light_on_button_pressed(TrafficLight *light, uint32_t now_ms)
{
    if (!light->button_enabled || light->state == TRAFFIC_STATE_GREEN) {
        return;
    }
    if (light->state == TRAFFIC_STATE_RED) {
        const uint32_t deadline = light->state_started_ms + light->red_ms / 4U;
        light->red_shortened = true;
        if (deadline_reached(now_ms, deadline)) {
            enter_state(light, TRAFFIC_STATE_GREEN, now_ms);
        } else {
            light->deadline_ms = deadline;
        }
    } else {
        light->shorten_next_red = true;
    }
}

void traffic_light_update(TrafficLight *light, uint32_t now_ms)
{
    for (uint32_t step = 0U; step < 16U; ++step) {
        if (!deadline_reached(now_ms, light->deadline_ms)) {
            return;
        }
        const uint32_t scheduled_ms = light->deadline_ms;
        switch (light->state) {
            case TRAFFIC_STATE_RED:
                enter_state(light, TRAFFIC_STATE_GREEN, scheduled_ms);
                break;
            case TRAFFIC_STATE_GREEN:
                enter_state(light, TRAFFIC_STATE_BLINKING_GREEN, scheduled_ms);
                break;
            case TRAFFIC_STATE_BLINKING_GREEN:
                ++light->blink_edge;
                if (light->blink_edge >= TRAFFIC_BLINK_EDGE_COUNT) {
                    enter_state(light, TRAFFIC_STATE_YELLOW, scheduled_ms);
                } else {
                    light->deadline_ms = scheduled_ms + TRAFFIC_BLINK_HALF_PERIOD_MS;
                    apply_output(light);
                }
                break;
            case TRAFFIC_STATE_YELLOW:
                enter_state(light, TRAFFIC_STATE_RED, scheduled_ms);
                break;
            default:
                enter_state(light, TRAFFIC_STATE_RED, now_ms);
                return;
        }
    }
}

bool traffic_light_set_timeout(TrafficLight *light, uint32_t seconds, uint32_t now_ms)
{
    if (seconds == 0U || seconds > TRAFFIC_MAX_TIMEOUT_SECONDS) {
        return false;
    }
    light->red_ms = seconds * 1000U;
    if (light->state == TRAFFIC_STATE_RED) {
        light->deadline_ms = light->state_started_ms
            + (light->red_shortened ? light->red_ms / 4U : light->red_ms);
        traffic_light_update(light, now_ms);
    }
    return true;
}

const char *traffic_light_state_name(const TrafficLight *light)
{
    switch (light->state) {
        case TRAFFIC_STATE_RED: return "red";
        case TRAFFIC_STATE_GREEN: return "green";
        case TRAFFIC_STATE_BLINKING_GREEN: return "blinking green";
        case TRAFFIC_STATE_YELLOW: return "yellow";
        default: return "unknown";
    }
}
