#pragma once
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace Seldon
{

class Network
{
public:
    enum class EdgeDirection
    {
        Incoming,
        Outgoing
    };

    using WeightT = double;

    Network(
        std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list,
        EdgeDirection direction );

    // Gives the total number of nodes in the network
    std::size_t n_agents() const;

    /*
        Gives the number of edges going out/coming in from/at agent_idx.
        If agent_idx is nullopt, give the total number of edges
    */
    std::size_t n_edges( std::optional<std::size_t> agent_idx = std::nullopt ) const;

    std::span<const size_t> get_neighbours( std::size_t agent_idx ) const;
    std::span<size_t> get_neighbours( std::size_t agent_idx );
    std::span<const WeightT> get_weights( std::size_t agent_idx ) const;
    std::span<WeightT> get_weights( std::size_t agent_idx );

    std::vector<std::vector<size_t>> strongly_connected_components() const;

    void set_weights( std::size_t agent_idx, std::span<const WeightT> weights );

    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, const WeightT & weight );

    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, std::span<const WeightT> buffer_weights );

    void push_back_neighbour_and_weight( size_t i, size_t j, WeightT w );

    void transpose();

    const EdgeDirection & direction() const;

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
    EdgeDirection _direction;
    void swap_direction();
};

} // namespace Seldon