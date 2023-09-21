#include "network.hpp"
#include <cstddef>
#include <iterator>
#include <random>
#include <set>

Seldon::Network::Network( int n_agents, int n_connections )
{
    // Loop through all the agents
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        connectionVectorT vec_i_agent;       // vector of tuples of (agent_idx,weight)
        std::set<int> j_agents;              // Set of agents connected to i
        std::vector<double> j_agent_weights; // Vector of weights
        double weight;                       // Weight of a particular j_agent
        // Distributions to draw from
        std::random_device rd;                                      // a seed source for the random number engine
        std::mt19937 gen( rd() );                                   // mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> distrib( 0, n_agents - 1 ); // for the j agent index
        std::uniform_real_distribution<> dis( 1.0, 2.0 );           // Values don't matter, will be normalized
        int max_iter     = 10000; // Maximum loop iterations to search for a unique connection
        bool agent_found = false;
        int j_agent_idx;
        double norm_weight = 0.0; // Do something else later? Sum of all weights per row

        // Add i_agent to j_agents
        j_agents.insert( i_agent ); //
        // Draw weight, for now vector is not sorted according to agent idx
        weight = dis( gen );
        j_agent_weights.push_back( weight );
        norm_weight += weight;

        // Maybe also make n_connections variable for each agent?

        // ---------
        // Draw the agent index, for n_connections-1
        // The agent itself is counted in n_connections
        // Here we will also draw the weights (I guess the order doesn't matter)
        // Loop through all connections
        agent_found = false;
        for( size_t j = 0; j < n_connections; ++j )
        {
            // Draw a new agent index
            for( size_t k = 0; k < max_iter; ++k )
            {
                j_agent_idx = distrib( gen ); // Draw an agent index
                // Check and see if it is inside the set
                auto it = j_agents.find( j_agent_idx );
                // Agent not found, break out
                if( it == j_agents.end() )
                {
                    agent_found = true;
                    break; // Break out of the loop
                }          // not foundiopok
                else
                {
                    continue;
                } // agent found, try again
            }

            // If a unique agent idx has not been found, skip this connection
            if( !agent_found )
            {
                continue;
            }

            // Now that a unique agent has been found, add to the set
            j_agents.insert( j_agent_idx );
            // Draw the weight for the agent
            weight = dis( gen );
            j_agent_weights.push_back( weight );
            norm_weight += weight;

        } // end of loop through all connections

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        // Also update the vector of tuples
        for( size_t j = 0; j < j_agents.size(); ++j )
        {
            weight    = j_agent_weights[j] / norm_weight;
            int j_idx = *std::next( j_agents.begin(), j );             // Accesses the j^th agent in set
            vec_i_agent.push_back( std::make_tuple( j_idx, weight ) ); // Update the vector
        }
        // ---------

        // Add the vector for i_agent to the adjacency list
        adjacency_list.push_back( vec_i_agent );

    } // end of loop through n_agents
}

void Seldon::Network::get_edges( std::size_t agent_idx, std::vector<std::tuple<std::size_t, double>> & buffer )
{
    // return ;
}