add_library(calculatePercentage STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../calculate_percentage_test.c)
add_library(calc_percentage_test OBJECT calculate_percentage_test.cpp)
target_link_libraries(calc_percentage_test PRIVATE Catch2::Catch2)
target_link_libraries(calc_percentage_test PRIVATE calculatePercentage)

