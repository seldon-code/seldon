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

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
};

} // namespace Seldon