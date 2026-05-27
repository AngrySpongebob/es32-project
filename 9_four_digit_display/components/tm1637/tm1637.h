#ifndef TM1637_H
#define TM1637_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TM1637_MAX_BRIGHTNESS 7

typedef struct {
    gpio_num_t clk;
    gpio_num_t dio;
} tm1637_led_t;

tm1637_led_t* tm1637_init(gpio_num_t clk, gpio_num_t dio);
void tm1637_set_brightness(tm1637_led_t* lcd, uint8_t brightness);
void tm1637_set_segment_raw(tm1637_led_t* lcd, uint8_t pos, uint8_t data);
void tm1637_set_segment_number(tm1637_led_t* lcd, uint8_t pos, uint8_t num, bool dot);
void tm1637_set_number(tm1637_led_t* lcd, int num);
void tm1637_set_number_lead_dot(tm1637_led_t* lcd, int num, bool lead_zero, uint8_t dot_mask);

#ifdef __cplusplus
}
#endif

#endif