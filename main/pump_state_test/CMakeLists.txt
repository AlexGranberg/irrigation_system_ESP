add_library(pumpState STATIC ${CMAKE_CURRENT_SOURCE_DIR}/pump_state_test.c)
add_library(pump_state_test OBJECT pump_state_test.cpp)
target_link_libraries(pump_state_test PRIVATE Catch2::Catch2)
target_link_libraries(pump_state_test PRIVATE pumpState)

