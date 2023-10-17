#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "ultrasonic_task.h"
#include "ultrasonic.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"

static const char *TAG = "ultrasonic";

ultrasonic_sensor_t ultrasonic = {
    .trigger_pin = 0,
    .echo_pin = 2,
};


// Calculate the percentage based on the current distance
double calculate_percentage(double current_distance_cm) {
    // Ensure that the distance is within the valid range
    if (current_distance_cm < FULL_DISTANCE_CM) {
        return 100.0; // Tank is full
    } else if (current_distance_cm >= EMPTY_DISTANCE_CM) {
        return 0.0; // Tank is empty
    } else {
        // Calculate the percentage using linear interpolation
        distance_percentage = 100.0 - ((current_distance_cm - FULL_DISTANCE_CM) /
                                      (EMPTY_DISTANCE_CM - FULL_DISTANCE_CM) * 100.0);
        return distance_percentage;
    }
}

void ultrasonic_task(void *pvParameters){
    esp_err_t res = ultrasonic_init(&ultrasonic);

    while (1){
        res = ultrasonic_measure_cm(&ultrasonic, 50, &distance_cm);

        if (res == ESP_OK) {
            distance_percentage = calculate_percentage(distance_cm);
            uint16_t rounded_distance_cm = (int)(distance_cm + 0.5); // Round to the nearest integer
            distance_percentage_rounded = (int)(distance_percentage + 0.5); // Round to the nearest integer
            //ESP_LOGI(TAG2, "measure %d, percentage = %d", rounded_distance_cm, distance_percentage_rounded);
            ESP_LOGI(TAG, "measure %d, distance_cm = %.2f, distance_percentage = %d", rounded_distance_cm, distance_cm, distance_percentage_rounded);

        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    
}