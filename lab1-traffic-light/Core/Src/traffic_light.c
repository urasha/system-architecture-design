#include "traffic_light.h"

static bool deadline_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return (int32_t)(now_ms - deadline_ms) >= 0;
}

static void apply_output(const TrafficLight *traffic_light)
{
    bool red = false;
    bool yellow = false;
    bool green = false;

    switch (traffic_light->state) {
        case TRAFFIC_STATE_RED:
            red = true;
            break;
        case TRAFFIC_STATE_GREEN:
            green = true;
            break;
        case TRAFFIC_STATE_BLINKING_GREEN:
            green = (traffic_light->blink_edge % 2U) == 0U;
            break;
        case TRAFFIC_STATE_YELLOW:
            yellow = true;
            break;
        default:
            break;
    }

    traffic_light->output(red, yellow, green);
}

static void enter_state(TrafficLight *traffic_light, TrafficLightState state, uint32_t now_ms)
{
    traffic_light->state = state;
    traffic_light->state_started_ms = now_ms;
    traffic_light->blink_edge = 0U;

    switch (state) {
        case TRAFFIC_STATE_RED:
            traffic_light->deadline_ms = now_ms
                + (traffic_light->shorten_next_red ? TRAFFIC_SHORT_RED_MS : TRAFFIC_RED_MS);
            traffic_light->shorten_next_red = false;
            break;
        case TRAFFIC_STATE_GREEN:
            traffic_light->deadline_ms = now_ms + TRAFFIC_GREEN_MS;
            break;
        case TRAFFIC_STATE_BLINKING_GREEN:
            traffic_light->deadline_ms = now_ms + TRAFFIC_BLINK_HALF_PERIOD_MS;
            break;
        case TRAFFIC_STATE_YELLOW:
            traffic_light->deadline_ms = now_ms + TRAFFIC_YELLOW_MS;
            break;
        default:
            break;
    }

    apply_output(traffic_light);
}

void traffic_light_init(TrafficLight *traffic_light, uint32_t now_ms, TrafficLightOutput output)
{
    traffic_light->shorten_next_red = false;
    traffic_light->output = output;
    enter_state(traffic_light, TRAFFIC_STATE_RED, now_ms);
}

void traffic_light_on_button_pressed(TrafficLight *traffic_light, uint32_t now_ms)
{
    if (traffic_light->state == TRAFFIC_STATE_GREEN) {
        return;
    }

    if (traffic_light->state == TRAFFIC_STATE_RED) {
        const uint32_t shortened_deadline = traffic_light->state_started_ms + TRAFFIC_SHORT_RED_MS;
        if (deadline_reached(now_ms, shortened_deadline)) {
            enter_state(traffic_light, TRAFFIC_STATE_GREEN, now_ms);
        } else {
            traffic_light->deadline_ms = shortened_deadline;
        }
    } else {
        traffic_light->shorten_next_red = true;
    }
}

void traffic_light_update(TrafficLight *traffic_light, uint32_t now_ms)
{
    for (uint32_t step = 0U; step < 16U; ++step) {
        if (!deadline_reached(now_ms, traffic_light->deadline_ms)) {
            return;
        }

        const uint32_t scheduled_ms = traffic_light->deadline_ms;
        switch (traffic_light->state) {
            case TRAFFIC_STATE_RED:
                enter_state(traffic_light, TRAFFIC_STATE_GREEN, scheduled_ms);
                break;
            case TRAFFIC_STATE_GREEN:
                enter_state(traffic_light, TRAFFIC_STATE_BLINKING_GREEN, scheduled_ms);
                break;
            case TRAFFIC_STATE_BLINKING_GREEN:
                ++traffic_light->blink_edge;
                if (traffic_light->blink_edge >= TRAFFIC_BLINK_EDGE_COUNT) {
                    enter_state(traffic_light, TRAFFIC_STATE_YELLOW, scheduled_ms);
                } else {
                    traffic_light->deadline_ms = scheduled_ms + TRAFFIC_BLINK_HALF_PERIOD_MS;
                    apply_output(traffic_light);
                }
                break;
            case TRAFFIC_STATE_YELLOW:
                enter_state(traffic_light, TRAFFIC_STATE_RED, scheduled_ms);
                break;
            default:
                enter_state(traffic_light, TRAFFIC_STATE_RED, now_ms);
                return;
        }
    }
}
