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

// TEST_CASE( "Test reading in the agents from a file", "[io_agents]" )
// {
//     using namespace Seldon;
//     using namespace Catch::Matchers;

//     auto proj_root_path = fs::current_path();
//     auto network_file   = proj_root_path / fs::path( "test/res/opinions.txt" );

//     auto agents = Seldon::agents_from_file<ActivityDrivenModel::AgentT>( network_file );

//     std::vector<double> opinions_expected    = { 2.1127107987061544, 0.8088982488089491, -0.8802809369462433 };
//     std::vector<double> activities_expected  = { 0.044554683389757696, 0.015813166022685163, 0.015863953902810535 };
//     std::vector<double> reluctances_expected = { 1.0, 1.0, 2.3 };

//     REQUIRE( agents.size() == 3 );

//     for( size_t i = 0; i < agents.size(); i++ )
//     {
//         fmt::print( "{}", i );
//         REQUIRE_THAT( agents[i].data.opinion, Catch::Matchers::WithinAbs( opinions_expected[i], 1e-16 ) );
//         REQUIRE_THAT( agents[i].data.activity, Catch::Matchers::WithinAbs( activities_expected[i], 1e-16 ) );
//         REQUIRE_THAT( agents[i].data.reluctance, Catch::Matchers::WithinAbs( reluctances_expected[i], 1e-16 ) );
//     }
// }