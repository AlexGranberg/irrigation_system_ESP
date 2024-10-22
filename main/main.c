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

//#include "esp_https_ota.h"
//#include "cJSON.h"

#include "common.h"

#define UPDATE_JSON_URL 		"https://github.com/AlexGranberg/irrigation_system_ESP/tree/main/bin"
/*
// receive buffer
char rcv_buffer[200];

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    
	switch(evt->event_id) {
        case HTTP_EVENT_REDIRECT:
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
				strncpy(rcv_buffer, (char*)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}


void check_update_task(void *pvParameter) {
	int cnt = 0;
	while(1) {
        char buf[255];
        sprintf(buf, "%s?token=%d",UPDATE_JSON_URL,cnt);
        cnt++;
		printf("Looking for a new firmware at %s", buf);
	
		// configure the esp_http_client
		esp_http_client_config_t config = {
        .url = buf,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,        
        .timeout_ms = 30000,
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);
	
		// downloading the json file
		esp_err_t err = esp_http_client_perform(client);
		if(err == ESP_OK) {
			
			// parse the json file	
			cJSON *json = cJSON_Parse(rcv_buffer);
			if(json == NULL) printf("downloaded file is not a valid json, aborting...\n");
			else {	
				cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
				cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
				
				// check the version
				if(!cJSON_IsNumber(version)) printf("unable to read new version, aborting...\n");
				else {
					
					double new_version = version->valuedouble;
					if(new_version > FIRMWARE_VERSION) {
						
						printf("current firmware version (%.1f) is lower than the available one (%.1f), updating...\n", FIRMWARE_VERSION, new_version);
						if(cJSON_IsString(file) && (file->valuestring != NULL)) {
							printf("downloading and installing new firmware (%s)...\n", file->valuestring);
							
							esp_http_client_config_t ota_client_config = {
								.url = file->valuestring,
                                .keep_alive_enable = true,
							};
                            esp_https_ota_config_t ota_config = {
                                    .http_config = &ota_client_config,
                            };                            
							esp_err_t ret = esp_https_ota(&ota_config);
							if (ret == ESP_OK) {
								printf("OTA OK, restarting...\n");
								esp_restart();
							} else {
								printf("OTA failed...\n");
							}
						}
						else printf("unable to read the new file name, aborting...\n");
					}
					else printf("current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...\n", FIRMWARE_VERSION, new_version);
				}
			}
		}
		else printf("unable to download the json file, aborting...\n");
		
		// cleanup
		esp_http_client_cleanup(client);
		
		printf("\n");
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}
*/

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
		//xTaskCreate(check_update_task, "check_update_task", 8192, NULL, 5, NULL);
	}
}