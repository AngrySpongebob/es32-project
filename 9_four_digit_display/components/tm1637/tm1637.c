#include "tm1637.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const uint8_t seg_table[] = {
    0x3F, 0x06, 0x5B, 0x4F,
    0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F, 0x77, 0x7C,
    0x39, 0x5E, 0x79, 0x71
};

static void tm1637_start(tm1637_led_t* lcd) {
    gpio_set_level(lcd->clk, 1);
    gpio_set_level(lcd->dio, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(lcd->dio, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(lcd->clk, 0);
}

static void tm1637_stop(tm1637_led_t* lcd) {
    gpio_set_level(lcd->clk, 0);
    gpio_set_level(lcd->dio, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(lcd->clk, 1);
    gpio_set_level(lcd->dio, 1);
}

static void tm1637_write_byte(tm1637_led_t* lcd, uint8_t data) {
    for (int i = 0; i < 8; i++) {
        gpio_set_level(lcd->clk, 0);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        if (data & 0x01) gpio_set_level(lcd->dio, 1);
        else gpio_set_level(lcd->dio, 0);
        data >>= 1;
        gpio_set_level(lcd->clk, 1);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    // ACK
    gpio_set_level(lcd->clk, 0);
    gpio_set_level(lcd->dio, 1);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(lcd->clk, 1);
    vTaskDelay(1 / portTICK_PERIOD_MS);
}

tm1637_led_t* tm1637_init(gpio_num_t clk, gpio_num_t dio) {
    tm1637_led_t* lcd = calloc(1, sizeof(tm1637_led_t));
    lcd->clk = clk;
    lcd->dio = dio;

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << clk) | (1ULL << dio);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    tm1637_start(lcd);
    tm1637_write_byte(lcd, 0x40);
    tm1637_stop(lcd);

    tm1637_set_brightness(lcd, 3);
    return lcd;
}

void tm1637_set_brightness(tm1637_led_t* lcd, uint8_t brightness) {
    if (brightness > TM1637_MAX_BRIGHTNESS) brightness = TM1637_MAX_BRIGHTNESS;
    tm1637_start(lcd);
    tm1637_write_byte(lcd, 0x88 | brightness);
    tm1637_stop(lcd);
}

void tm1637_set_segment_raw(tm1637_led_t* lcd, uint8_t pos, uint8_t data) {
    tm1637_start(lcd);
    tm1637_write_byte(lcd, 0xC0 | pos);
    tm1637_write_byte(lcd, data);
    tm1637_stop(lcd);
}

void tm1637_set_segment_number(tm1637_led_t* lcd, uint8_t pos, uint8_t num, bool dot) {
    uint8_t val = seg_table[num & 0x0F];
    if (dot) val |= 0x80;
    tm1637_set_segment_raw(lcd, pos, val);
}

void tm1637_set_number(tm1637_led_t* lcd, int num) {
    tm1637_set_number_lead_dot(lcd, num, true, 0x00);
}

void tm1637_set_number_lead_dot(tm1637_led_t* lcd, int num, bool lead_zero, uint8_t dot_mask) {
    for (int i = 3; i >= 0; i--) {
        int d = num % 10;
        num /= 10;
        bool dot = (dot_mask >> i) & 1;
        if (lead_zero || num > 0 || i == 0) {
            tm1637_set_segment_number(lcd, i, d, dot);
        } else {
            tm1637_set_segment_raw(lcd, i, 0x00);
        }
    }
}