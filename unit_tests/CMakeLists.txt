
cmake_minimum_required(VERSION 3.16)
project(auto_watering_ESP_unit_tests)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../main/ultrasonic_unit_test ultratest)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../main/calculate_percentage_test percentagetest)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../main/pump_state_test pumpstate)
add_subdirectory(lib/Catch2)


add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests
    Catch2::Catch2WithMain
    ultra_test
    calc_percentage_test
    pump_state_test
)

