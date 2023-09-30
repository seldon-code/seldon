#include <catch2/catch_test_macros.hpp>

#include "connectivity.hpp"
#include <vector>

void create_neighbour_list( uint32_t number ) {
    return number <= 1 ? number : factorial(number-1) * number;
}

TEST_CASE( "Test for Tarjan's algorithm for strongly connected networks", "[tarjan]" ) {
    REQUIRE( factorial( 1) == 1 );
    REQUIRE( factorial( 2) == 2 );
    REQUIRE( factorial( 3) == 6 );
}