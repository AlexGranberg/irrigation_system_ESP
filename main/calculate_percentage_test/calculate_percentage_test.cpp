#include <catch2/catch_all.hpp>

extern "C" {
#include "calculate_percentage_test.h"
}

TEST_CASE("calculate_percentage function tests", "[TEST]") {
    const double tolerance = 1e-1; // Define a tolerance for floating-point comparisons

    SECTION("Test when current_distance_cm is less than FULL_DISTANCE_CM") {
        double current_distance_cm = 4.0; // Replace with your desired value
        double result = calculate_percentage(current_distance_cm);

        // In this scenario, for the given values of FULL_DISTANCE_CM and EMPTY_DISTANCE_CM
        // the expected result should be 50% (halfway between full and empty)
        double expected = 57.1;

        REQUIRE(std::abs(result - expected) < tolerance);
    }

    SECTION("Test when current_distance_cm is equal to FULL_DISTANCE_CM") {
        double current_distance_cm = 8; // Tank is empty
        double result = calculate_percentage(current_distance_cm);
        double expected = 0.0; // empty tank

        //REQUIRE(std::abs(result - expected) < tolerance);
        REQUIRE(result == expected);
    }

    SECTION("Test when current_distance_cm is equal to or less than EMPTY_DISTANCE_CM") {
        double current_distance_cm = 0.5; // Replace with your desired value
        double result = calculate_percentage(current_distance_cm);
        double expected = 100.0; // Tank is full

        //REQUIRE(std::abs(result - expected) < tolerance);
        REQUIRE(result == expected);
    }
}