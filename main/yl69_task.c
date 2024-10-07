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

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"

uint16_t adc_reading = 0;
uint16_t adc_percentage = 50;
uint8_t pump_state = 0; // 0: Pump is off, 1: Pump is on


void setup(){
    yl69_setup(YL69_ADC_CHANNEL);
    
}

void yl69_task(void *arg) {
    uint16_t adc_reading_counter = 0;
    uint16_t watering_timer = 0;
    uint16_t watering_timer_limit = 10000;
    // Configure YL-69 power control pin as an output
    esp_rom_gpio_pad_select_gpio(YL69_READ_ACTIVE);
    gpio_set_direction(YL69_READ_ACTIVE, GPIO_MODE_OUTPUT);

    // Turn off the YL-69 sensor initially
    gpio_set_level(YL69_READ_ACTIVE, 0);

    // Initialize the pump pin
    esp_rom_gpio_pad_select_gpio(PUMP);
    gpio_set_direction(PUMP, GPIO_MODE_OUTPUT);
    gpio_set_level(PUMP, 0);

    while(1) {
        gpio_set_level(YL69_READ_ACTIVE, 1);

        uint16_t adc_5VReading = 1050;
        adc_reading = yl69_read();
        adc_reading = adc_reading - adc_5VReading;
        //ESP_LOGI(TAG, "Raw ADC Reading: %d", adc_reading); // Add this line for debugging
        adc_percentage = yl69_normalization(adc_reading);

        if (adc_percentage < 40) {
            // Soil is dry, increase reading frequency to 2 second
            reading_interval = 2000;

            // Check if the pump is off, then turn it on
            if (pump_state == 0) {
                gpio_set_level(PUMP, 1);
                pump_state = 1;
            }
        } else if (adc_percentage > 60) {
            // Soil is wet, decrease reading frequency to 20 seconds
            reading_interval = 20000;

            // Check if the pump is on, then turn it off
            if (pump_state == 1) {
                gpio_set_level(PUMP, 0);
                pump_state = 0;
            }
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(YL69_READ_ACTIVE, 0);

        vTaskDelay(reading_interval / portTICK_PERIOD_MS);
    }
}