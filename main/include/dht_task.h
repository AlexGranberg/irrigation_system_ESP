#ifndef __DHT_TASK_H__
#define __DHT_TASK_H__


#include "freertos/FreeRTOS.h"

#define DHT_READ_DATA 16

extern int16_t humidity;
extern int16_t temperature;

void dht22_task(void *pvParameters);

#endif /* __DHT_TASK_H__ */