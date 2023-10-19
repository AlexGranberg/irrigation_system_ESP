# add_library(ultrasonic STATIC ${CMAKE_CURRENT_SOURCE_DIR}../test.c)
# add_library(ultrasonic_tests OBJECT ultrasonic_test.cpp)
# target_link_libraries(ultrasonic_tests PRIVATE Catch2::Catch2)
# target_link_libraries(ultrasonic_tests PRIVATE ultrasonic)

add_library(ultrasonic STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../vafantest.c)
add_library(VAFAN OBJECT vafantest.cpp)
target_link_libraries(VAFAN PRIVATE Catch2::Catch2)
target_link_libraries(VAFAN PRIVATE ultrasonic)

