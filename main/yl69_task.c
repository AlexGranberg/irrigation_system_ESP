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

static const char *TAG = "YL69";

uint16_t adc_reading = 0;
uint16_t adc_percentage = 50;
uint8_t pump_state = 0; // 0: Pump is off, 1: Pump is on


void setup(){
    yl69_setup(YL69_ADC_CHANNEL);
    
}

void yl69_task(void *arg) {
    uint32_t reading_interval = 20000; // 20 seconds for wet soil
    uint16_t watering_timer = 0; // Timer to track how long the pump is on (in ms)
    uint32_t watering_timer_limit = 10000; // Set limit to 10 seconds (10,000 ms)

    // Configure YL-69 power control pin as an output
    esp_rom_gpio_pad_select_gpio(YL69_READ_ACTIVE);
    gpio_set_direction(YL69_READ_ACTIVE, GPIO_MODE_OUTPUT);

    // Initialize the pump pin
    esp_rom_gpio_pad_select_gpio(PUMP);
    gpio_set_direction(PUMP, GPIO_MODE_OUTPUT);
    gpio_set_level(PUMP, 0);

    while(1) {
        gpio_set_level(YL69_READ_ACTIVE, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);  // wait 10ms
        adc_reading = yl69_read();
        adc_percentage = yl69_normalization(adc_reading);
        ESP_LOGI(TAG, "Raw ADC Reading: %d", adc_reading);
        gpio_set_level(YL69_READ_ACTIVE, 0);

        if (adc_percentage < 23) {
            reading_interval = 2500; // 1000 = 1 second

            // Check if the pump is off, then turn it on
            if (pump_state == 0) {
                gpio_set_level(PUMP, 1);
                pump_state = 1;
                watering_timer = 0; // Reset the timer when the pump starts
            }

            // Keep the pump on for the watering_timer_limit (10 seconds) or until soil moisture is above 50%
            while (watering_timer < watering_timer_limit && adc_percentage <= 50) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 1 second
                watering_timer += 1000;  // Increase timer by 1000ms (1 second)

                if (adc_reading > 1){
                    // Read moisture level again in the cycle
                    gpio_set_level(YL69_READ_ACTIVE, 1);
                    vTaskDelay(10 / portTICK_PERIOD_MS);  // wait 10ms
                    adc_reading = yl69_read();
                    adc_percentage = yl69_normalization(adc_reading);
                    ESP_LOGI(TAG, "Raw ADC Reading: %d", adc_reading);
                    ESP_LOGI(TAG, "Percentage ADC Reading: %d", adc_percentage);
                    gpio_set_level(YL69_READ_ACTIVE, 0);
                }
            }

            // Stop the pump after 10 seconds or if the soil moisture is above 50%
            gpio_set_level(PUMP, 0);
            pump_state = 0;
            watering_timer = 0; // Reset for next cycle
            vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay before checking again

        } else {
            // Soil is wet, decrease reading frequency to 20 seconds
            reading_interval = 20000; // Check every 20 seconds
            ESP_LOGI(TAG, "Percentage ADC Reading: %d", adc_percentage); // Add this line for debugging

            // Check if the pump is on, then turn it off
            if (pump_state == 1) {
                gpio_set_level(PUMP, 0);
                pump_state = 0;
            }

            // Wait for the reading interval before checking moisture again
            vTaskDelay(reading_interval / portTICK_PERIOD_MS);
        }
    }
}

