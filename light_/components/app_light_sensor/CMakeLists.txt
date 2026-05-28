idf_component_register(
        SRCS "app_light_sensor.c"
        INCLUDE_DIRS "."
        REQUIRES esp_adc freertos
)