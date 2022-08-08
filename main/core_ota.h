#ifndef __APP_OTA_SERVER_H__
#define __APP_OTA_SERVER_H__

#undef CORE_OTA_LOG_ENABLE 

typedef enum
{
    CORE_OTA_STATUS_ERROR = -1,
    CORE_OTA_STATUS_OK = 0,
} core_ota_status_t;

core_ota_status_t core_ota_init();
core_ota_status_t core_ota_deInit();
core_ota_status_t core_ota_finish();
core_ota_status_t core_ota_write(const void*p_otaData, size_t otaSize);
void core_ota_rebootReadyCallback();

#endif