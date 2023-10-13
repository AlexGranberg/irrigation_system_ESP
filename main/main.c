
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "yl69.h"
#include "dht.h"
#include "ssd1306.h"
#include "connect_wifi.h"
#include "esp_http_client.h"
#include "ultrasonic.h"

#define I2C_MASTER_SCL_IO 26        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 25        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

#define DHT_READ_DATA 16
#define YL69_READ_ACTIVE 17
#define YL69_ADC_CHANNEL 4
#define ADC_CHANNEL_6 36
#define PUMP 27

#define FULL_DISTANCE_CM 1
#define EMPTY_DISTANCE_CM 8

static const char *TAG = "HTTP_CLIENT";
static const char *TAG2 = "ultrasonic";

char api_key[] = "AI7LUUZI0USAXOAJ";

char message[] = "Hello This is a test message";

int16_t humidity = 0;
int16_t temperature = 0;
int16_t adc_reading = 0;
int16_t adc_percentage = 50;
SemaphoreHandle_t adc_semaphore = NULL;
uint16_t distance_cm;
uint16_t distance_percentage;

ultrasonic_sensor_t ultrasonic = {
    .trigger_pin = 0,
    .echo_pin = 2,
};

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
                humidity_float, temperature_float, adc_percentage, distance_percentage);

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

        vTaskDelay(20000 / portTICK_PERIOD_MS); // Delay for 20 seconds
    }
}

