add_library(ultrasonic STATIC ${CMAKE_CURRENT_SOURCE_DIR}/ultra_test.c)
add_library(ultra_test OBJECT ultra_test.cpp)
target_link_libraries(ultra_test PRIVATE Catch2::Catch2)
target_link_libraries(ultra_test PRIVATE ultrasonic)

