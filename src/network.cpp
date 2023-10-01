#include "network.hpp"
#include "connectivity.hpp"
#include <fmt/format.h>
#include <cstddef>
#include <optional>

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