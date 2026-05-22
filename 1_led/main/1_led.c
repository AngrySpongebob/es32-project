#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO_NUM  GPIO_NUM_21

/**
 * 初始化21引脚
 */
void init_gpio(void) {
    gpio_reset_pin(LED_GPIO_NUM);
    gpio_set_direction(LED_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_NUM, 0); // 一开始灭掉
}

void app_main(void) {
    init_gpio();

    while (1) {
        gpio_set_level(LED_GPIO_NUM, 1); // 亮
        vTaskDelay(pdMS_TO_TICKS(500)); // .5秒闪一次

        gpio_set_level(LED_GPIO_NUM, 0); // 灭
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
