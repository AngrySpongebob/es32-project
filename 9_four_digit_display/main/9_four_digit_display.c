#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "nvs_flash.h"

#include "tm1637.h"

#define TAG "clock"

// ==================== 你的WiFi ====================
#define WIFI_SSID      "Wokwi-GUEST"
#define WIFI_PASS      ""
// ===================================================

#define TM1637_CLK GPIO_NUM_4
#define TM1637_DIO GPIO_NUM_5

// 事件组位定义
#define WIFI_CONNECTED_BIT  BIT0
#define NTP_SYNC_DONE_BIT   BIT1

// 全局事件组
static EventGroupHandle_t s_system_event_group;

// ------------------------------
// WiFi事件回调
// ------------------------------
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_system_event_group, WIFI_CONNECTED_BIT);
    }
}

// ------------------------------
// WiFi初始化
// ------------------------------
static void wifi_init_sta(void)
{
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL, NULL);

    wifi_config_t wifi_config = { 0 };
    strlcpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    //TODO ====== 配置认证模式阈值 ======
    // 这里需要根据个人wifi实际认证方式进行修改，WIFI_AUTH_OPEN是 wokwi的wifi配置
    // 常用值：
    // WIFI_AUTH_OPEN      0   开放网络
    // WIFI_AUTH_WPA_PSK   1   WPA
    // WIFI_AUTH_WPA2_PSK  3   WPA2（最常用）
    // WIFI_AUTH_WPA_WPA2_PSK 4 WPA/WPA2 混合
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    // 等待WiFi连接成功
    xEventGroupWaitBits(
        s_system_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );
}

// ------------------------------
// NTP同步
// ------------------------------
static void obtain_ntp_time(void)
{
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.ntsc.ac.cn");
    esp_sntp_setservername(1, "cn.pool.ntp.org");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };

    for (int i = 0; i < 30; i++)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year > 100) break;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    setenv("TZ", "GMT-8", 1);
    tzset();

    // NTP同步完成 → 发信号
    xEventGroupSetBits(s_system_event_group, NTP_SYNC_DONE_BIT);
}

// =======================================================
// 任务1：系统初始化（NVS + WiFi + NTP）
// =======================================================
void sys_init_task(void *arg)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_sta();
    obtain_ntp_time();

    vTaskDelete(NULL);
}

// =======================================================
// 任务2：时钟显示（等待初始化完成才运行）
// =======================================================
void clock_display_task(void *arg)
{
    // ======================
    // 🔥 等待初始化完成！
    // ======================
    xEventGroupWaitBits(
        s_system_event_group,
        NTP_SYNC_DONE_BIT,   // 等待NTP同步完成
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    // 初始化完成，才开始显示
    tm1637_led_t *lcd = tm1637_init(TM1637_CLK, TM1637_DIO);
    tm1637_set_brightness(lcd, 3);

    while (1)
    {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        int time_val = 100 * timeinfo.tm_hour + timeinfo.tm_min;
        tm1637_set_number_lead_dot(lcd, time_val, true, timeinfo.tm_sec % 2 ? 0xFF : 0x00);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// =======================================================
// MAIN：创建事件组 + 创建两个任务
// =======================================================
void app_main(void)
{
    // 创建系统事件组（同步任务用）
    s_system_event_group = xEventGroupCreate();

    // 创建任务
    xTaskCreate(sys_init_task,     "sys_init",  4096, NULL, 2, NULL);
    xTaskCreate(clock_display_task,"clock",     4096, NULL, 5, NULL);
}