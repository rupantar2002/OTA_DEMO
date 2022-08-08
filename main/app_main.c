#include <stdio.h>
#include <stdlib.h>
#include <sys/reent.h>

/**/
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
//#include "app_http_server.h"

#include "core_wifi_sta.h"
#include "app_http_server.h"
//static const char *TAG = "APP_MAIN";

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//#define APP_MAIN_NODE_SSID "VCCS Node ID: " STR(APP_CONFIG_NODE_ID_TAG)
#define APP_MAIN_NODE_SSID "Typical"

#define APP_MAIN_NODE_PASS "Arghya@19"

static uint8_t g_wifiApStarted;

void core_wifi_ap_apStartedEventCallback() {
    g_wifiApStarted = 1;
}

static const uint8_t GREEN_LED_GPIO = 4;
static const uint8_t RED_LED_GPIO = 3;
static const uint16_t DELAY = 2000;

static const char *TAG = "app_main";
char *SSID = "Typical";
char *PASSWORD = "Arghya@19";

typedef enum
{
    SELECTION_RED_LED,
    SELECTION_GREEN_LED
} led_sellection_t;

typedef enum
{
    LOW = 0,
    HIGH
} led_state_t;

/* ---------wifi staton code-------- */

void led_blink_task(void *p)
{

    uint8_t selecton = (led_sellection_t)p;
    uint8_t pin = 0;

    if (selecton == SELECTION_RED_LED)
    {
        pin = RED_LED_GPIO;
    }
    else if (selecton == SELECTION_GREEN_LED)
    {
        pin = GREEN_LED_GPIO;
    }
    else
    {
        pin = 0;
    }

    while (pin != 0)
    {
        gpio_set_level(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
        gpio_set_level(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
    }
}

void config_gpio(void)
{
    gpio_set_direction(GREEN_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);
}

void config_wifi_resources(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void app_main(void)
{
    config_gpio();
    config_wifi_resources();

    core_wifi_sta_init(SSID, PASSWORD);
    vTaskDelay(pdMS_TO_TICKS(100));

    if(!core_wifi_sta_isConnected()){
        ESP_LOGI(TAG, "--------connection succesful------");
        app_http_server_start();
        ESP_LOGI(TAG, "--------Server started------");
    }

     xTaskCreate(led_blink_task, "LED_BLIN_TASK", 1024, (void *)SELECTION_RED_LED, 1, NULL);
    //xTaskCreate(led_blink_task, "LED_BLIN_TASK", 1024, (void *)SELECTION_GREEN_LED, 1, NULL);
}
