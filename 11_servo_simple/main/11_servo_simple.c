#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "iot_servo.h"

/* ==================== 配置 ==================== */
#define SERVO_GPIO      15          // 舵机信号线
#define SERVO_CHANNEL   0           // 第 0 路舵机

/**
 * @brief  舵机控制任务
 */
static void servo_task(void *arg)
{
    // 1. 官方标准初始化配置
    servo_config_t servo_cfg = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50,
        .timer_number = LEDC_TIMER_0,
        .channels = {
            .servo_pin = {
                SERVO_GPIO,
            },
            .ch = {
                LEDC_CHANNEL_0,
            },
        },
        .channel_number = 1,
    };

    // 2. 官方初始化函数
    iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);

    while (1) {
        // 官方设置角度 API
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, 0);
        printf("Angle: 0\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, 90);
        printf("Angle: 90\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, SERVO_CHANNEL, 180);
        printf("Angle: 180\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    xTaskCreate(servo_task, "servo_task", 4096, NULL, 5, NULL);
}