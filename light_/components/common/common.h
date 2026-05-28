#pragma once
#define UNUSED(x) (void)x

#define DEVICE_PREFIX          "smart_switch"
#define SOFTWARE_VERSION        "2.0.0"

// MQTT
#define MQTT_BROKER_URI        "mqtt://192.168.xxx.xxx"
#define MQTT_USERNAME          "user"
#define MQTT_PASSWORD          "pass"

// 配网模式（SoftAP + Web Server）
#define WIFI_AP_SSID_PREFIX    "SmartSwitch_"
#define WIFI_AP_PASSWORD       "12345678"
#define WIFI_CONNECT_TIMEOUT   10000

// 低功耗
#define ENABLE_LOW_POWER       1
#define SLEEP_INTERVAL_SEC     300