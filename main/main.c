
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "yl69_task.h"
#include "yl69.h"
#include "dht_task.h"
#include "dht.h"
#include "ssd1306.h"
#include "ssd1306_task.h"
#include "connect_wifi.h"
#include "esp_http_client.h"
#include "ultrasonic.h"
#include "ultrasonic_task.h"


static const char *TAG = "HTTP_CLIENT";

char api_key[] = "AI7LUUZI0USAXOAJ";

char message[] = "Hello This is a test message";


SemaphoreHandle_t adc_semaphore = NULL;

float distance_cm;
float distance_percentage;
uint16_t distance_percentage_rounded;


void setup(){
    yl69_setup(YL69_ADC_CHANNEL);
    
}

void thingspeak_send_data(void *pvParameters)
{
    while (1)
    {
        float humidity_float = (float)humidity / 10.0;
        float temperature_float = (float)temperature / 10.0;
        char thingspeak_url[200];
        // snprintf(thingspeak_url,
        //          sizeof(thingspeak_url),
        //          "%s%s%s%s%s",
        //          "https://api.thingspeak.com/update?api_key=",
        //          api_key,
        //          "&field1=", data1, "&field2=", data2);
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

        vTaskDelay(60000 / portTICK_PERIOD_MS); // Delay for 20 seconds
    }
}


void app_main(void){

    esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    adc_semaphore = xSemaphoreCreateMutex();

    xTaskCreate(dht22_task, "dht22_task", 4096, NULL, 5, NULL);
    xTaskCreate(yl69_task, "yl69_task", 4*1024, NULL, 4, NULL);
    xTaskCreate(ssd1306_task, "ssd1306_task", 4096, NULL, 3, NULL);
    xTaskCreate(ultrasonic_task, "ultrasonic_task", 4096, NULL, 2, NULL);

    connect_wifi();
	if (wifi_connect_status){
		xTaskCreate(thingspeak_send_data, "thingspeak_send_data", 8192, NULL, 6, NULL);
	}
}