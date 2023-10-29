#include "network.hpp"
#include "network_generation.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <cstddef>
#include <random>
#include <set>

TEST_CASE( "Testing the network generation functions" )
{
    using namespace Seldon;
    using WeightT = Network::WeightT;

    std::vector<size_t> buffer_n_get{};  // buffer for getting neighbours
    std::vector<WeightT> buffer_w_get{}; // buffer for getting the weights

    SECTION( "Checking network generation for a fully-connected network" )
    {
        // Generate a fully-connected network
        const size_t n_agents = 3;
        std::vector<size_t> neigh{ { 0, 1, 2 } }; // Neighbours of fully-connected network
        SECTION( "Fully-connected network initialized with a constant weight" )
        {
            WeightT weight = 0.25;
            std::vector<WeightT> weights{ weight, weight, weight }; // Weights to set to
            auto network = generate_fully_connected( n_agents, weight );
            // Make sure that the network has been generated correctly
            REQUIRE( network->n_agents() == n_agents ); // There should be n_agents in the new network

            // All neighbours should be equal to neigh
            for( size_t i = 0; i < n_agents; ++i )
            {
                network->get_neighbours( i, buffer_n_get );
                network->get_weights( i, buffer_w_get );
                REQUIRE_THAT( buffer_n_get, Catch::Matchers::UnorderedRangeEquals( neigh ) );
                REQUIRE_THAT( buffer_w_get, Catch::Matchers::UnorderedRangeEquals( weights ) );
            }
        }
    }
}