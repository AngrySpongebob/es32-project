#include <esp_log.h>
#include "app_servo.h"
#include "app_light_sensor.h"
#include "app_switch.h"
#include "wifi_manager.h"
#include "app_mqtt.h"
#include "http_server.h"
#include "web_prov.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "==================================");
    ESP_LOGI(TAG, " 商用智能开关 - Web 配网 + 低功耗");
    ESP_LOGI(TAG, " ESP-IDF v6.0.1");
    ESP_LOGI(TAG, "==================================");

    app_servo_init();
    app_light_sensor_init();
    wifi_manager_start();

    // 配网网页
    web_prov_start();

    mqtt_client_start();
    app_switch_start();
    http_server_start();

    ESP_LOGI(TAG, "✅ 系统启动完成");
}