#include "network.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <set>

Seldon::Network::Network( size_t n_agents, size_t n_connections, std::optional<int> seed )
        : neighbour_list( std::vector<std::vector<size_t>>( 0 ) ), weight_list( std::vector<std::vector<double>>( 0 ) )
{
    initialize_rng( seed );

    // Distributions to draw from                              // mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis( 0.0, 1.0 ); // Values don't matter, will be normalized
    auto j_idx_buffer = std::vector<size_t>(); // for the j_agents indices connected to i_agent (adjacencies/neighbours)
    auto j_agent_weights = std::vector<double>(); // Vector of weights of the j neighbours of i
    double norm_weight   = 0.0;                   // Factor for normalizing the weights so that the row sums to 1.0

    // Loop through all the agents
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        j_idx_buffer.clear();
        j_agent_weights.clear();

        norm_weight = 0.0; // Sum of all weights per row

        // Get the vector of sorted adjacencies, excluding i (include i later)
        // TODO: option for making the n_conections variable
        this->draw_unique_k_from_n( i_agent, n_connections, n_agents, j_idx_buffer );

        // Draw the weights, and calculate the normalizing factor
        j_agent_weights.resize( j_idx_buffer.size() );
        for( size_t j = 0; j < j_idx_buffer.size(); ++j )
        {
            j_agent_weights[j] = dis( gen ); // Draw the weight
            norm_weight += j_agent_weights[j];
        }

        // Put the self-interaction as the last entry
        auto self_interaction_weight = dis( gen );
        norm_weight += self_interaction_weight;
        j_idx_buffer.push_back( i_agent ); // Add the agent itself
        j_agent_weights.push_back( self_interaction_weight );

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        for( size_t j = 0; j < j_idx_buffer.size(); ++j )
        {
            j_agent_weights[j] /= norm_weight;
        }
        // ---------

        // Add the neighbour vector for i_agent to the neighbour list
        neighbour_list.push_back( j_idx_buffer );
        // Add the weight interactions for the neighbours of i_agent
        weight_list.push_back( j_agent_weights );

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
    const size_t n_edges = neighbour_list[agent_idx].size();
    buffer.resize( n_edges );
    for( size_t i_edge = 0; i_edge < n_edges; i_edge++ )
    {
        buffer[i_edge] = neighbour_list[agent_idx][i_edge];
    }
}

void Seldon::Network::get_weights( std::size_t agent_idx, std::vector<double> & buffer ) const
{
    // TODO: rewrite this using std::span
    const size_t n_edges = weight_list[agent_idx].size();
    buffer.resize( n_edges );
    for( size_t i_edge = 0; i_edge < n_edges; i_edge++ )
    {
        buffer[i_edge] = weight_list[agent_idx][i_edge];
    }
}