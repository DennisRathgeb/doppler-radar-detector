#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "string.h"

#define WIFI_SSID "MyWiFiNetwork"
#define WIFI_PASS "MyWiFiPassword"

// Fast scan only
#define SCAN_METHOD WIFI_FAST_SCAN
#define SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#define RSSI_THRESHOLD -70
#define AUTHMODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define RSSI_5G_ADJUSTMENT 0

static const char *TAG = "scan";

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static void fast_scan(const char *ssid, const char *password)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    wifi_config.sta.scan_method = SCAN_METHOD;
    wifi_config.sta.sort_method = SORT_METHOD;
    wifi_config.sta.threshold.rssi = RSSI_THRESHOLD;
    wifi_config.sta.threshold.authmode = AUTHMODE_THRESHOLD;
    wifi_config.sta.threshold.rssi_5g_adjustment = RSSI_5G_ADJUSTMENT;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    const char *ssid = WIFI_SSID;
    const char *password = WIFI_PASS;

    fast_scan(ssid, password);
}
