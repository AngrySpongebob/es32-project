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

// 初始化 TM1637 数码管
// 参数：clk -> 时钟引脚   dio -> 数据引脚
// 返回：TM1637 设备句柄（类似设备对象）
tm1637_led_t* tm1637_init(gpio_num_t clk, gpio_num_t dio) {
    // 1. 申请一段内存，用来存放 TM1637 的设备信息（引脚、配置等）
    // calloc 会自动把内存清零，比 malloc 更安全
    tm1637_led_t* lcd = calloc(1, sizeof(tm1637_led_t));

    // 2. 把传入的 CLK 和 DIO 引脚保存到设备结构体中
    lcd->clk = clk;
    lcd->dio = dio;

    // 3. 配置 GPIO 模式
    gpio_config_t io_conf = {};  // 定义GPIO配置结构体，初始化为0

    // 关闭中断（TM1637 通信不需要中断）
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // 设置为推挽输出模式
    io_conf.mode = GPIO_MODE_OUTPUT;

    // 把 CLK 和 DIO 两个引脚一起配置
    io_conf.pin_bit_mask = (1ULL << clk) | (1ULL << dio);

    // 关闭内部下拉电阻
    io_conf.pull_down_en = 0;

    // 关闭内部上拉电阻
    io_conf.pull_up_en = 0;

    // 把上面的配置真正写入硬件寄存器
    gpio_config(&io_conf);

    // 4. 发送 TM1637 初始化指令：设置数据显示模式（普通写显示模式）
    tm1637_start(lcd);        // 发送起始信号
    tm1637_write_byte(lcd, 0x40); // 写指令：0x40 = 数据写模式，自动地址加1
    tm1637_stop(lcd);         // 发送停止信号

    // 5. 设置默认亮度（3级，范围0~7）
    tm1637_set_brightness(lcd, 1);

    // 6. 返回初始化好的 TM1637 设备句柄
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