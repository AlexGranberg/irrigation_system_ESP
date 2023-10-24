// logic.c
#include "pump_state_test.h"

uint8_t checkPumpState(uint16_t adc_percentage) {
    if (adc_percentage < 40) {
        return 1;  // Pump is turned on
    } else {
        return 0;  // Pump is turned off
    }
}