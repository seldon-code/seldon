#pragma once
#include <cstddef>
#include <optional>
#include <vector>

namespace Seldon
{

class Network
{
public:
    using WeightT = double;

    Network( std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list );

    std::size_t n_agents() const;

    void get_neighbours( std::size_t agent_idx, std::vector<size_t> & buffer ) const;

    void get_weights( std::size_t agent_idx, std::vector<WeightT> & buffer ) const;

    std::size_t get_n_edges( std::size_t agent_idx ) const;

    WeightT & get_weight( std::size_t agent_idx, std::size_t i_weight );

    std::size_t & get_neighbour( std::size_t agent_idx, std::size_t i_neighbour );

    void set_weights( std::size_t agent_idx, const std::vector<WeightT> & weights );

    void set_neighbours_and_weights(
        std::size_t agent_idx, const std::vector<size_t> & buffer_neighbours, const WeightT & weight );

    void set_neighbours_and_weights(
        std::size_t agent_idx, const std::vector<size_t> & buffer_neighbours,
        const std::vector<WeightT> & buffer_weights );

    void push_back_neighbour_and_weight( size_t i, size_t j, WeightT w );

    void transpose();

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
};

} // namespace Seldon