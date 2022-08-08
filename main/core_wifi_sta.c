#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "core_wifi_sta.h"
#include "esp_log.h"

static const char *__attribute__((__unused__)) TAG = "CORE_WIFI_STA";

static uint8_t g_isConnected = 0;
static esp_netif_t *gp_staNetif;
static uint8_t g_autoReconnectEnable = 1;

void __attribute__((weak)) core_wifi_sta_wifiDisconnectedEventCallback(uint8_t __attribute__((__unused__)) disconnectReason)
{
}
void __attribute__((weak)) core_wifi_sta_wifiConnectedEventCallback(esp_netif_ip_info_t __attribute__((__unused__)) ipInfo)
{
}
void __attribute__((weak)) core_wifi_sta_rssiLowEventCallback(int32_t __attribute__((__unused__)) rssi)
{
}
void __attribute__((weak)) core_wifi_sta_deinitCallback()
{
}
void __attribute__((weak)) core_wifi_sta_initCallback()
{
}

void core_wifi_sta_autoReconnectEnable(uint8_t flag)
{
    if (flag)
        g_autoReconnectEnable = 1;
    else
        g_autoReconnectEnable = 0;
}

uint8_t core_wifi_sta_isConnected()
{
    return g_isConnected;
}

core_wifi_sta_status_t core_wifi_sta_getRSSI(int *p_rssi)
{
    wifi_ap_record_t ap_info;
    memset(&ap_info, '\0', sizeof(ap_info));
    if (esp_wifi_sta_get_ap_info(&ap_info))
        return CORE_WIFI_STA_STATUS_ERROR;
    *p_rssi = ap_info.rssi;
    return CORE_WIFI_STA_STATUS_OK;
}

static void staReconnectTask(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_wifi_connect();

#ifdef CORE_WIFI_STA_LOG_ENABLE
    ESP_LOGI(TAG, "Retrying WiFi STA connection.");
#endif

    vTaskDelete(NULL);
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *__attribute__((__unused__)) event = (wifi_event_sta_disconnected_t *)event_data;

#ifdef CORE_WIFI_STA_LOG_ENABLE
        ESP_LOGI(TAG, "Disconnect Reason: %d", event->reason);
#endif

        if (g_isConnected)
            core_wifi_sta_wifiDisconnectedEventCallback(event->reason);
        g_isConnected = 0;
        if (g_autoReconnectEnable)
        {
            xTaskCreate(staReconnectTask, "staReconnectTask", 2048, NULL, 10, NULL);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *__attribute__((__unused__)) event = (ip_event_got_ip_t *)event_data;
        g_isConnected = 1;

#ifdef CORE_WIFI_STA_LOG_ENABLE
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
#endif

        core_wifi_sta_wifiConnectedEventCallback(event->ip_info);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_BSS_RSSI_LOW)
    {
        wifi_event_bss_rssi_low_t *event = (wifi_event_bss_rssi_low_t *)event_data;
        core_wifi_sta_rssiLowEventCallback(event->rssi);
#ifdef CORE_WIFI_STA_LOG_ENABLE
        ESP_LOGE(TAG, "Station has low RSSI, %d dBm.", event->rssi);
#endif
    }
}

void core_wifi_sta_deInit(void)
{
    g_autoReconnectEnable = 0;
    core_wifi_sta_deinitCallback();
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
}

core_wifi_sta_status_t core_wifi_sta_init(char *wifiSsid, char *wifiPassword)
{
    // s_wifi_event_group = xEventGroupCreate();
    core_wifi_sta_status_t returnStatus = CORE_WIFI_STA_STATUS_ERROR;
    if (!gp_staNetif)
        gp_staNetif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    strcpy((char *)wifi_config.sta.ssid, wifiSsid);
    strcpy((char *)wifi_config.sta.password, wifiPassword);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

#ifdef CORE_WIFI_STA_LOG_ENABLE
    ESP_LOGI(TAG, "Wifi SSID : %s, SSID Length : %d", wifiPassword, strlen(wifiPassword));
    ESP_LOGI(TAG, "Wifi Password : %s, Password Length : %d", wifiPassword, strlen(wifiPassword));
#endif

    g_autoReconnectEnable = 1;

#ifdef CORE_WIFI_STA_LOG_ENABLE
    ESP_LOGI(TAG, "WiFi Station init finished.");
#endif

    core_wifi_sta_initCallback();

    returnStatus = CORE_WIFI_STA_STATUS_OK;
    return returnStatus;
}