idf_component_register(SRCS "9_four_digit_display.c"
                    INCLUDE_DIRS "."
        REQUIRES tm1637
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        freertos
        lwip
)
