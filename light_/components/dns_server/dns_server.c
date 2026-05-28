#include "dns_server.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_netif.h"

static const char* TAG = "dns_server";
static TaskHandle_t s_task = NULL;
static bool s_running = false;
static dns_server_config_t s_cfg;

#define DNS_PORT 53
#define DNS_BUF_SIZE 512

// 简单 DNS 报文解析 + 构造应答（只处理 A 记录）
static int dns_reply(uint8_t* buf, int len, uint32_t reply_ip)
{
    if (len < 12) return -1;

    // 删掉这一行：uint16_t tid = *((uint16_t*)&buf[0]);
    uint16_t flags = *((uint16_t*)&buf[2]);
    uint16_t qdcount = *((uint16_t*)&buf[4]);

    if ((flags & 0x8000) || qdcount != 1) return -1;

    // 后续代码不变
    buf[2] = 0x81;
    buf[3] = 0x80;
    *((uint16_t*)&buf[6]) = htons(1);

    int pos = 12;
    while (buf[pos] != 0) pos++;
    pos += 5;

    buf[pos++] = 0xC0;
    buf[pos++] = 0x0C;
    *((uint16_t*)&buf[pos]) = htons(1); pos += 2;
    *((uint16_t*)&buf[pos]) = htons(1); pos += 2;
    *((uint32_t*)&buf[pos]) = htonl(300); pos += 4;
    *((uint16_t*)&buf[pos]) = htons(4); pos += 2;
    *((uint32_t*)&buf[pos]) = htonl(reply_ip); pos += 4;

    return pos;
}

static void dns_server_task(void* arg)
{
    (void)arg;
    uint8_t buf[DNS_BUF_SIZE];

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "socket create fail");
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(DNS_PORT);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ESP_LOGE(TAG, "bind fail");
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "DNS server started (port 53) → 192.168.4.1");
    s_running = true;

    while (s_running) {
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        int len = recvfrom(sock, buf, DNS_BUF_SIZE, 0,
                            (struct sockaddr*)&from, &from_len);
        if (len <= 0) continue;

        int rlen = dns_reply(buf, len, s_cfg.ip);
        if (rlen > 0) {
            sendto(sock, buf, rlen, 0,
                   (struct sockaddr*)&from, from_len);
        }
    }

    close(sock);
    s_task = NULL;
    vTaskDelete(NULL);
}

esp_err_t dns_server_start(const dns_server_config_t* cfg)
{
    if (s_running) return ESP_ERR_INVALID_STATE;
    if (!cfg) return ESP_ERR_INVALID_ARG;

    s_cfg = *cfg;
    if (xTaskCreate(dns_server_task, "dns_server", 4096, NULL, 5, &s_task) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

void dns_server_stop(void)
{
    s_running = false;
    if (s_task) vTaskDelay(pdMS_TO_TICKS(100));
}