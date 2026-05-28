#include "web_prov.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "common.h"

static const char *TAG = "web_prov";

static const char HTML[] =
"<!DOCTYPE html><html><head><meta charset='utf-8'>"
"<title>智能开关配网</title></head><body>"
"<h3>WiFi 配网</h3>"
"<form action='/set'>"
"WiFi名称: <input name='ssid'><br><br>"
"密码: <input name='pass'><br><br>"
"<button type='submit'>保存并连接</button>"
"</form></body></html>";

static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, HTML);
    return ESP_OK;
}

static esp_err_t set_handler(httpd_req_t *req)
{
    char buf[256];
    int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
    if (ret == ESP_OK) {
        char ssid[64], pass[64];
        httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid));
        httpd_query_key_value(buf, "pass", pass, sizeof(pass));

        wifi_config_t cfg = {0};
        strcpy((char*)cfg.sta.ssid, ssid);
        strcpy((char*)cfg.sta.password, pass);

        esp_wifi_set_config(WIFI_IF_STA, &cfg);
        esp_wifi_connect();
        httpd_resp_sendstr(req, "OK，重启中...");
    }
    return ESP_OK;
}

void web_prov_start(void)
{
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t h;

    httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
    httpd_uri_t set = {.uri = "/set", .method = HTTP_GET, .handler = set_handler};

    httpd_start(&h, &c);
    httpd_register_uri_handler(h, &root);
    httpd_register_uri_handler(h, &set);
    ESP_LOGI(TAG, "配网网页已启动");
}