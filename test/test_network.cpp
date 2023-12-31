#include "network.hpp"
#include "network_generation.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <cstddef>
#include <random>
#include <set>

TEST_CASE( "Testing the network class" )
{
    using namespace Seldon;

    // Generate some network
    const int n_agents      = 20;
    const int n_connections = 10;
    std::mt19937 gen( 0 );
    auto network = generate_n_connections( n_agents, n_connections, gen );

    // Does n_agents work?
    REQUIRE( network->n_agents() == n_agents );

    // Check that the function for setting neighbours and a single weight work
    // Agent 3
    std::vector<size_t> buffer_n_get{};                       // buffer for getting neighbours
    std::vector<Seldon::Network::WeightT> buffer_w_get{};     // buffer for getting the weights
    std::vector<size_t> neigh{ { 0, 10 } };                   // new neighbours
    std::vector<Seldon::Network::WeightT> weight{ 0.5, 0.5 }; // new weights (const)
    network->set_neighbours_and_weights( 3, neigh, 0.5 );
    network->get_weights( 3, buffer_w_get );
    REQUIRE_THAT( weight, Catch::Matchers::UnorderedRangeEquals( buffer_w_get ) );

    // Change the connections for agent 3
    std::vector<size_t> buffer_n{ { 0, 10, 15 } };                   // new neighbours
    std::vector<Seldon::Network::WeightT> buffer_w{ 0.1, 0.2, 0.3 }; // new weights
    network->set_neighbours_and_weights( 3, buffer_n, buffer_w );

    // Make sure the changes worked
    network->get_neighbours( 3, buffer_n_get );
    network->get_weights( 3, buffer_w_get );

    REQUIRE_THAT( buffer_n_get, Catch::Matchers::UnorderedRangeEquals( buffer_n ) );
    REQUIRE_THAT( buffer_w_get, Catch::Matchers::UnorderedRangeEquals( buffer_w ) );

    // Check that the push_back function works for agent 3
    buffer_n.push_back( 5 );                              // new neighbour
    buffer_w.push_back( 1.0 );                            // new weight for this new connection
    network->push_back_neighbour_and_weight( 3, 5, 1.0 ); // new connection added with weight
    // Check that the change worked for the push_back function
    network->get_neighbours( 3, buffer_n_get );
    network->get_weights( 3, buffer_w_get );
    REQUIRE_THAT( buffer_n_get, Catch::Matchers::UnorderedRangeEquals( buffer_n ) );
    REQUIRE_THAT( buffer_w_get, Catch::Matchers::UnorderedRangeEquals( buffer_w ) );

    // Now we test the transpose() function

    // First record all the old edges as tuples (i,j,w) where this edge goes from j -> i with weight w
    std::set<std::tuple<size_t, size_t, Network::WeightT>> old_edges;
    for( size_t i_agent = 0; i_agent < network->n_agents(); i_agent++ )
    {
        network->get_neighbours( i_agent, buffer_n );
        network->get_weights( i_agent, buffer_w );

        for( size_t i_neighbour = 0; i_neighbour < buffer_n.size(); i_neighbour++ )
        {
            auto neighbour = buffer_n[i_neighbour];
            auto weight    = buffer_w[i_neighbour];
            std::tuple<size_t, size_t, Network::WeightT> edge{ i_agent, neighbour, weight };
            old_edges.insert( edge );
        }
    }

    network->transpose();

    // Now we go over the transposed network and try to re-identify all edges
    for( size_t i_agent = 0; i_agent < network->n_agents(); i_agent++ )
    {
        network->get_neighbours( i_agent, buffer_n );
        network->get_weights( i_agent, buffer_w );

        for( size_t i_neighbour = 0; i_neighbour < buffer_n.size(); i_neighbour++ )
        {
            auto neighbour = buffer_n[i_neighbour];
            auto weight    = buffer_w[i_neighbour];
            std::tuple<size_t, size_t, Network::WeightT> edge{
                neighbour, i_agent, weight
            };                                     // Note that i_agent and neighbour are flipped compared to before
            REQUIRE( old_edges.contains( edge ) ); // can we find the transposed edge?
            old_edges.extract( edge );             // extract the edge afterwards
        }
    }

    REQUIRE( old_edges.empty() );
}