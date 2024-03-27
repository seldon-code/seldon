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
    A class that represents an undirected graph using adjacency lists.
*/
template<typename AgentType, typename WeightType = double>
class UndirectedNetwork
{
public:
    using WeightT = WeightType;
    using AgentT  = AgentType;
    // @TODO: Make this private later
    std::vector<AgentT> agents{}; // List of agents of type AgentType

    UndirectedNetwork() = default;

    UndirectedNetwork( size_t n_agents )
            : agents( std::vector<AgentT>( n_agents ) ),
              neighbour_list( std::vector<std::vector<size_t>>( n_agents, std::vector<size_t>{} ) ),
              weight_list( std::vector<std::vector<WeightT>>( n_agents, std::vector<WeightT>{} ) )
    {
    }

    UndirectedNetwork( std::vector<AgentT> agents )
            : agents( agents ),
              neighbour_list( std::vector<std::vector<size_t>>( agents.size(), std::vector<size_t>{} ) ),
              weight_list( std::vector<std::vector<WeightT>>( agents.size(), std::vector<WeightT>{} ) )
    {
    }

    UndirectedNetwork(
        std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list )
            : agents( std::vector<AgentT>( neighbour_list.size() ) ),
              neighbour_list( neighbour_list ),
              weight_list( weight_list )
    {
    }

    /*
    Gives the total number of nodes in the network
    */
    [[nodiscard]] std::size_t n_agents() const
    {
        return agents.size();
    }

    /*
    Gives the number of edges connected to agent_idx
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
            // Return the number of edges in the undirected graph, which is half effectively half the edges
            return 0.5
                   * std::transform_reduce(
                       neighbour_list.cbegin(), neighbour_list.cend(), 0, std::plus{},
                       []( const auto & neigh_list ) { return neigh_list.size(); } );
        }
    }

    /*
    Gives the strongly connected components in the graph
    @TODO: implement as visitor on the graph?
    */
    [[nodiscard]] std::vector<std::vector<size_t>> strongly_connected_components() const
    {
        // Now that we have the neighbour list (or adjacency list)
        // Run Tarjan's algorithm for strongly connected components
        auto tarjan_scc = TarjanConnectivityAlgo( neighbour_list );
        return tarjan_scc.scc_list;
    }

    /*
    Gives a view into the neighbour indices connected to agent_idx
    */
    [[nodiscard]] std::span<const size_t> get_neighbours( std::size_t agent_idx ) const
    {
        return std::span( neighbour_list[agent_idx].data(), neighbour_list[agent_idx].size() );
    }

    /*
    Gives a view into the edge weights corresponding to edges connected to agent_idx
    */
    [[nodiscard]] std::span<const WeightT> get_weights( std::size_t agent_idx ) const
    {
        return std::span<const WeightT>( weight_list[agent_idx].data(), weight_list[agent_idx].size() );
    }

    /*
    Sets the weight for agent_idx, for an existing neighbour index
    */
    void set_edge_weight( std::size_t agent_idx, std::size_t index_neighbour, WeightT weight )
    {
        weight_list[agent_idx][index_neighbour] = weight;
        auto agent_jdx                          = neighbour_list[agent_idx][index_neighbour];
        auto it = std::find( neighbour_list[agent_jdx].begin(), neighbour_list[agent_jdx].end(), agent_idx );
        // If agent_idx is not in the neighbour list of agent_jdx, add it to the list and add the weight
        if( it == neighbour_list[agent_jdx].end() )
        {
            neighbour_list[agent_jdx].push_back( agent_idx );
            weight_list[agent_jdx].push_back( weight );
        }
        // If found then update the weight
        else
        {
            weight_list[agent_jdx][*it] = weight;
        }
    }

    /*
    Gets the weight for agent_idx, for a neighbour index
    */
    const WeightT get_edge_weight( std::size_t agent_idx, std::size_t index_neighbour ) const
    {
        return weight_list[agent_idx][index_neighbour];
    }

    /*
    Adds an edge between agent_idx_i and agent_idx_j with weight w
    This could cause double counting, if not carefully called.
    */
    void push_back_neighbour_and_weight( size_t agent_idx_i, size_t agent_idx_j, WeightT w )
    {
        // agent_idx_j is a neighbour of agent_idx_i
        neighbour_list[agent_idx_i].push_back( agent_idx_j );
        weight_list[agent_idx_i].push_back( w );
        // agent_idx_i is a neighbour of agent_idx_j
        neighbour_list[agent_idx_j].push_back( agent_idx_i );
        weight_list[agent_idx_j].push_back( w );
    }

    /*
    Sorts the neighbours by index and removes doubly counted edges by summing the weights
    */
    void remove_double_counting()
    {
        std::vector<size_t> sorting_indices{};

        for( size_t idx_agent = 0; idx_agent < n_agents(); idx_agent++ )
        {

            auto & neighbours = neighbour_list[idx_agent];
            auto & weights    = weight_list[idx_agent];

            std::vector<WeightT> weights_copy{};
            std::vector<size_t> neighbours_copy{};

            const auto n_neighbours = neighbours.size();

            // First we will the sorting_indices array
            sorting_indices.resize( n_neighbours );
            std::iota( sorting_indices.begin(), sorting_indices.end(), 0 );

            // Then, we figure out how to sort the neighbour indices list
            std::sort(
                sorting_indices.begin(), sorting_indices.end(),
                [&]( auto i1, auto i2 ) { return neighbours[i1] < neighbours[i2]; } );

            std::optional<size_t> last_neighbour_index = std::nullopt;
            for( size_t i = 0; i < n_neighbours; i++ )
            {
                const auto sort_idx              = sorting_indices[i];
                const auto current_neigbhour_idx = neighbours[sort_idx];
                const auto current_weight        = weights[sort_idx];

                if( last_neighbour_index != current_neigbhour_idx )
                {
                    weights_copy.push_back( current_weight );
                    neighbours_copy.push_back( current_neigbhour_idx );
                    last_neighbour_index = current_neigbhour_idx;
                }
                else
                {
                    weights_copy.back() += current_weight;
                }
            }

            weight_list[idx_agent]    = weights_copy;
            neighbour_list[idx_agent] = neighbours_copy;
        }
    }

    /*
    Clears the network
    */
    void clear()
    {
        for( auto & w : weight_list )
            w.clear();

        for( auto & n : neighbour_list )
            n.clear();
    }

private:
    std::vector<std::vector<size_t>> neighbour_list{}; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list{};   // List for the interaction weights of each connections
};

} // namespace Seldon