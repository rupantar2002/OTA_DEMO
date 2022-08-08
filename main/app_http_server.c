
//#include <esp_http_server.h>
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <http_parser.h>
#include <sdkconfig.h>
#include <esp_err.h>

#include <esp_log.h>
#include <esp_err.h>
#include <freertos/event_groups.h>
#include <esp_http_server.h>
#include <esp_event_base.h>
#include <cJSON.h>
#include "core_ota.h"

#define MIN(a, b) a > b ? b : a

static const char *APP_HTTP_SERVER_TAG = "app_http_Server";

esp_err_t app_http_server_get_handler(httpd_req_t *req){

    //ESP_LOGI(TAG, "GET SETUP HTML REQUEST");
    extern const unsigned char ota_html_start[] asm("_binary_ota_html_start");
    extern const unsigned char ota_html_end[] asm("_binary_ota_html_end");
    const size_t ota_html_size = (ota_html_end - ota_html_start);
    httpd_resp_send_chunk(req, (const char *)ota_html_start, ota_html_size);

    //terminate html header
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;

}

 esp_err_t app_http_server_post_handler(httpd_req_t *req)
{
    static char otaDataBuff[8000] = {0};
    uint64_t otaFileSize = req->content_len;
    int dataReadLen;
    //ESP_LOGI(TAG,"OTA File size: %llu\r\n", otaFileSize);
    while (otaFileSize > 0)
    {

        if ((dataReadLen = httpd_req_recv(req, otaDataBuff, sizeof(otaDataBuff))) <= 0)
        {
            if (dataReadLen == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
        }
        else
        {
            otaFileSize -= dataReadLen;
            core_ota_status_t err = core_ota_write(otaDataBuff, dataReadLen);
            // if ( err!= CORE_OTA_STATUS_OK)
            // {
            //     return ESP_FAIL;
            // }
        }
    }
    extern const unsigned char ota_html_start[] asm("_binary_ota_html_start");
    extern const unsigned char ota_html_end[] asm("_binary_ota_html_end");
    const size_t ota_html_size = (ota_html_end - ota_html_start);
    httpd_resp_send_chunk(req, (const char *)ota_html_start, ota_html_size);
    httpd_resp_sendstr_chunk(req, NULL);
    core_ota_finish();
    return ESP_OK;
    
}

/* URI handler structure for GET /uri */
httpd_uri_t app_http_server_uri_get = {
    .uri      = "/ota",
    .method   = HTTP_GET,
    .handler  = app_http_server_get_handler,
    .user_ctx = NULL,
};
httpd_uri_t app_http_server_uri_post = {
    .uri      = "/ota",
    .method   = HTTP_POST,
    .handler  = app_http_server_post_handler,
    .user_ctx = NULL,
};

 httpd_handle_t app_http_server_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(APP_HTTP_SERVER_TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        //ESP_LOGI(APP_HTTP_SERVER_TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &app_http_server_uri_get);
        httpd_register_uri_handler(server, &app_http_server_uri_post);

        return server;
    }

    //ESP_LOGI(APP_HTTP_SERVER_TAG, "Error starting server!");
    return NULL;
}

 esp_err_t app_http_server_stop(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

 void app_http_server_disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        //ESP_LOGI(APP_HTTP_SERVER_TAG, "Stopping webserver");
        if (app_http_server_stop(*server) == ESP_OK) {
            *server = NULL;
        } else {
            //ESP_LOGE(APP_HTTP_SERVER_TAG, "Failed to stop http server");
        }
    }
}

 void app_http_server_connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        //ESP_LOGI(APP_HTTP_SERVER_TAG, "Starting webserver");
        *server = app_http_server_start();
    }
}


// void app_main(void)
// {
//     static httpd_handle_t server = NULL;

//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     /* Start the server for the first time */
//     server = start_webserver();
// }