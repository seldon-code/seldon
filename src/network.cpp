#include "network.hpp"
#include "connectivity.hpp"
#include <fmt/format.h>
#include <cstddef>
#include <optional>
#include <stdexcept>

Seldon::Network::Network(
    std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list )
        : neighbour_list( neighbour_list ), weight_list( weight_list )
{
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

size_t Seldon::Network::n_agents() const
{
    return neighbour_list.size();
}

void Seldon::Network::get_neighbours( std::size_t agent_idx, std::vector<size_t> & buffer ) const
{
    buffer = neighbour_list[agent_idx];
}

void Seldon::Network::set_neighbours_and_weights(
    std::size_t agent_idx, const std::vector<size_t> & buffer_neighbours, const WeightT & weight )
{
    neighbour_list[agent_idx] = buffer_neighbours;

    weight_list[agent_idx].resize( buffer_neighbours.size() );
    for( auto & w : weight_list[agent_idx] )
    {
        w = weight;
    }
}

void Seldon::Network::set_neighbours_and_weights(
    std::size_t agent_idx, const std::vector<size_t> & buffer_neighbours,
    const std::vector<Seldon::Network::WeightT> & buffer_weights )
{
    if( buffer_neighbours.size() != buffer_weights.size() )
        [[unlikely]]
        {
            throw std::runtime_error(
                "Network::set_neighbours_and_weights: both buffers need to have the same length!" );
        }

    neighbour_list[agent_idx] = buffer_neighbours;
    weight_list[agent_idx]    = buffer_weights;
}

void Seldon::Network::push_back_neighbour_and_weight( size_t i, size_t j, WeightT w )
{
    neighbour_list[i].push_back( j );
    weight_list[i].push_back( w );
}

void Seldon::Network::get_weights( std::size_t agent_idx, std::vector<Seldon::Network::WeightT> & buffer ) const
{
    buffer = weight_list[agent_idx];
}

void Seldon::Network::transpose()
{
    std::vector<std::vector<size_t>> neighbour_list_transpose( n_agents(), std::vector<size_t>( 0 ) );
    std::vector<std::vector<WeightT>> weight_list_transpose( n_agents(), std::vector<WeightT>( 0 ) );

    for( size_t i_agent = 0; i_agent < n_agents(); i_agent++ )
    {
        for( size_t i_neighbour = 0; i_neighbour < neighbour_list[i_agent].size(); i_neighbour++ )
        {
            const auto neighbour = neighbour_list[i_agent][i_neighbour];
            const auto weight    = weight_list[i_agent][i_neighbour];
            neighbour_list_transpose[neighbour].push_back( i_agent );
            weight_list_transpose[neighbour].push_back( weight );
        }
    }

    neighbour_list = std::move( neighbour_list_transpose );
    weight_list    = std::move( weight_list_transpose );
}