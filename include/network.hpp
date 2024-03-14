#pragma once
#include "connectivity.hpp"
#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace Seldon
{

/*
    A class that represents a directed graph using adjacency lists.
    Either incoming or outgoing edges are stored.
    To switch between the different representations (in/out)
    and the network and its transpose (N/N^T), refer to the following table:

              | N (inc)   |  N (out)  |  N^T (inc) | N^T (out) |
    ------------------------------------------------------------
    N (inc)   |     X     |  toggle   |  transpose |   switch  |
    N (out)   |  toggle   |     X     |  switch    | transpose |
    N^T (inc) | transpose |   switch  |      X     |  toggle   |
    N^T (out) |   switch  | transpose |   toggle   |     X     |

    Note: switch is equivalent to toggle + transpose, but much cheaper!
*/
template<typename AgentType>
class Network
{
public:
    enum class EdgeDirection
    {
        Incoming,
        Outgoing
    };

    using WeightT = double;
    using AgentT  = AgentType;

    Network(
        std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list,
        EdgeDirection direction )
            : neighbour_list( neighbour_list ),
              weight_list( weight_list ),
              _direction( direction ),
              agents( std::vector<AgentT>( neighbour_list.size() ) )
    {
    }

    /*
    Gives the total number of nodes in the network
    */
    [[nodiscard]] std::size_t n_agents() const
    {
        return neighbour_list.size();
    }

    /*
    Gives the number of edges going out/coming in at agent_idx, depending on the value of direction().
    If agent_idx is nullopt, gives the total number of edges
    */
    [[nodiscard]] std::size_t n_edges( std::optional<std::size_t> agent_idx = std::nullopt ) const
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

    /*
    Returns the current directionality of the adjacency list. That is if incoming or outgoing edges are stored.
    */
    [[nodiscard]] const EdgeDirection & direction() const
    {
        return _direction;
    }

    /*
    Gives the strongly connected components in the graph
    */
    [[nodiscard]] std::vector<std::vector<size_t>> strongly_connected_components() const
    {
        // Now that we have the neighbour list (or adjacency list)
        // Run Tarjan's algorithm for strongly connected components
        auto tarjan_scc = TarjanConnectivityAlgo( neighbour_list );
        return tarjan_scc.scc_list;
    }

    /*
        Gives a view into the neighbour indices going out/coming in at agent_idx
    */
    [[nodiscard]] std::span<const size_t> get_neighbours( std::size_t agent_idx ) const
    {
        return std::span( neighbour_list[agent_idx].data(), neighbour_list[agent_idx].size() );
    }
    [[nodiscard]] std::span<size_t> get_neighbours( std::size_t agent_idx )
    {
        return std::span( neighbour_list[agent_idx].data(), neighbour_list[agent_idx].size() );
    }

    /*
    Gives a view into the edge weights going out/coming in at agent_idx
    */
    [[nodiscard]] std::span<const WeightT> get_weights( std::size_t agent_idx ) const
    {
        return std::span<const WeightT>( weight_list[agent_idx].data(), weight_list[agent_idx].size() );
    }
    [[nodiscard]] std::span<WeightT> get_weights( std::size_t agent_idx )
    {
        return std::span<WeightT>( weight_list[agent_idx].data(), weight_list[agent_idx].size() );
    }

    /*
    Gives a view into the edge weights going out/coming in at agent_idx
    */
    void set_weights( std::size_t agent_idx, std::span<const WeightT> weights )
    {
        if( neighbour_list[agent_idx].size() != weights.size() )
        {
            throw std::runtime_error( "Network::set_weights: tried to set weights of the wrong size!" );
        }
        weight_list[agent_idx].assign( weights.begin(), weights.end() );
    }

    /*
    Sets the neighbour indices and sets the weight to a constant value at agent_idx
    */
    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, const WeightT & weight )
    {
        neighbour_list[agent_idx].assign( buffer_neighbours.begin(), buffer_neighbours.end() );
        weight_list[agent_idx].resize( buffer_neighbours.size() );
        std::fill( weight_list[agent_idx].begin(), weight_list[agent_idx].end(), weight );
    }
    /*
    Sets the neighbour indices and weights at agent_idx
    */
    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, std::span<const WeightT> buffer_weights )
    {
        if( buffer_neighbours.size() != buffer_weights.size() )
        {
            throw std::runtime_error(
                "Network::set_neighbours_and_weights: both buffers need to have the same length!" );
        }

        neighbour_list[agent_idx].assign( buffer_neighbours.begin(), buffer_neighbours.end() );
        weight_list[agent_idx].assign( buffer_weights.begin(), buffer_weights.end() );
    }

    /*
    Adds an edge between agent_idx_i and agent_idx_j with weight w
    */
    void push_back_neighbour_and_weight( size_t agent_idx_i, size_t agent_idx_j, WeightT w )
    {
        neighbour_list[agent_idx_i].push_back( agent_idx_j );
        weight_list[agent_idx_i].push_back( w );
    }

    /*
    Transposes the network, without switching the direction flag (expensive).
    Example: N(inc) -> N(inc)^T
    */
    void transpose()
    {
        toggle_incoming_outgoing();
        switch_direction_flag();
    }

    /*
    Switches the direction flag *without* transposing the network (expensive)
    Example: N(inc) -> N(out)
    */
    void toggle_incoming_outgoing()
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

        // Swap the edge direction
        switch_direction_flag();
    }

    /*
    Only switches the direction flag. This effectively transposes the network and, simultaneously, changes its
    representation.
    Example: N(inc) -> N^T(out)
    */
    void switch_direction_flag()
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

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
    EdgeDirection _direction;
    std::vector<AgentT> agents; // List of agents of type AgentType
};

} // namespace Seldon