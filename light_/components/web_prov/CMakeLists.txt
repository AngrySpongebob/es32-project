idf_component_register(
        SRCS "web_prov.c"
        INCLUDE_DIRS "."
        REQUIRES esp_http_server esp_wifi common
)