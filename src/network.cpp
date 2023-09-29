#include "network.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <set>

Seldon::Network::Network( size_t n_agents, size_t n_connections, std::optional<int> seed )
{
    initialize_rng( seed );

    // Distributions to draw from                              // mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis( 0.0, 1.0 ); // Values don't matter, will be normalized
    auto j_idx_buffer = std::vector<size_t>();        // for the j_agents indices connected to i_agent (adjacencies)
    connectionVectorT vec_i_agent;                    // vector of tuples of (agent_idx, weight)
    std::vector<double> j_agent_weights;              // Vector of weights

    // Loop through all the agents
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        vec_i_agent.clear();
        j_agent_weights.clear();

        double norm_weight = 0.0; // Do something else later? Sum of all weights per row

        // Get the vector of sorted adjacencies, including i
        // TODO: option for making the n_conections variable
        this->draw_unique_k_from_n( i_agent, n_connections, n_agents, j_idx_buffer );

        // Draw the weights, and calculate the normalizing factor
        j_agent_weights.resize( j_idx_buffer.size() );
        for( size_t j = 0; j < j_idx_buffer.size(); ++j )
        {
            j_agent_weights[j] = dis( gen ); // Draw the weight
            norm_weight += j_agent_weights[j];
        }

        // Put the self-interaction as the first entry
        auto self_interaction_weight = dis( gen );
        norm_weight += self_interaction_weight;
        self_interaction_weight /= norm_weight;
        vec_i_agent.push_back( std::make_tuple( i_agent, self_interaction_weight ) );

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        // Also update the vector of tuples
        for( size_t j = 0; j < j_idx_buffer.size(); ++j )
        {
            const size_t j_idx = j_idx_buffer[j]; // Accesses the j^th agent index
            vec_i_agent.push_back( std::make_tuple( j_idx, j_agent_weights[j] / norm_weight ) ); // Update the vector
        }
        // ---------

        // Add the vector for i_agent to the adjacency list
        adjacency_list.push_back( vec_i_agent );

    } // end of loop through n_agents
}

// Function for drawing k agents (indices), from n, without repitition
// Includes agent_idx of the i^th agent
void Seldon::Network::draw_unique_k_from_n(
    std::size_t agent_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer )
{
    struct SequenceGenerator
    {
        /* An iterator that generates a sequence of integers 2, 3, 4 ...*/
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = size_t;
        using pointer           = size_t *; // or also value_type*
        using reference         = size_t &;

        SequenceGenerator( size_t i ) : i( i ) {}
        size_t i;
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
            return *this;
        }
    };

    buffer.resize( k );
    std::sample( SequenceGenerator( 0 ), SequenceGenerator( n ), buffer.begin(), k, gen );
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