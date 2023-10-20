#include <catch2/catch_all.hpp>
#include "../ultra_test.h"

TEST_CASE("Our First Test", "[Test]") {
    REQUIRE(1 == 1);
    REQUIRE(4 == 4);
    REQUIRE(2 == 3);
}
