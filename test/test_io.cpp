#include "agent_generation.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "models/ActivityDrivenModel.hpp"
#include "network.hpp"
#include "network_generation.hpp"
#include "network_io.hpp"

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
    using AgentT  = ActivityDrivenModel::AgentT;
    using Network = Network<AgentT>;

    auto proj_root_path = fs::current_path();
    auto network_file   = proj_root_path / fs::path( "test/res/network.txt" );

    auto network = Seldon::NetworkGeneration::generate_from_file<AgentT>( network_file );

    REQUIRE( network.n_agents() == 3 );

    std::vector<std::vector<int>> neighbours_expected           = { { 2, 1 }, {}, { 1 } };
    std::vector<std::vector<Network::WeightT>> weights_expected = { { 0.1, -0.2 }, {}, { 1.2 } };

    for( size_t i = 0; i < network.n_agents(); i++ )
    {
        fmt::print( "{}", i );
        REQUIRE_THAT( neighbours_expected[i], Catch::Matchers::UnorderedRangeEquals( network.get_neighbours( i ) ) );
        REQUIRE_THAT( weights_expected[i], Catch::Matchers::UnorderedRangeEquals( network.get_weights( i ) ) );
    }
}

TEST_CASE( "Test writing the network to a file", "[io_network]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;
    using AgentT  = ActivityDrivenModel::AgentT;
    using Network = Network<AgentT>;

    std::vector<std::vector<size_t>> neighbours        = { { 3, 1 }, {}, { 1 } };
    std::vector<std::vector<Network::WeightT>> weights = { { -0.1, -0.5 }, {}, { -0.2 } };

    // Construct a network
    auto network = Network( neighbours, weights, Network::EdgeDirection::Incoming );

    // Save the network to a file
    auto proj_root_path = fs::current_path();
    auto network_file   = proj_root_path / fs::path( "test/network_out.txt" );
    network_to_file( network, network_file );

    // Read the network back in
    auto network_from_file = Seldon::NetworkGeneration::generate_from_file<AgentT>( network_file );

    for( size_t i = 0; i < network.n_agents(); i++ )
    {
        fmt::print( "{}", i );
        REQUIRE_THAT(
            network.get_neighbours( i ),
            Catch::Matchers::UnorderedRangeEquals( network_from_file.get_neighbours( i ) ) );
        REQUIRE_THAT(
            network.get_weights( i ), Catch::Matchers::UnorderedRangeEquals( network_from_file.get_weights( i ) ) );
    }
}

TEST_CASE( "Test reading in the agents from a file", "[io_agents]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto proj_root_path = fs::current_path();
    auto network_file   = proj_root_path / fs::path( "test/res/opinions.txt" );

    auto agents = Seldon::agents_from_file<ActivityDrivenModel::AgentT>( network_file );

    std::vector<double> opinions_expected    = { 2.1127107987061544, 0.8088982488089491, -0.8802809369462433 };
    std::vector<double> activities_expected  = { 0.044554683389757696, 0.015813166022685163, 0.015863953902810535 };
    std::vector<double> reluctances_expected = { 1.0, 1.0, 2.3 };

    REQUIRE( agents.size() == 3 );

    for( size_t i = 0; i < agents.size(); i++ )
    {
        fmt::print( "{}", i );
        REQUIRE_THAT( agents[i].data.opinion, Catch::Matchers::WithinAbs( opinions_expected[i], 1e-16 ) );
        REQUIRE_THAT( agents[i].data.activity, Catch::Matchers::WithinAbs( activities_expected[i], 1e-16 ) );
        REQUIRE_THAT( agents[i].data.reluctance, Catch::Matchers::WithinAbs( reluctances_expected[i], 1e-16 ) );
    }
}