#include "catch2/matchers/catch_matchers.hpp"
#include "util/misc.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

TEST_CASE( "Test parse_comma_separated_list", "[util_parse_list]" )
{
    const std::string str = "12, aa, -2.0, 10, 13.0  \n";

    std::vector<double> dbl_vec{};
    std::vector<int> int_vec{};

    std::vector<double> dbl_vec_expected = { -2.0, 13.0 };
    std::vector<int> int_vec_expected    = { 12, 10 };

    auto callback = [&]( int idx_list, std::string & substr )
    {
        if( idx_list == 0 || idx_list == 3 )
        {
            int_vec.push_back( std::stoi( substr ) );
        }
        else if( idx_list == 2 || idx_list == 4 )
        {
            dbl_vec.push_back( std::stod( substr ) );
        }
    };

    Seldon::parse_comma_separated_list( str, callback );

    REQUIRE_THAT( dbl_vec, Catch::Matchers::RangeEquals( dbl_vec_expected ) );
    REQUIRE_THAT( int_vec, Catch::Matchers::RangeEquals( int_vec_expected ) );
}