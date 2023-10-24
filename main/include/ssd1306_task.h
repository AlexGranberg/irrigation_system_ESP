#ifndef __SSD1306_TASK_H__
#define __SSD1306_TASK_H__

#include "freertos/FreeRTOS.h"

#define I2C_MASTER_SCL_IO 26        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 25        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

extern const int epd_bitmap_allArray_LEN;
extern const unsigned char* epd_bitmap_allArray[];

void ssd1306_task(void *pvParameters);

#endif /* __ULTRASONIC_TASK_H__ */