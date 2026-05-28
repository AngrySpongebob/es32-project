#include "app_light_sensor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "light"

static adc_oneshot_unit_handle_t adc1_handle = NULL;
static uint16_t base = 0;

void app_light_sensor_init(void)
{
    // 修正结构体名：adc_oneshot_unit_init_cfg_t
    adc_oneshot_unit_init_cfg_t cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&cfg, &adc1_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &chan_cfg);

    uint32_t sum = 0;
    int raw;
    for(int i = 0; i < 10; i++){
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw);
        sum += raw;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    base = sum / 10;
    ESP_LOGI(TAG, "Base light=%d", base);
}

light_state_t app_light_get_state(void)
{
    uint32_t sum = 0;
    int raw;
    for(int i = 0; i < 10; i++){
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw);
        sum += raw;
    }
    uint16_t avg = sum / 10;
    return (avg > base + 600) ? LIGHT_ON : LIGHT_OFF;
}