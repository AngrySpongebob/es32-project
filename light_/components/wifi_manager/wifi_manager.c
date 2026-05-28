#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "dns_server.h"
#include "common.h"
// 新增必要头文件
#include "esp_mac.h"
#include "lwip/ip_addr.h"

static const char *TAG = "wifi";
static EventGroupHandle_t s_wifi_event_group;
static bool s_connected = false;

#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_FAIL_BIT          BIT1

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_connected = false;
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "WiFi 连接成功 ✅");
    }
}

static void wifi_start_sta(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL, NULL);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}

static void wifi_start_softap(void)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    char ssid[32];
    snprintf(ssid, 32, "%s%02X%02X", WIFI_AP_SSID_PREFIX, mac[4], mac[5]);

    wifi_config_t ap_cfg = {
        .ap = {
            .ssid = "",
            .ssid_len = 0,
            .password = WIFI_AP_PASSWORD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    memcpy(ap_cfg.ap.ssid, ssid, strlen(ssid));

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    esp_wifi_start();

    dns_server_config_t dns_cfg = DNS_SERVER_CONFIG_DEFAULT_SINGLE("*", "192.168.4.1");
    dns_server_start(&dns_cfg);
    ESP_LOGI(TAG, "SoftAP 已启动：%s", ssid);
}

void wifi_manager_start(void)
{
    s_wifi_event_group = xEventGroupCreate();
    nvs_flash_init();
    esp_event_loop_create_default();

    wifi_start_sta();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE,
        pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT));

    if (!(bits & WIFI_CONNECTED_BIT)) {
        wifi_start_softap();
    }
}

bool wifi_is_connected(void)
{
    return s_connected;
}