
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "ssd1306.h"
#include "connect_wifi.h"
#include "esp_http_client.h"

#define I2C_MASTER_SCL_IO 26        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 25        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */


static const char *TAG = "HTTP_CLIENT";

char api_key[] = "CMV42NIJPYJJTSK6";

char message[] = "Hello This is a test message";

unsigned int getDummyData(void){
    uint16_t data;
    data = rand() % 100;
    return data;
}

void thingspeak_send_data(void *pvParameters)
{
    while (1)
    {
        uint16_t data1 = getDummyData();
        uint16_t data2 = getDummyData();
        char thingspeak_url[200];
        // snprintf(thingspeak_url,
        //          sizeof(thingspeak_url),
        //          "%s%s%s%s%s",
        //          "https://api.thingspeak.com/update?api_key=",
        //          api_key,
        //          "&field1=", data1, "&field2=", data2);
        snprintf(thingspeak_url,
                sizeof(thingspeak_url),
                "%s%s&field1=%d&field2=%d",
                "https://api.thingspeak.com/update?api_key=",
                api_key,
                data1, data2);

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

static ssd1306_handle_t ssd1306_dev = NULL;

void app_main(void)
{
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
    char data_str2[20] = {0};
    char data_str3[20] = {0};
    sprintf(data_str1, "Soil: 75%%");
    sprintf(data_str2, "Temperature: 22c");
    sprintf(data_str3, "Humidity: 62%%");
    ssd1306_draw_string(ssd1306_dev, 10, 5, (const uint8_t *)data_str1, 12, 1);
    ssd1306_draw_string(ssd1306_dev, 10, 25, (const uint8_t *)data_str2, 12, 1);
    ssd1306_draw_string(ssd1306_dev, 10, 45, (const uint8_t *)data_str3, 12, 1);
    ssd1306_refresh_gram(ssd1306_dev);


    esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	connect_wifi();
	if (wifi_connect_status)
	{
		xTaskCreate(thingspeak_send_data, "thingspeak_send_data", 8192, NULL, 6, NULL);
	}
}