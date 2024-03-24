#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "util/math.hpp"
#include <fmt/ostream.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
namespace fs = std::filesystem;

template<typename T, std::size_t N>
std::ostream & operator<<( std::ostream & os, std::array<T, N> const & v1 )
{
    std::for_each( begin( v1 ), end( v1 ), [&os]( T val ) { os << val << " "; } );
    return os;
}

// Samples the distribution n_samples times and writes results to file
template<typename distT>
void write_results_to_file( int N_Samples, distT dist, const std::string & filename )
{
    auto proj_root_path = fs::current_path();
    auto file           = proj_root_path / fs::path( "test/output_probability_distributions/" + filename );
    fmt::print( "file = {}\n", fmt::streamed( file ) );
    fs::create_directories( file.parent_path() );

    auto gen = std::mt19937( 0 );

    std::ofstream filestream( file );
    filestream << std::setprecision( 16 );

    for( int i = 0; i < N_Samples; i++ )
    {
        filestream << dist( gen ) << "\n";
    }
    filestream.close();
}

TEST_CASE( "Test the probability distributions", "[prob]" )
{
    write_results_to_file( 10000, Seldon::truncated_normal_distribution( 1.0, 0.5, 0.75 ), "truncated_normal.txt" );
    write_results_to_file( 10000, Seldon::power_law_distribution( 0.01, 2.1 ), "power_law.txt" );
    write_results_to_file( 10000, Seldon::bivariate_normal_distribution( 0.5 ), "bivariate_normal.txt" );
}

TEST_CASE( "Test bivariate gaussian copula", "[prob_copula]" )
{
    auto dist1  = Seldon::power_law_distribution( 0.02, 2.5 );
    auto dist2  = Seldon::truncated_normal_distribution( 1.0, 0.75, 0.2 );
    auto copula = Seldon::bivariate_gaussian_copula( 0.5, dist1, dist2 );
    write_results_to_file( 10000, copula, "gaussian_copula.txt" );
}
