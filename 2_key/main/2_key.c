#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdint.h>


#define LED_GPIO_NUM  4
#define KEY_GPIO_NUM  16

static QueueHandle_t gpio_evt_queue = NULL;


static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t) (uintptr_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void init_led_gpio(void) {
    gpio_reset_pin(LED_GPIO_NUM);
    gpio_set_direction(LED_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_NUM, 0); // 初始为灭
}

void init_key_gpio(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << (uint64_t) KEY_GPIO_NUM),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
}

static void button_task(void *arg) {
    uint32_t io_num;
    bool led_on = false;

    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // 软件去抖：等待 50ms 再确认状态
            vTaskDelay(pdMS_TO_TICKS(50));

            int level = gpio_get_level((gpio_num_t) io_num);
            if (level == 0) {
                // 仍为按下态
                led_on = !led_on;
                gpio_set_level(LED_GPIO_NUM, led_on ? 1 : 0);

                // 等待按键释放，避免重复触发
                while (gpio_get_level((gpio_num_t) io_num) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
    }
}

void app_main(void) {
    // 初始化 GPIO
    init_led_gpio();
    init_key_gpio();

    // 创建队列用于 ISR -> task 通信
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // 安装 ISR 服务并注册处理函数
    gpio_install_isr_service(0);
    gpio_isr_handler_add(KEY_GPIO_NUM, gpio_isr_handler, (void *) (uintptr_t) KEY_GPIO_NUM);

    // 创建处理按键事件的任务
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);
}
