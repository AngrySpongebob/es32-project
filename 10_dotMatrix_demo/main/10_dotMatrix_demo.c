#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// 引脚
#define DIN 23  // 数据线（发内容）
#define CS  15  // 片选线（开始 / 结束）
#define CLK 18  // 时钟线（同步信号）

// MAX7219 基础驱动
void max7219_send(uint8_t data) {
    for (int i = 0; i < 8; i++, data <<= 1) {
        gpio_set_level(CLK, 0);
        gpio_set_level(DIN, data & 0x80);
        gpio_set_level(CLK, 1);
    }
}

void max7219_write(uint8_t addr, uint8_t data) {
    gpio_set_level(CS, 0);
    max7219_send(addr);
    max7219_send(data);
    gpio_set_level(CS, 1);
}

void max7219_init() {
    gpio_set_direction(DIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);

    // 设置译码模式：0x00 代表无译码（No Decode）。
    // 关闭内部的 BCD 翻译功能，这样才能直接控制每一个 LED 灯珠来显示图案。
    max7219_write(0x09, 0x00);

    // 设置显示亮度：0x03 代表第 3 档亮度（范围是 0x00 到 0x0F，共16级）。
    // 如果觉得太暗，可以把 0x03 改成 0x08 或 0x0F。
    max7219_write(0x0A, 0x03);

    // 设置扫描界限：0x07 代表扫描全部 8 行（数值 0-7 对应 1-8 行
    // 8x8 点阵屏必须设为 0x07，否则会有几行完全不亮。
    max7219_write(0x0B, 0x07);

    // 设置工作模式：0x01 代表正常开启（Normal Operation）
    // 唤醒屏幕，如果设为 0x00 屏幕会进入关机/省电模式（黑屏）。
    max7219_write(0x0C, 0x01);

    // 设置显示测试：0x00 代表关闭测试模式（Normal Display）
    // 如果设为 0x01 屏幕会强制全亮（出厂检测用），这里必须关掉才能正常显示图案。
    max7219_write(0x0F, 0x00);
}

// 显示图片
void max7219_show(const uint8_t *img) {
    for (int i = 0; i < 8; i++) {
        max7219_write(i + 1, img[i]);
    }
}

// ==================== 图案 ====================
// 8x8点阵屏：爱心图案数据
// 数组每1个元素 = 点阵屏1行（共8行）
// 二进制 1 = 灯亮   0 = 灯灭
uint8_t heart[] = {
    0b00000000, // 第0行：全灭 → 顶部空白
    0b01100110, // 第1行：左右各亮两盏 → 爱心上半部分
    0b11111111, // 第2行：全亮 → 爱心最宽处
    0b11111111, // 第3行：全亮 → 爱心饱满区域
    0b01111110, // 第4行：中间亮、两边灭 → 爱心开始收窄
    0b00111100, // 第5行：中间四盏亮 → 继续收窄
    0b00011000, // 第6行：中间两盏亮 → 爱心尖顶
    0b00000000 // 第7行：全灭 → 底部空白
};

// 8x8点阵屏：笑脸图案数据
// 数组里 每1个元素 = 点阵屏的 1行（共8行）
// 二进制 1 = 灯亮   0 = 灯灭
uint8_t smile[] = {
    0b00111100, // 第0行：中间4个亮 → 笑脸额头
    0b01000010, // 第1行：左右各亮1个 → 眼睛外侧
    0b10100101, // 第2行：亮、灭、亮、灭、灭、亮、灭、亮 → 双眼
    0b10000001, // 第3行：左右两端亮 → 脸颊外侧
    0b10100101, // 第4行：同第2行 → 对称脸部
    0b10011001, // 第5行：两端亮 + 中间2个亮 → 嘴巴
    0b01000010, // 第6行：左右各亮1个 → 下巴两侧
    0b00111100 // 第7行：中间4个亮 → 笑脸下巴
};

uint8_t clear[] = {
    0, 0, 0, 0, 0, 0, 0, 0
};

// ==================== 主任务 ====================
void matrix_task(void *arg) {
    max7219_init();

    while (1) {
        max7219_show(heart); // 爱心
        vTaskDelay(pdMS_TO_TICKS(1000));

        max7219_show(smile); // 笑脸
        vTaskDelay(pdMS_TO_TICKS(1000));

        max7219_show(clear); // 清空
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void app_main(void) {
    xTaskCreate(matrix_task, "matrix", 4096, NULL, 5, NULL);
}
