#pragma once
#include "app_light_sensor.h"

void mqtt_client_start(void);
void mqtt_publish_state(light_state_t state);