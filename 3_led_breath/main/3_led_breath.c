/* 简洁的 ESP-IDF v6.0 LED 呼吸灯（GPIO21）
   使用 LEDC 的 fade 接口实现平滑淡入淡出，代码更短且直观。
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define LED_GPIO_NUM 18
#define LED_DUTY_RES   LEDC_TIMER_10_BIT
#define LED_FREQ_HZ    5000

static void init_led(void)
{
    // 初始化 LEDC timer
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LED_DUTY_RES,
        .freq_hz = LED_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    // 初始化 LEDC channel
    ledc_channel_config_t ch = {
        .gpio_num = LED_GPIO_NUM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));
}

void app_main(void)
{
    // 初始化 LED
    init_led();

    const int max_duty = (1 << LED_DUTY_RES) - 1; // 最大占空比 (1023)
    const int step = 4;     // 步进，越小越平滑但越慢
    const int delay_ms = 10; // 每步延时，控制速度

    // 简单的三角波呼吸：从 0 增加到 max，再降回 0，循环
    while (1) {
        for (int d = 0; d <= max_duty; d += step) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, d);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        for (int d = max_duty; d >= 0; d -= step) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, d);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
}
