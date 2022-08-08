#include <esp_http_server.h>
#include <esp_event_base.h>

 httpd_handle_t app_http_server_start(void);

 esp_err_t app_http_server_stop(httpd_handle_t server);

 void app_http_server_disconnect_handler(void *arg, esp_event_base_t event_base,
                                               int32_t event_id, void *event_data);

 void app_http_server_connect_handler(void *arg, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data);
