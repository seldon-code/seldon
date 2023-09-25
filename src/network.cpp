#include "network.hpp"
#include <cstddef>
#include <iterator>
#include <set>

Seldon::Network::Network( size_t n_agents, size_t n_connections ) // : gen( 0 )
{
    std::random_device rd;    // a seed source for the random number engine
    std::mt19937 gen( rd() ); // mersenne_twister_engine seeded with rd()
    // Distributions to draw from                              // mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis( 1.0, 2.0 ); // Values don't matter, will be normalized
    auto j_idx_buffer = std::vector<size_t>();        // for the j_agents indices connected to i_agent (adjacencies)

    // Loop through all the agents
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        connectionVectorT vec_i_agent;       // vector of tuples of (agent_idx,weight)
        std::vector<double> j_agent_weights; // Vector of weights
        double weight;                       // Weight of a particular j_agent
        double norm_weight = 0.0;            // Do something else later? Sum of all weights per row

        // Get the vector of sorted adjacencies, including i
        // TODO: option for making the n_conections variable
        this->draw_unique_k_from_n( i_agent, n_connections, n_agents, j_idx_buffer );

        // Draw the weights, and calculate the normalizing factor
        j_agent_weights.resize( j_idx_buffer.size() );
        for( size_t j = 0; j < j_idx_buffer.size(); ++j )
        {
            weight             = dis( gen ); // Draw the weight
            j_agent_weights[j] = weight;     // Update the weight vector
            norm_weight += weight;
        }

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        // Also update the vector of tuples
        for( size_t j = 0; j < j_idx_buffer.size(); ++j )
        {
            weight       = j_agent_weights[j] / norm_weight;
            size_t j_idx = j_idx_buffer[j];                            // Accesses the j^th agent index
            vec_i_agent.push_back( std::make_tuple( j_idx, weight ) ); // Update the vector
        }
        // ---------

        // Add the vector for i_agent to the adjacency list
        adjacency_list.push_back( vec_i_agent );

    } // end of loop through n_agents
}

// Function for drawing k agents (indices), from n, without repitition
// Includes agent_idx of the i^th agent
void Seldon::Network::draw_unique_k_from_n(
    std::size_t agent_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer ) const
{
    std::random_device rd;    // a seed source for the random number engine
    std::mt19937 gen( rd() ); // mersenne_twister_engine seeded with rd()
    // Distributions to draw from
    std::uniform_int_distribution<> distrib( 0, n - 1 ); // for the j agent index
    std::set<std::size_t> j_agents;                      // Set of agents connected to i (agent_idx)
    size_t max_iter          = 10000;                    // Maximum loop iterations to search for a unique connection
    bool agent_found         = false; // Checks whether a j_agent has been found, which is not a duplicate
    std::size_t number_drawn = 0;     // Number of agents drawn, should be k or less
    std::size_t j_agent_idx;          // Agent index drawn

    // Add the agent itself
    j_agents.insert( agent_idx ); //

    agent_found = false;
    // Loop through all n
    for( size_t j = 0; j < k; ++j )
    {
        // Draw a new agent index
        for( size_t itr = 0; itr < max_iter; ++itr )
        {
            j_agent_idx = distrib( gen ); // Draw an agent index
            // Check and see if the agent drawn is inside the set
            if( j_agents.insert( j_agent_idx ).second == false )
            {
                continue; // try drawing j_agent_idx again
            }             // duplicate agent found
            else
            {
                agent_found = true;
                break;
            } // duplicate agent not found, break out of loop
        }     // end of agent draw

        // If a unique agent idx has not been found in max_itr, skip this connection
        if( !agent_found )
        {
            continue;
        }
    } // end of n draws

    // Update the number of connections drawn
    number_drawn = j_agents.size();

    buffer.resize( number_drawn ); // Resize the output buffer

    // Update the vector with the sorted values in the set:
    for( size_t j = 0; j < number_drawn; ++j )
    {
        int j_idx = *std::next( j_agents.begin(), j ); // Accesses the j^th agent in set
        buffer[j] = j_idx;
    }
}

void Seldon::Network::get_adjacencies( std::size_t agent_idx, std::vector<size_t> & buffer ) const
{
    // TODO: rewrite this using std::span
    const size_t n_edges = adjacency_list[agent_idx].size();
    buffer.resize( n_edges );
    for( size_t i_edge = 0; i_edge < n_edges; i_edge++ )
    {
        buffer[i_edge] = std::get<0>( adjacency_list[agent_idx][i_edge] );
    }
}

void Seldon::Network::get_edges( std::size_t agent_idx, Seldon::Network::connectionVectorT & buffer ) const
{
    // TODO: rewrite this using std::span
    const size_t n_edges = adjacency_list[agent_idx].size();
    buffer.resize( n_edges );
    for( size_t i_edge = 0; i_edge < n_edges; i_edge++ )
    {
        buffer[i_edge] = adjacency_list[agent_idx][i_edge];
    }
}