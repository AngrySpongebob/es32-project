#include "app_servo.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define TAG "servo"
#define SERVO_GPIO            13
#define SERVO_CH              LEDC_CHANNEL_0
#define SERVO_TIMER           LEDC_TIMER_0
#define SERVO_MODE            LEDC_LOW_SPEED_MODE

#define SERVO_IDLE_US     1500   // 中位
#define SERVO_ACT_US      2000   // 拨动
#define SERVO_FREQ        50

void app_servo_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = SERVO_MODE,
        .timer_num = SERVO_TIMER,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = LEDC_TIMER_14_BIT,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = SERVO_MODE,
        .channel = SERVO_CH,
        .timer_sel = SERVO_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ch);

    ledc_set_duty(SERVO_MODE, SERVO_CH, ((SERVO_IDLE_US * (1<<14)) / 20000));
    ledc_update_duty(SERVO_MODE, SERVO_CH);
    ESP_LOGI(TAG, "Servo init OK");
}

void app_servo_set_idle(void)
{
    uint32_t duty = ((SERVO_IDLE_US * (1<<14)) / 20000);
    ledc_set_duty(SERVO_MODE, SERVO_CH, duty);
    ledc_update_duty(SERVO_MODE, SERVO_CH);
}

void app_servo_toggle(void)
{
    uint32_t duty_act = ((SERVO_ACT_US * (1<<14)) / 20000);
    ledc_set_duty(SERVO_MODE, SERVO_CH, duty_act);
    ledc_update_duty(SERVO_MODE, SERVO_CH);
}