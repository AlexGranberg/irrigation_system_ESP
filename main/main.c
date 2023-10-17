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
#include "send_data_task.h"



void app_main(void){

    esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    xTaskCreate(dht22_task, "dht22_task", 4096, NULL, 5, NULL);
    xTaskCreate(yl69_task, "yl69_task", 4*1024, NULL, 4, NULL);
    xTaskCreate(ssd1306_task, "ssd1306_task", 4096, NULL, 3, NULL);
    xTaskCreate(ultrasonic_task, "ultrasonic_task", 4096, NULL, 2, NULL);

    connect_wifi();
	if (wifi_connect_status){
		xTaskCreate(thingspeak_send_data, "thingspeak_send_data", 8192, NULL, 6, NULL);
	}
}