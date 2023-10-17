#ifndef __ULTRASONIC_TASK_H__
#define __ULTRASONIC_TASK_H__

#include "freertos/FreeRTOS.h"

#define FULL_DISTANCE_CM 1
#define EMPTY_DISTANCE_CM 8

extern float distance_cm;
//uint16_t distance_percentage;
extern float distance_percentage;
extern uint16_t distance_percentage_rounded;

void ultrasonic_task(void *pvParameters);
double calculate_percentage(double current_distance_cm);


#endif /* __ULTRASONIC_TASK_H__ */