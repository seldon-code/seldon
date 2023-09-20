#pragma once
#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>

namespace Seldon
{

class Network
{
public:
    void get_edges( std::size_t agent_idx, std::vector<std::tuple<std::size_t, double>> & buffer );

    // private:
    //     std::vector<std::tuple<std::size_t, std::size_t, double>> adjacency_edges;
};

} // namespace Seldon