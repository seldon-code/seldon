#include "util/math.hpp"
#include <fmt/format.h>
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cstddef>
#include <random>
#include <set>
#include <vector>

double compute_p( size_t k, size_t n )
{
    if( k == 0 )
    {
        return 0.0;
    }
    else
    {
        double p = 1.0 / ( double( n ) - 1.0 );
        return p + ( 1.0 - p ) * compute_p( k - 1, n - 1 );
    }
}

TEST_CASE( "Testing sampling functions" )
{
    std::random_device rd;
    std::mt19937 gen( rd() );

    SECTION( "draw_unique_k_from_n", "Drawing k numbers out of n" )
    {

        const size_t N_RUNS = 10000;

        const size_t k          = 6;
        const size_t n          = 100;
        const size_t ignore_idx = 11;

        std::vector<size_t> histogram( n, 0 ); // Count how often each element occurs amongst all samples

        std::vector<size_t> buffer{};
        for( size_t i = 0; i < N_RUNS; i++ )
        {
            Seldon::draw_unique_k_from_n( ignore_idx, k, n, buffer, gen );
            for( const auto & n : buffer )
            {
                histogram[n]++;
            }
        }

        // In each run there is a probability of p for each element to be selected
        // That means for each histogram bin we have a binomial distribution with p
        double p = compute_p( k, n );

        size_t mean = N_RUNS * p;
        // The variance of a binomial distribution is var = n*p*(1-p)
        size_t sigma = std::sqrt( N_RUNS * p * ( 1.0 - p ) );

        INFO( "Binomial distribution parameters" );
        INFO( fmt::format( " p = {}", p ) );
        INFO( fmt::format( " mean = {}", mean ) );
        INFO( fmt::format( " sigma = {}", sigma ) );

        REQUIRE( histogram[ignore_idx] == 0 ); // The ignore_idx should never be selected

        size_t number_outside_three_sigma = 0;
        for( const auto & n : histogram )
        {
            if( n == 0 )
            {
                continue;
            }
            INFO( fmt::format( " n = {}", n ) );
            INFO( fmt::format( " mean = {}", mean ) );
            INFO( fmt::format( " sigma = {}", sigma ) );

            if( std::abs( double( n ) - double( mean ) ) > 3.0 * sigma )
            {
                number_outside_three_sigma++;
            }

            REQUIRE_THAT( n, Catch::Matchers::WithinAbs( mean, 5 * sigma ) );
        }

        if( number_outside_three_sigma > 0.01 * N_RUNS )
            WARN( fmt::format(
                "Many deviations beyond the 3 sigma range. {} out of {}", number_outside_three_sigma, N_RUNS ) );
    }

    SECTION( "weighted_reservior_sampling", "Testing weighted reservoir sampling with A_ExpJ algorithm" )
    {

        const size_t N_RUNS = 10000;

        const size_t k           = 6;
        const size_t n           = 100;
        const size_t ignore_idx  = 11;
        const size_t ignore_idx2 = 29;

        std::vector<size_t> histogram( n, 0 ); // Count how often each element occurs amongst all samples

        auto weight_callback = []( size_t idx )
        {
            if( ( idx == ignore_idx ) || ( idx == ignore_idx2 ) )
            {
                return 0.0;
            }
            else
            {
                return std::abs( double( n / 2.0 ) - double( idx ) );
            }
        };

        std::vector<size_t> buffer{};

        for( size_t i = 0; i < N_RUNS; i++ )
        {
            Seldon::reservoir_sampling_A_ExpJ( k, n, weight_callback, buffer, gen );
            for( const auto & n : buffer )
            {
                histogram[n]++;
            }
        }

        REQUIRE( histogram[ignore_idx] == 0 );  // The ignore_idx should never be selected
        REQUIRE( histogram[ignore_idx2] == 0 ); // The ignore_idx should never be selected

        // TODO: histogram and sigma test
    }
}