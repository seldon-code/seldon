#include "catch2/matchers/catch_matchers.hpp"
#include "network.hpp"
#include "network_generation.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <config_parser.hpp>
#include <filesystem>
#include <simulation.hpp>
namespace fs = std::filesystem;

TEST_CASE( "Test reading in the network from a file", "[io_network]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto proj_root_path = fs::current_path();
    auto network_file   = proj_root_path / fs::path( "test/res/network.txt" );

    auto network = Seldon::NetworkGeneration::generate_from_file( network_file );

    REQUIRE( network->n_agents() == 3 );

    std::vector<std::vector<int>> neighbours_expected           = { { 2, 1 }, {}, { 1 } };
    std::vector<std::vector<Network::WeightT>> weights_expected = { { 0.1, -0.2 }, {}, { 1.2 } };

    for( size_t i = 0; i < network->n_agents(); i++ )
    {
        fmt::print( "{}", i );
        REQUIRE_THAT( neighbours_expected[i], Catch::Matchers::UnorderedRangeEquals( network->get_neighbours( i ) ) );
        REQUIRE_THAT( weights_expected[i], Catch::Matchers::UnorderedRangeEquals( network->get_weights( i ) ) );
    }
}