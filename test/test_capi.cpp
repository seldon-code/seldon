#include "seldon_capi.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE( "C API DeGroot matches the two-agent symmetric case", "[capi]" )
{
    using namespace Catch::Matchers;
    size_t n_in[2]   = { 2, 2 };
    size_t neigh[4]  = { 1, 0, 0, 1 };
    double weight[4] = { 0.2, 0.8, 0.2, 0.8 };
    double x[2]      = { 0.0, 1.0 };
    int rounds       = 0;
    REQUIRE( seldon_degroot_settle( 2, n_in, neigh, weight, x, 1e-6, 100, &rounds ) == 0 );
    REQUIRE( rounds > 0 );
    REQUIRE_THAT( x[0], WithinAbs( 0.5, 1e-5 ) );
    REQUIRE_THAT( x[1], WithinAbs( 0.5, 1e-5 ) );
}
