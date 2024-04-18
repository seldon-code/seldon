#pragma once
#include "network_base.hpp"

namespace Seldon
{

/*
    A class that represents an undirected graph using adjacency lists.
*/
template<typename AgentType, typename WeightType = double>
class UndirectedNetwork : public NetworkBase<AgentType, WeightType>
{
public:
    using WeightT = WeightType;
    using AgentT  = AgentType;

    UndirectedNetwork() = default;

    UndirectedNetwork( size_t n_agents ) : NetworkBase<AgentT>( n_agents ) {}

    UndirectedNetwork( std::vector<AgentT> agents ) : NetworkBase<AgentT>( agents ) {}

    UndirectedNetwork(
        std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list )
            : NetworkBase<AgentT>( std::move( neighbour_list ), std::move( weight_list ) )
    {
    }

    /*
    Gives the number of edges connected to agent_idx
    If agent_idx is nullopt, gives the total number of edges
    */
    [[nodiscard]] std::size_t n_edges( std::optional<std::size_t> agent_idx = std::nullopt ) const override
    {
        if( agent_idx.has_value() )
        {
            return this->neighbour_list[agent_idx.value()].size();
        }
        else
        {
            // Return the number of edges in the undirected graph, which is half effectively half the edges
            return 0.5
                   * std::transform_reduce(
                       this->neighbour_list.cbegin(), this->neighbour_list.cend(), 0, std::plus{},
                       []( const auto & neigh_list ) { return neigh_list.size(); } );
        }
    }

    /*
    Sets the weight for agent_idx, for an existing neighbour index
    */
    void set_edge_weight( std::size_t agent_idx, std::size_t index_neighbour, WeightT weight ) override
    {
        this->weight_list[agent_idx][index_neighbour] = weight;
        auto agent_jdx                                = this->neighbour_list[agent_idx][index_neighbour];
        auto it
            = std::find( this->neighbour_list[agent_jdx].begin(), this->neighbour_list[agent_jdx].end(), agent_idx );
        // If agent_idx is not in the neighbour list of agent_jdx, add it to the list and add the weight
        if( it == this->neighbour_list[agent_jdx].end() )
        {
            this->neighbour_list[agent_jdx].push_back( agent_idx );
            this->weight_list[agent_jdx].push_back( weight );
        }
        // If found then update the weight
        else
        {
            this->weight_list[agent_jdx][*it] = weight;
        }
    }

    /*
    Adds an edge between agent_idx_i and agent_idx_j with weight w
    This could cause double counting, if not carefully called.
    */
    void push_back_neighbour_and_weight( size_t agent_idx_i, size_t agent_idx_j, WeightT w ) override
    {
        // agent_idx_j is a neighbour of agent_idx_i
        this->neighbour_list[agent_idx_i].push_back( agent_idx_j );
        this->weight_list[agent_idx_i].push_back( w );
        // agent_idx_i is a neighbour of agent_idx_j
        this->neighbour_list[agent_idx_j].push_back( agent_idx_i );
        this->weight_list[agent_idx_j].push_back( w );
    }

    /*
    Sorts the neighbours by index and removes doubly counted edges by summing the weights
    */
    void remove_double_counting() override
    {
        std::vector<size_t> sorting_indices{};

        for( size_t idx_agent = 0; idx_agent < this->n_agents(); idx_agent++ )
        {

            auto & neighbours   = this->neighbour_list[idx_agent];
            auto & weights      = this->weight_list[idx_agent];
            bool updated_weight = false;

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
                    updated_weight = true;
                }
            }

            this->neighbour_list[idx_agent] = neighbours_copy;

            // If the weights were updated, change the weight for the other edge:
            if( updated_weight )
            {
                for( size_t i = 0; i < n_neighbours; i++ )
                {
                    set_edge_weight( idx_agent, this->neighbour_list[idx_agent][i], this->weight_list[idx_agent][i] );
                }
            }
            else
            {
                this->weight_list[idx_agent] = weights_copy;
            }
        }
    }
};

} // namespace Seldon