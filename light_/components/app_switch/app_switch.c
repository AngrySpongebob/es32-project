#include "app_switch.h"
#include "app_servo.h"
#include "app_light_sensor.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_mqtt.h"

#define TAG "switch"

static void switch_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    app_servo_set_idle();
    vTaskDelay(pdMS_TO_TICKS(500));

    while(1)
    {
        light_state_t st = app_light_get_state();
        mqtt_publish_state(st);
        ESP_LOGI(TAG, "Light state: %s", st ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_switch_trigger_toggle(void)
{
    ESP_LOGI(TAG, "Execute toggle...");
    app_servo_toggle();
    vTaskDelay(pdMS_TO_TICKS(200));
    app_servo_set_idle();
    vTaskDelay(pdMS_TO_TICKS(300));
}

void app_switch_start(void)
{
    xTaskCreate(switch_task, "switch_task", 4096, NULL, 5, NULL);
}