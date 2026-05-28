#include <string.h>
#include "esp_log.h"
#include "app_mqtt.h"
#include "mqtt_client_config.h"
#include "common.h"
#include "app_switch.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "mqtt";
static esp_mqtt_client_handle_t s_client = NULL;
static char g_device_id[32];

#define TOPIC_CMD    "home/" DEVICE_PREFIX "/%s/command"
#define TOPIC_STATE  "home/" DEVICE_PREFIX "/%s/state"
#define TOPIC_CONFIG "homeassistant/switch/" DEVICE_PREFIX "/%s/config"

static void mqtt_publish_config(void)
{
    char config[512];
    char state_topic[128], cmd_topic[128], uniq_id[64];

    snprintf(state_topic, 128, TOPIC_STATE, g_device_id);
    snprintf(cmd_topic, 128, TOPIC_CMD, g_device_id);
    snprintf(uniq_id, 64, DEVICE_PREFIX "_%s", g_device_id);

    snprintf(config, 512,
        "{"
            "\"name\":\"卧室智能开关\","
            "\"uniq_id\":\"%s\","
            "\"dev\":{\"ids\":[\"%s\"]},"
            "\"pl_on\":\"ON\","
            "\"pl_off\":\"OFF\","
            "\"stat_t\":\"%s\","
            "\"cmd_t\":\"%s\","
            "\"qos\":0"
        "}",
        uniq_id, uniq_id, state_topic, cmd_topic);

    char config_topic[128];
    snprintf(config_topic, 128, TOPIC_CONFIG, g_device_id);

    esp_mqtt_client_publish(s_client, config_topic, config, 0, 1, 0);
    ESP_LOGI(TAG, "HA 自动发现已发送");
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t e = event_data;
    if (e->event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT 连接成功 ✅");
        mqtt_publish_config();

        char cmd_topic[128];
        snprintf(cmd_topic, 128, TOPIC_CMD, g_device_id);
        esp_mqtt_client_subscribe(s_client, cmd_topic, 0);
    }
    else if (e->event_id == MQTT_EVENT_DATA) {
        if (strstr(e->data, "ON") || strstr(e->data, "OFF")) {
            app_switch_trigger_toggle();
        }
    }
}

void mqtt_client_start(void)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    snprintf(g_device_id, 32, "%02X%02X%02X", mac[3], mac[4], mac[5]);

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };

    s_client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);
}

void mqtt_publish_state(light_state_t state)
{
    if (!s_client) return;
    char topic[128];
    snprintf(topic, 128, TOPIC_STATE, g_device_id);
    const char *payload = (state == LIGHT_ON) ? "ON" : "OFF";
    esp_mqtt_client_publish(s_client, topic, payload, 0, 1, 0);
}