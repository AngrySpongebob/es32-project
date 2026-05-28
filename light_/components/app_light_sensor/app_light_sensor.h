#pragma once

typedef enum {
    LIGHT_OFF,
    LIGHT_ON
} light_state_t;

void app_light_sensor_init(void);
light_state_t app_light_get_state(void);