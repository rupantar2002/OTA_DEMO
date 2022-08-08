#include <string.h>
#include "esp_ota_ops.h"
#include "core_ota.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *__attribute__((__unused__)) TAG = "CORE_OTA";

static esp_ota_handle_t g_otaHandle = 0;
static const esp_partition_t *gp_updatePartition = NULL;

void __attribute__((weak)) core_ota_rebootReadyCallback()
{
}

core_ota_status_t core_ota_init()
{
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_err_t err;
    core_ota_status_t returnStatus = CORE_OTA_STATUS_ERROR;

#ifdef CORE_OTA_LOG_ENABLE
    ESP_LOGI(TAG, "Starting OTA");
#endif

    gp_updatePartition = esp_ota_get_next_update_partition(NULL);

#ifdef CORE_OTA_LOG_ENABLE
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             gp_updatePartition->subtype, gp_updatePartition->address);
#endif

    assert(gp_updatePartition != NULL);

    err = esp_ota_begin(gp_updatePartition, OTA_SIZE_UNKNOWN, &g_otaHandle);
    if (err != ESP_OK)
    {
        returnStatus = CORE_OTA_STATUS_ERROR;
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
    }
    else
    {
        returnStatus = CORE_OTA_STATUS_OK;

#ifdef CORE_OTA_LOG_ENABLE
        ESP_LOGI(TAG, "esp_ota_begin succeeded");
#endif
    }

    return returnStatus;
}

core_ota_status_t core_ota_finish()
{
    esp_err_t err;
    if (esp_ota_end(g_otaHandle) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        return CORE_OTA_STATUS_ERROR;
    }
    err = esp_ota_set_boot_partition(gp_updatePartition);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return CORE_OTA_STATUS_ERROR;
    }
#ifdef CORE_OTA_LOG_ENABLE
    ESP_LOGI(TAG, "Prepare to restart system!");
#endif
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
    return CORE_OTA_STATUS_OK;
}

core_ota_status_t core_ota_write(const void *p_otaData, size_t otaSize)
{
    esp_err_t err;
    err = esp_ota_write(g_otaHandle, p_otaData, otaSize);
    if (err == ESP_OK)
        return CORE_OTA_STATUS_OK;
    else
        return CORE_OTA_STATUS_ERROR;
}

core_ota_status_t core_ota_deInit()
{
    esp_err_t err;
    core_ota_status_t returnStatus = CORE_OTA_STATUS_OK;
    if (g_otaHandle)
    {
        err = esp_ota_abort(g_otaHandle);
        if (err == ESP_OK)
        {
            g_otaHandle = 0;
            returnStatus = CORE_OTA_STATUS_OK;
        }
        else
            returnStatus = CORE_OTA_STATUS_ERROR;
    }
    return returnStatus;
}
