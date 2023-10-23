#include "calculate_percentage_test.h"

#define FULL_DISTANCE_CM 1
#define EMPTY_DISTANCE_CM 8

float distance_percentage = 0;

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