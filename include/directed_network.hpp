#pragma once
#include "network_base.hpp"
#include <utility>

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
template<typename AgentType, typename WeightType = double>
class DirectedNetwork : public NetworkBase<AgentType, WeightType>
{
public:
    enum class EdgeDirection
    {
        Incoming,
        Outgoing
    };

    using WeightT = WeightType;
    using AgentT  = AgentType;

    DirectedNetwork() = default;

    DirectedNetwork( size_t n_agents ) : NetworkBase<AgentT>( n_agents ) {}

    DirectedNetwork( std::vector<AgentT> agents ) : NetworkBase<AgentT>( agents ) {}

    DirectedNetwork(
        std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list,
        EdgeDirection direction )
            : NetworkBase<AgentT>( std::move( neighbour_list ), std::move( weight_list ) ), _direction( direction )
    {
    }

    /*
    Gives the number of edges going out/coming in at agent_idx, depending on the value of direction().
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
            return std::transform_reduce(
                this->neighbour_list.cbegin(), this->neighbour_list.cend(), 0, std::plus{},
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
    Set the edge weights going out/coming in at agent_idx
    */
    void set_weights( std::size_t agent_idx, const std::span<const WeightT> weights )
    {
        if( this->neighbour_list[agent_idx].size() != weights.size() )
        {
            throw std::runtime_error( "DirectedNetwork::set_weights: tried to set weights of the wrong size!" );
        }
        this->weight_list[agent_idx].assign( weights.begin(), weights.end() );
    }

    /*
    Sets the neighbour indices
    */
    void set_edge( std::size_t agent_idx, std::size_t index_neighbour, std::size_t agent_jdx )
    {
        this->neighbour_list[agent_idx][index_neighbour] = agent_jdx;
    }

    /*
    Sets the neighbour indices and sets the weight to a constant value at agent_idx
    */
    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, const WeightT & weight )
    {
        this->neighbour_list[agent_idx].assign( buffer_neighbours.begin(), buffer_neighbours.end() );
        this->weight_list[agent_idx].resize( buffer_neighbours.size() );
        std::fill( this->weight_list[agent_idx].begin(), this->weight_list[agent_idx].end(), weight );
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
                "DirectedNetwork::set_neighbours_and_weights: both buffers need to have the same length!" );
        }

        this->neighbour_list[agent_idx].assign( buffer_neighbours.begin(), buffer_neighbours.end() );
        this->weight_list[agent_idx].assign( buffer_weights.begin(), buffer_weights.end() );
    }

    /*
    Sets the weight for agent_idx, for a neighbour index
    */
    void set_edge_weight( std::size_t agent_idx, std::size_t index_neighbour, WeightT weight ) override
    {
        this->weight_list[agent_idx][index_neighbour] = weight;
    }

    /*
    Adds an edge between agent_idx_i and agent_idx_j with weight w
    */
    void push_back_neighbour_and_weight( size_t agent_idx_i, size_t agent_idx_j, WeightT w ) override
    {
        this->neighbour_list[agent_idx_i].push_back( agent_idx_j );
        this->weight_list[agent_idx_i].push_back( w );
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
        std::vector<std::vector<size_t>> neighbour_list_transpose( this->n_agents(), std::vector<size_t>( 0 ) );
        std::vector<std::vector<WeightT>> weight_list_transpose( this->n_agents(), std::vector<WeightT>( 0 ) );

        for( size_t i_agent = 0; i_agent < this->n_agents(); i_agent++ )
        {
            for( size_t i_neighbour = 0; i_neighbour < this->neighbour_list[i_agent].size(); i_neighbour++ )
            {
                const auto neighbour = this->neighbour_list[i_agent][i_neighbour];
                const auto weight    = this->weight_list[i_agent][i_neighbour];
                neighbour_list_transpose[neighbour].push_back( i_agent );
                weight_list_transpose[neighbour].push_back( weight );
            }
        }

        this->neighbour_list = std::move( neighbour_list_transpose );
        this->weight_list    = std::move( weight_list_transpose );

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

    /*
    Sorts the neighbours by index and removes doubly counted edges by summing the weights
    */
    void remove_double_counting() override
    {
        std::vector<size_t> sorting_indices{};

        for( size_t idx_agent = 0; idx_agent < this->n_agents(); idx_agent++ )
        {

            auto & neighbours = this->neighbour_list[idx_agent];
            auto & weights    = this->weight_list[idx_agent];

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

            this->weight_list[idx_agent]    = weights_copy;
            this->neighbour_list[idx_agent] = neighbours_copy;
        }
    }

private:
    EdgeDirection _direction{};
};

} // namespace Seldon