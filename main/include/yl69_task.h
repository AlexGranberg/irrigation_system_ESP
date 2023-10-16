#ifndef __YL69_TASK_H__
#define __YL69_TASK_H__


#include "freertos/FreeRTOS.h"

#define PUMP 27
#define YL69_READ_ACTIVE 17
#define YL69_ADC_CHANNEL 36
//#define ADC_CHANNEL_6 36

extern uint16_t adc_reading;
extern uint16_t adc_percentage;

void setup();
void yl69_task(void *arg);

#endif /* __YL69_TASK_H__ */