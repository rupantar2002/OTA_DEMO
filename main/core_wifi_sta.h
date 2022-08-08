#ifndef __APP_STA_H__
#define __APP_STA_H__

#undef CORE_WIFI_STA_LOG_ENABLE 

typedef enum
{
    CORE_WIFI_STA_STATUS_ERROR_CONNECTION_FAILED = -2,
    CORE_WIFI_STA_STATUS_ERROR = -1,
    CORE_WIFI_STA_STATUS_OK = 0,
} core_wifi_sta_status_t;

core_wifi_sta_status_t core_wifi_sta_init(char *wifiSsid, char* wifiPassword);
void core_wifi_sta_deInit(void);

core_wifi_sta_status_t core_wifi_sta_getRSSI(int *p_rssi);
uint8_t core_wifi_sta_isConnected(void);
void core_wifi_sta_autoReconnectEnable(uint8_t val);


void core_wifi_sta_wifiDisconnectedEventCallback(uint8_t disconnectReason);
void core_wifi_sta_wifiConnectedEventCallback(esp_netif_ip_info_t ipInfo);
void core_wifi_sta_rssiLowEventCallback(int32_t rssi);

void core_wifi_sta_deinitCallback();
void core_wifi_sta_initCallback();

#endif