void dht22_task(void *pvParameters){

    while(1){
        dht_read_data(DHT_TYPE_AM2301, DHT_READ_DATA, &humidity, &temperature);

        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}

void setup(){
    yl69_setup(ADC_CHANNEL_6);


}

// static void yl69_task(void *arg) {
//     uint32_t reading_interval = 20000;
//     // Configure YL-69 power control pin as an output
//     esp_rom_gpio_pad_select_gpio(YL69_READ_ACTIVE);
//     gpio_set_direction(YL69_READ_ACTIVE, GPIO_MODE_OUTPUT);

//     // Turn off the YL-69 sensor initially
//     gpio_set_level(YL69_READ_ACTIVE, 0);

//     while(1) {

//         gpio_set_level(YL69_READ_ACTIVE, 1);

//         uint16_t adc_5VReading = 1050;
//         adc_reading = yl69_read();
//         adc_reading = adc_reading - adc_5VReading; 
//         //ESP_LOGI(TAG, "Raw ADC Reading: %d", adc_reading); // Add this line for debugging
//         adc_percentage = yl69_normalization(adc_reading);

//         vTaskDelay(500 / portTICK_PERIOD_MS);

//         if (adc_percentage < 40) {
//             // Soil is dry, increase reading frequency to 1 second

//             //ADD PUMP CONTROLL HERE OR USE ANOTHER TASK FOR THAT?

//             reading_interval = 1000;
//         } else if (adc_percentage > 60) {
//             // Soil is wet, decrease reading frequency to 20 seconds
//             reading_interval = 20000;
//         }
//         gpio_set_level(YL69_READ_ACTIVE, 0);

//         vTaskDelay(reading_interval / portTICK_PERIOD_MS);
//     }
    
//     //vTaskDelete(NULL); 
// }

static void yl69_task(void *arg) {
    uint32_t reading_interval = 20000;
    uint8_t pump_state = 0; // 0: Pump is off, 1: Pump is on

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
            // Soil is dry, increase reading frequency to 1 second
            reading_interval = 1000;

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


void ssd1306_task(void *pvParameters){
    static ssd1306_handle_t ssd1306_dev = NULL;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
    ssd1306_refresh_gram(ssd1306_dev);
    ssd1306_clear_screen(ssd1306_dev, 0x00);

    char data_str1[20] = {0};
    char data_str2[25] = {0};
    char data_str3[40] = {0};

    while (1) {
        // Update data strings based on DHT22 data or other sensors
        snprintf(data_str1, sizeof(data_str1), "Humidity: %.1f%%", (float)humidity / 10.0);
        snprintf(data_str2, sizeof(data_str2), "Temperature: %.1fc", (float)temperature / 10.0);
        snprintf(data_str3, sizeof(data_str3), "Soil: %d%%", adc_percentage);

        // Clear the SSD1306 screen
        ssd1306_clear_screen(ssd1306_dev, 0x00);
        // ...

        // Draw strings on the SSD1306 display
        ssd1306_draw_string(ssd1306_dev, 10, 5, (const uint8_t *)data_str1, 12, 1);
        ssd1306_draw_string(ssd1306_dev, 10, 25, (const uint8_t *)data_str2, 12, 1);
        ssd1306_draw_string(ssd1306_dev, 10, 45, (const uint8_t *)data_str3, 12, 1);
        if (wifi_connect_status){
            ssd1306_draw_bitmap(ssd1306_dev, 120, 47, c_chWiFiConnected88, 8, 8);    
        }
        else {
            ssd1306_draw_bitmap(ssd1306_dev, 120, 47, c_chWiFiDisconnected88, 8, 8); 
        }
        if (distance_percentage >= 0 && distance_percentage < 25) {
            // Display the empty water square icon
            ssd1306_draw_bitmap(ssd1306_dev, 110, 47, c_chwaterSquareEmpty88, 8, 8);  
        } else if (distance_percentage >= 25 && distance_percentage < 50) {
            // Display the 25% filled water square icon
            ssd1306_draw_bitmap(ssd1306_dev, 110, 47, c_chwaterSquareQuarter88, 8, 8);
        } else if (distance_percentage >= 50 && distance_percentage < 75) {
            // Display the 50% filled water square icon
            ssd1306_draw_bitmap(ssd1306_dev, 110, 47, c_chwaterSquareHalf88, 8, 8);
        } else if (distance_percentage >= 75 && distance_percentage < 100) {
            // Display the 75% filled water square icon
            ssd1306_draw_bitmap(ssd1306_dev, 110, 47, c_chwaterSquareThreeQuarter88, 8, 8);
        } else if (distance_percentage >= 100) {
            // Display the 100% filled water square icon
            ssd1306_draw_bitmap(ssd1306_dev, 110, 47, c_chwaterSquareFull88, 8, 8);
        }

        ssd1306_refresh_gram(ssd1306_dev);

        // vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay for 20 seconds

        // // Update data strings based on DHT22 data or other sensors
        // snprintf(data_str1, sizeof(data_str1), "PUMP OFF");
        // snprintf(data_str2, sizeof(data_str2), "Water level: %d%%", distance_percentage);
        // snprintf(data_str3, sizeof(data_str3), "WIFI ON");

        // // Clear the SSD1306 screen
        // ssd1306_clear_screen(ssd1306_dev, 0x00);
        // // ...

        // // Draw strings on the SSD1306 display
        // ssd1306_draw_string(ssd1306_dev, 10, 5, (const uint8_t *)data_str1, 12, 1);
        // ssd1306_draw_string(ssd1306_dev, 10, 25, (const uint8_t *)data_str2, 12, 1);
        // ssd1306_draw_string(ssd1306_dev, 10, 45, (const uint8_t *)data_str3, 12, 1);
        // ssd1306_refresh_gram(ssd1306_dev);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 20 seconds
    }
}

// void control_pump() {
//     esp_rom_gpio_pad_select_gpio(PUMP);
//     gpio_set_direction(PUMP, GPIO_MODE_OUTPUT);
//     gpio_set_level(PUMP, 0);

//     if (adc_percentage < 40) {
//         // Soil is dry, turn on the pump
//         gpio_set_level(PUMP, 1);
//     } else if (adc_percentage > 60) {
//         // Soil is wet, turn off the pump
//         gpio_set_level(PUMP, 0);
//     }
// }

// void pump_task(void *pvParameters) {
//     esp_rom_gpio_pad_select_gpio(PUMP);
//     gpio_set_direction(PUMP, GPIO_MODE_OUTPUT);
//     gpio_set_level(PUMP, 0);

//     while (1) {
//         // if (xSemaphoreTake(adc_semaphore, portMAX_DELAY) == pdTRUE) {
//             if (adc_percentage < 40) {
//                 gpio_set_level(PUMP, 1);
//                 gpio_set_level(YL69_READ_ACTIVE, 1);

//                 // Add a delay to allow the sensor reading to stabilize
//                 vTaskDelay(500 / portTICK_PERIOD_MS);

//                 // Read and update adc_percentage
//                 uint16_t adc_5VReading = 1050;
//                 adc_reading = yl69_read();
//                 adc_reading = adc_reading - adc_5VReading;
//                 adc_percentage = yl69_normalization(adc_reading);

//                 // Turn off the pump if the soil moisture is too high
//                 if (adc_percentage >= 60) {
//                     gpio_set_level(PUMP, 0);
//                     gpio_set_level(YL69_READ_ACTIVE, 0);
//                 }
//             // }
//             // xSemaphoreGive(adc_semaphore);
//         }

//         // Delay between checks
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

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
            ESP_LOGI(TAG2, "measure %d, percentage = %.d", distance_cm, distance_percentage);
        }
        vTaskDelay(20000 / portTICK_PERIOD_MS);
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
        //xTaskCreate(pump_task, "pump_task", 4096, NULL, 2, NULL);
        // xTaskCreate(control_pump, "control_pump", 4096, NULL, 2, NULL);
	}
}