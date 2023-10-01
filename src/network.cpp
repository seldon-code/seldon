#include "network.hpp"
#include "connectivity.hpp"
#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <set>

Seldon::Network::Network(
    std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list,
    std::mt19937 & _gen )
        : gen( &_gen ), neighbour_list( neighbour_list ), weight_list( weight_list )
{
}

Seldon::Network::Network( size_t n_agents, size_t n_connections, std::mt19937 & _gen )
        : gen( &_gen ),
          neighbour_list( std::vector<std::vector<size_t>>( 0 ) ),
          weight_list( std::vector<std::vector<double>>( 0 ) )
{
    // Distributions to draw from                              // mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis( 0.0, 1.0 ); // Values don't matter, will be normalized
    auto incoming_neighbour_buffer
        = std::vector<size_t>(); // for the j_agents indices connected to i_agent (adjacencies/neighbours)
    auto incoming_neighbour_weights = std::vector<double>(); // Vector of weights of the j neighbours of i
    double outgoing_norm_weight     = 0;

    // Loop through all the agents
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        outgoing_norm_weight = 0.0;

        incoming_neighbour_buffer.clear();
        incoming_neighbour_weights.clear();

        // Get the vector of sorted adjacencies, excluding i (include i later)
        // TODO: option for making the n_conections variable
        this->draw_unique_k_from_n( i_agent, n_connections, n_agents, incoming_neighbour_buffer );

        incoming_neighbour_weights.resize( incoming_neighbour_buffer.size() );
        for( size_t j = 0; j < incoming_neighbour_buffer.size(); ++j )
        {
            incoming_neighbour_weights[j] = dis( *gen ); // Draw the weight
            outgoing_norm_weight += incoming_neighbour_weights[j];
        }

        // Put the self-interaction as the last entry
        auto self_interaction_weight = dis( *gen );
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

    // Now that we have the neighbour list (or adjacency list)
    // Run Tarjan's algorithm for strongly connected components
    auto tarjan_scc = TarjanConnectivityAlgo( neighbour_list );

    // For a strongly connected network, the number of SCCs should be 1
    // Print a warning if this is not true
    if( tarjan_scc.scc_list.size() != 1 )
    {
        fmt::print(
            "WARNING: You have {} strongly connected components in your network!\n", tarjan_scc.scc_list.size() );
    }
}

// Function for drawing k agents (indices), from n, without repitition
void Seldon::Network::draw_unique_k_from_n(
    std::size_t ignore_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer )
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
    std::sample( SequenceGenerator( 0, ignore_idx ), SequenceGenerator( n, ignore_idx ), buffer.begin(), k, *gen );
}

size_t Seldon::Network::n_agents() const
{
    return neighbour_list.size();
}

void Seldon::Network::get_neighbours( std::size_t agent_idx, std::vector<size_t> & buffer ) const
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