#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN_RED GPIO_NUM_18
#define LED_PIN_YELLOW GPIO_NUM_19
#define LED_PIN_GREEN GPIO_NUM_21
/**
 * 初始化多个引脚
 */
void init_gpio() {
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << LED_PIN_RED) | (1ULL << LED_PIN_YELLOW) | (1ULL << LED_PIN_GREEN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&config);

    gpio_set_level(LED_PIN_RED, 0); // 红灯灭
    gpio_set_level(LED_PIN_YELLOW, 0); // 黄灯灭
    gpio_set_level(LED_PIN_GREEN, 0); // 绿灯灭
}

void app_main(void) {
    init_gpio();

    while (1) {
        //红灯亮
        gpio_set_level(LED_PIN_RED, 1);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN_RED, 0); // 红灯灭
        // 黄灯闪烁三次
        for (int i = 0; i < 3; ++i) {
            gpio_set_level(LED_PIN_YELLOW, 1);
            vTaskDelay(600 / portTICK_PERIOD_MS);
            gpio_set_level(LED_PIN_YELLOW, 0);
            vTaskDelay(600 / portTICK_PERIOD_MS);
        }
        // 绿灯亮
        gpio_set_level(LED_PIN_GREEN, 1);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN_GREEN, 0);
    }
}
