#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "send_data_task.h"
#include "connect_wifi.h"
#include "esp_http_client.h"
#include "dht_task.h"
#include "yl69_task.h"
#include "ultrasonic_task.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"

static const char *TAG = "HTTP_CLIENT";

char api_key[] = "";


void thingspeak_send_data(void *pvParameters)
{
    while (1)
    {
        float humidity_float = (float)humidity / 10.0;
        float temperature_float = (float)temperature / 10.0;
        char thingspeak_url[200];
        snprintf(thingspeak_url,
                sizeof(thingspeak_url),
                "%s%s&field1=%.1f&field2=%.1f&field3=%d&field4=%d",
                "https://api.thingspeak.com/update?api_key=",
                api_key,
                humidity_float, temperature_float, adc_percentage, distance_percentage_rounded);

        esp_http_client_config_t config = {
            .url = thingspeak_url,
            .method = HTTP_METHOD_GET,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            int status_code = esp_http_client_get_status_code(client);
            if (status_code == 200)
            {
                ESP_LOGI(TAG, "Message sent Successfully");
            }
            else
            {
                ESP_LOGI(TAG, "Message sent Failed");
            }
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
        esp_http_client_cleanup(client);

        vTaskDelay(120000 / portTICK_PERIOD_MS);
    }
}