#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "dht_task.h"
#include "dht.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"

int16_t humidity = 0; // Declare as pointers and initialize
int16_t temperature = 0;

void dht22_task(void *pvParameters){

    while(1){
        dht_read_data(DHT_TYPE_AM2301, DHT_READ_DATA, &humidity, &temperature);

        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}