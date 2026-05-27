idf_component_register(
        SRCS "tm1637.c"
        INCLUDE_DIRS "."
        REQUIRES esp_driver_gpio freertos
)