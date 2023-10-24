#include <catch2/catch_all.hpp>

extern "C" {
#include "pump_state_test.h"
}

TEST_CASE("Pump State Test", "[pump_state]") {
    // Test when adc_percentage is below 40
    uint8_t result1 = checkPumpState(30);
    REQUIRE(result1 == 1); // Verify that the pump is turned on when adc_percentage is below 40

    // Test when adc_percentage is above 60
    uint8_t result2 = checkPumpState(70);
    REQUIRE(result2 == 0); // Verify that the pump is turned off when adc_percentage is above 60
}


