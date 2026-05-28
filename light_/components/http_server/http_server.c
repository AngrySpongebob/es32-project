#include "http_server.h"
#include "app_switch.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define TAG "http"

static esp_err_t handler_toggle(httpd_req_t *req)
{
    app_switch_trigger_toggle();
    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}

void http_server_start(void)
{
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t h;

    httpd_uri_t u = {
        .uri = "/toggle",
        .method = HTTP_GET,
        .handler = handler_toggle
    };
    httpd_start(&h, &c);
    httpd_register_uri_handler(h, &u);
    ESP_LOGI(TAG, "HTTP server on port 80");
}