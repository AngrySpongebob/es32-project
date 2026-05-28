idf_component_register(
        SRCS "app_mqtt.c"
        INCLUDE_DIRS "."
        REQUIRES espressif__mqtt esp_event freertos app_light_sensor common app_switch
)