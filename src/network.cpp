#include "network.hpp"
#include "connectivity.hpp"
#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <stdexcept>

Seldon::Network::Network(
    std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list,
    EdgeDirection direction )
        : neighbour_list( neighbour_list ), weight_list( weight_list ), _direction( direction )
{
}

size_t Seldon::Network::n_agents() const
{
    return neighbour_list.size();
}

std::size_t Seldon::Network::n_edges( std::optional<std::size_t> agent_idx ) const
{
    if( agent_idx.has_value() )
    {
        return neighbour_list[agent_idx.value()].size();
    }
    else
    {
        return std::transform_reduce(
            neighbour_list.cbegin(), neighbour_list.cend(), 0, std::plus{},
            []( const auto & neigh_list ) { return neigh_list.size(); } );
    }
}

const Seldon::Network::EdgeDirection & Seldon::Network::direction() const
{
    return _direction;
}

std::vector<std::vector<size_t>> Seldon::Network::strongly_connected_components() const
{
    // Now that we have the neighbour list (or adjacency list)
    // Run Tarjan's algorithm for strongly connected components
    auto tarjan_scc = TarjanConnectivityAlgo( neighbour_list );
    return tarjan_scc.scc_list;
}

std::span<const size_t> Seldon::Network::get_neighbours( std::size_t agent_idx ) const
{
    return std::span( neighbour_list[agent_idx].cbegin(), neighbour_list[agent_idx].cend() );
}

std::span<size_t> Seldon::Network::get_neighbours( std::size_t agent_idx )
{
    return std::span( neighbour_list[agent_idx].begin(), neighbour_list[agent_idx].end() );
}

void Seldon::Network::set_neighbours_and_weights(
    std::size_t agent_idx, std::span<const size_t> buffer_neighbours, const WeightT & weight )
{
    neighbour_list[agent_idx].assign( buffer_neighbours.begin(), buffer_neighbours.end() );
    weight_list[agent_idx].resize( buffer_neighbours.size() );
    std::fill( weight_list[agent_idx].begin(), weight_list[agent_idx].end(), weight );
}

void Seldon::Network::set_neighbours_and_weights(
    std::size_t agent_idx, std::span<const size_t> buffer_neighbours,
    std::span<const Seldon::Network::WeightT> buffer_weights )
{
    if( buffer_neighbours.size() != buffer_weights.size() )
    {
        throw std::runtime_error( "Network::set_neighbours_and_weights: both buffers need to have the same length!" );
    }

    neighbour_list[agent_idx].assign( buffer_neighbours.begin(), buffer_neighbours.end() );
    weight_list[agent_idx].assign( buffer_weights.begin(), buffer_weights.end() );
}

void Seldon::Network::push_back_neighbour_and_weight( size_t i, size_t j, WeightT w )
{
    neighbour_list[i].push_back( j );
    weight_list[i].push_back( w );
}

std::span<const Seldon::Network::WeightT> Seldon::Network::get_weights( std::size_t agent_idx ) const
{
    return std::span<const Seldon::Network::WeightT>( weight_list[agent_idx].cbegin(), weight_list[agent_idx].cend() );
}

std::span<Seldon::Network::WeightT> Seldon::Network::get_weights( std::size_t agent_idx )
{
    return std::span<Seldon::Network::WeightT>( weight_list[agent_idx].begin(), weight_list[agent_idx].end() );
}

void Seldon::Network::set_weights( std::size_t agent_idx, std::span<const Seldon::Network::WeightT> weights )
{
    if( neighbour_list[agent_idx].size() != weights.size() )
    {
        throw std::runtime_error( "Network::set_weights: tried to set weights of the wrong size!" );
    }
    weight_list[agent_idx].assign( weights.begin(), weights.end() );
}

void Seldon::Network::toggle_direction()
{
    // Swap the edge direction
    if( direction() == EdgeDirection::Incoming )
    {
        _direction = EdgeDirection::Outgoing;
    }
    else
    {
        _direction = EdgeDirection::Incoming;
    }
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

    toggle_direction();
    neighbour_list = std::move( neighbour_list_transpose );
    weight_list    = std::move( weight_list_transpose );
}