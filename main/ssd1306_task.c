#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "ssd1306_task.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "dht_task.h"
#include "yl69_task.h"
#include "ultrasonic_task.h"
#include "connect_wifi.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_adc_cal.h"

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

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 20 seconds
    }
}