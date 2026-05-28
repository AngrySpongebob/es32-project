idf_component_register(
        SRCS "main.c"
        INCLUDE_DIRS "."
        REQUIRES
        app_servo
        app_light_sensor
        app_switch
        wifi_manager
        app_mqtt_client
        http_server
        web_prov
        common
)