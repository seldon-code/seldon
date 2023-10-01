#pragma once
#include "network.hpp"
#include <algorithm>
#include <iterator>
#include <memory>
#include <random>

namespace Seldon
{
using WeightT = double;

// Function for getting a vector of k agents (corresponding to connections)
// drawing from n agents (without duplication)
// ignore_idx ignores the index of the agent itself, since we will later add the agent itself ourselves to prevent duplication
inline void draw_unique_k_from_n(
    std::size_t ignore_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer, std::mt19937 & gen )
{
    struct SequenceGenerator
    {
        /* An iterator that generates a sequence of integers 2, 3, 4 ...*/
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = size_t;
        using pointer           = size_t *; // or also value_type*
        using reference         = size_t &;

        SequenceGenerator( const size_t i_, const size_t ignore_idx ) : i( i_ ), ignore_idx( ignore_idx )
        {
            if( i == ignore_idx )
            {
                i++;
            }
        }
        size_t i;
        size_t ignore_idx;

        size_t & operator*()
        {
            return i;
        };
        bool operator==( const SequenceGenerator & it1 )
        {
            return i == it1.i;
        };
        SequenceGenerator & operator++()
        {
            i++;
            if( i == ignore_idx )
                i++;
            return *this;
        }
    };

    buffer.resize( k );
    std::sample( SequenceGenerator( 0, ignore_idx ), SequenceGenerator( n, ignore_idx ), buffer.begin(), k, gen );
}

// Returns a unique pointer to a new network with n_connections per agent
inline std::unique_ptr<Network> generate_n_connections( int n_agents, int n_connections, std::mt19937 & gen )
{
    std::vector<std::vector<size_t>> neighbour_list;  // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;    // List for the interaction weights of each connection
    std::uniform_real_distribution<> dis( 0.0, 1.0 ); // Values don't matter, will be normalized
    auto incoming_neighbour_buffer
        = std::vector<size_t>(); // for the j_agents indices connected to i_agent (adjacencies/neighbours)
    auto incoming_neighbour_weights = std::vector<double>(); // Vector of weights of the j neighbours of i
    double outgoing_norm_weight     = 0;

    // Loop through all the agents and create the neighbour_list and weight_list
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        outgoing_norm_weight = 0.0;

        incoming_neighbour_buffer.clear();
        incoming_neighbour_weights.clear();

        // Get the vector of sorted adjacencies, excluding i (include i later)
        // TODO: option for making the n_conections variable
        draw_unique_k_from_n( i_agent, n_connections, n_agents, incoming_neighbour_buffer, gen );

        incoming_neighbour_weights.resize( incoming_neighbour_buffer.size() );
        for( size_t j = 0; j < incoming_neighbour_buffer.size(); ++j )
        {
            incoming_neighbour_weights[j] = dis( gen ); // Draw the weight
            outgoing_norm_weight += incoming_neighbour_weights[j];
        }

        // Put the self-interaction as the last entry
        auto self_interaction_weight = dis( gen );
        outgoing_norm_weight += self_interaction_weight;

        // outgoing_norm_weights += self_interaction_weight;
        incoming_neighbour_buffer.push_back( i_agent ); // Add the agent itself
        incoming_neighbour_weights.push_back( self_interaction_weight );

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        for( size_t j = 0; j < incoming_neighbour_buffer.size(); ++j )
        {
            incoming_neighbour_weights[j] /= outgoing_norm_weight;
        }

        // Add the neighbour vector for i_agent to the neighbour list
        neighbour_list.push_back( incoming_neighbour_buffer );
        // Add the weight interactions for the neighbours of i_agent
        weight_list.push_back( incoming_neighbour_weights );

    } // end of loop through n_agents

    return std::make_unique<Network>( std::move( neighbour_list ), std::move( weight_list ) );
}

} // namespace Seldon