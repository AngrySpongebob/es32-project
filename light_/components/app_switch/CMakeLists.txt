idf_component_register(
        SRCS "app_switch.c"
        INCLUDE_DIRS "."
        REQUIRES app_mqtt_client freertos app_servo
)