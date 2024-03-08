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
    using WeightT = double;

    Network( std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list );

    // Gives the total number of nodes in the network
    std::size_t n_agents() const;

    /* Gives the number of edges going out from agent_idx.
       If agent_idx is nullopt, give the total number of edges
    */
    std::size_t n_edges( std::optional<std::size_t> agent_idx = std::nullopt ) const;

    std::span<const size_t> get_neighbours( std::size_t agent_idx ) const;
    std::span<size_t> get_neighbours( std::size_t agent_idx );
    std::span<const WeightT> get_weights( std::size_t agent_idx ) const;
    std::span<WeightT> get_weights( std::size_t agent_idx );

    void set_weights( std::size_t agent_idx, std::span<const WeightT> weights );

    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, const WeightT & weight );

    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, std::span<const WeightT> buffer_weights );

    void push_back_neighbour_and_weight( size_t i, size_t j, WeightT w );

    void transpose();

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
};

} // namespace Seldon