#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// 配置结构体
typedef struct {
    const char* domain;   // 要劫持的域名，"*"=所有
    uint32_t ip;           // 回复的IP（192.168.4.1）
} dns_server_config_t;

// 默认配置：所有域名 → 192.168.4.1
#define DNS_SERVER_CONFIG_DEFAULT_SINGLE(dom, ip_str) { \
.domain = dom, \
.ip = ipaddr_addr(ip_str) \
}

esp_err_t dns_server_start(const dns_server_config_t* cfg);
void dns_server_stop(void);

#ifdef __cplusplus
}
#endif

#endif