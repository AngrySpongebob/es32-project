#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// 共阴极版,高电平亮，低电平灭

#define SEG_A 16
#define SEG_B 4
#define SEG_C 5
#define SEG_D 18
#define SEG_E 19
#define SEG_F 22
#define SEG_G 23
// #define SEG_H 10 这里没有用到小数点，直接忽略即可


void digital_0() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 1);
    gpio_set_level(SEG_F, 1);
    gpio_set_level(SEG_G, 0);
    // gpio_set_level(SEG_H, 0);
}

void init_gpio() {
    // 初始化 GPIO 引脚
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT, // 设置为输出模式
        .pin_bit_mask = (1ULL << SEG_A) |
                        (1ULL << SEG_B) |
                        (1ULL << SEG_C) |
                        (1ULL << SEG_D) |
                        (1ULL << SEG_E) |
                        (1ULL << SEG_F) |
                        (1ULL << SEG_G),
        // (1ULL << SEG_H),
        .intr_type = GPIO_INTR_DISABLE, // 禁用中断
        .pull_up_en = GPIO_PULLUP_DISABLE, // 禁用上啦
        .pull_down_en = GPIO_PULLDOWN_DISABLE // 禁用下拉
    };
    gpio_config(&io_conf);
    // 初始化完成后，默认显示 0
    // digital_0();
}


void digital_1() {
    gpio_set_level(SEG_A, 0);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 0);
    gpio_set_level(SEG_E, 0);
    gpio_set_level(SEG_F, 0);
    gpio_set_level(SEG_G, 0);
    // gpio_set_level(SEG_H, 0);
}

void digital_2() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 0);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 1);
    gpio_set_level(SEG_F, 0);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void digital_3() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 0);
    gpio_set_level(SEG_F, 0);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void digital_4() {
    gpio_set_level(SEG_A, 0);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 0);
    gpio_set_level(SEG_E, 0);
    gpio_set_level(SEG_F, 1);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void digital_5() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 0);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 0);
    gpio_set_level(SEG_F, 1);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void digital_6() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 0);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 1);
    gpio_set_level(SEG_F, 1);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void digital_7() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 0);
    gpio_set_level(SEG_E, 0);
    gpio_set_level(SEG_F, 0);
    gpio_set_level(SEG_G, 0);
    // gpio_set_level(SEG_H, 0);
}

void digital_8() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 1);
    gpio_set_level(SEG_F, 1);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void digital_9() {
    gpio_set_level(SEG_A, 1);
    gpio_set_level(SEG_B, 1);
    gpio_set_level(SEG_C, 1);
    gpio_set_level(SEG_D, 1);
    gpio_set_level(SEG_E, 0);
    gpio_set_level(SEG_F, 1);
    gpio_set_level(SEG_G, 1);
    // gpio_set_level(SEG_H, 0);
}

void show_number(int number) {
    switch (number) {
        case 0:
            digital_0();
            break;
        case 1:
            digital_1();
            break;
        case 2:
            digital_2();
            break;
        case 3:
            digital_3();
            break;
        case 4:
            digital_4();
            break;
        case 5:
            digital_5();
            break;
        case 6:
            digital_6();
            break;
        case 7:
            digital_7();
            break;
        case 8:
            digital_8();
            break;
        case 9:
            digital_9();
            break;
    }
}

void app_main(void) {
    init_gpio();
    while (1) {
        for (int i = 0; i < 10; i++) {
            show_number(i);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    // 从 0 到 9 无线循环显示
}
