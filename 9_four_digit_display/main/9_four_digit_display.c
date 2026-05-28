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
#define WIFI_SSID      "Welcome To 快乐星球"
#define WIFI_PASS      "Xiugaigepi!@#123"
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

    wifi_config_t wifi_config = {0};
    // strlcpy(wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    // strlcpy(wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));
    strlcpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    //TODO ====== 配置认证模式阈值 ======
    // 这里需要根据个人wifi实际认证方式进行修改，WIFI_AUTH_OPEN是 wokwi的wifi配置
    // 常用值：
    // WIFI_AUTH_OPEN      0   开放网络
    // WIFI_AUTH_WPA_PSK   1   WPA
    // WIFI_AUTH_WPA2_PSK  3   WPA2（最常用）
    // WIFI_AUTH_WPA_WPA2_PSK 4 WPA/WPA2 混合
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

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
// ------------------------------
// NTP同步（修复版）
// ------------------------------
static void obtain_ntp_time(void)
{
    // 先设置时区（东八区）
    setenv("TZ", "GMT-8", 1);
    tzset();

    // 启用 SNTP 调试日志（能看到服务器连不连得上！）
    esp_log_level_set("esp-sntp", ESP_LOG_DEBUG);

    // 配置 SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    // 用国内最稳的 NTP 服务器（阿里云 + 国内公共）
    esp_sntp_setservername(0, "ntp.aliyun.com");
    esp_sntp_setservername(1, "time1.aliyun.com");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    bool ntp_success = false;

    ESP_LOGI("NTP", "开始 NTP 同步...(最多等15秒)");

    // 最多等 15 秒，每 500ms 检查一次
    for (int i = 0; i < 30; i++)
    {
        time(&now);
        localtime_r(&now, &timeinfo);

        // 年份 >= 2024 才算真正同步成功
        if (timeinfo.tm_year + 1900 >= 2024) {
            ntp_success = true;
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // ==========================================
    //🔥 关键：这里会告诉你 **到底成功还是失败**
     // ==========================================
     if (ntp_success) {
         ESP_LOGI("NTP", "=============================");
         ESP_LOGI("NTP", "✅ NTP 同步成功！");
         char time_buf[64];
         strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
         ESP_LOGI("NTP", "⏰ 当前时间：%s", time_buf);
         ESP_LOGI("NTP", "=============================");
     } else {
         ESP_LOGE("NTP", "==================================");
         ESP_LOGE("NTP", "❌ NTP 同步失败！还是 1970 年！");
         ESP_LOGE("NTP", "🔴 原因：ESP32 无法连接 NTP 服务器");
         ESP_LOGE("NTP", "✅ 解决：检查 WiFi 是否能上互联网");
         ESP_LOGE("NTP", "==================================");

         // 同步失败，不要继续执行！
         while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
     }

    xEventGroupSetBits(s_system_event_group, NTP_SYNC_DONE_BIT);
}

// =======================================================
// 任务1：系统初始化（NVS + WiFi + NTP）
// =======================================================
void sys_init_task(void* arg)
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
void clock_display_task(void* arg)
{
    // ======================
    // 🔥 等待初始化完成！
    // ======================
    xEventGroupWaitBits(
        s_system_event_group,
        NTP_SYNC_DONE_BIT, // 等待NTP同步完成
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    // 初始化完成，才开始显示
    tm1637_led_t* lcd = tm1637_init(TM1637_CLK, TM1637_DIO);
    tm1637_set_brightness(lcd, 3);

    // 保存上一秒，用来判断秒是否变化
    int last_second = -1;

    while (1)
    {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        int current_sec = timeinfo.tm_sec;
        // 计算要显示的时间：小时 + 分钟
        int time_val = 100 * timeinfo.tm_hour + timeinfo.tm_min;

        // 🔥 每秒翻转一次冒号（标准 1 秒闪烁）
        static bool colon_state = true;

        if (current_sec != last_second) {
            colon_state = !colon_state;  // 每秒变一次
            last_second = current_sec;
        }

        // 刷新显示（包含时间 + 冒号状态）
        tm1637_set_number_lead_dot(lcd, time_val, true, colon_state ? 0xFF : 0x00);

        // 刷新间隔 100ms，显示流畅
        vTaskDelay(pdMS_TO_TICKS(100));
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
    xTaskCreate(sys_init_task, "sys_init", 4096, NULL, 2, NULL);
    xTaskCreate(clock_display_task, "clock", 4096, NULL, 5, NULL);
}
