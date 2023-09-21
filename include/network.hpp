#pragma once
#include <tuple>
#include <vector>

namespace Seldon
{

class Network
{

    using connectionVectorT = std::vector<std::tuple<int, double>>;

public:
    Network() = default;
    Network( int n_agents, int n_connections );

    void get_edges( std::size_t agent_idx, std::vector<std::tuple<std::size_t, double>> & buffer );

private:
    std::vector<connectionVectorT> adjacency_list; // Adjacency list for the connections

    // Function for constructing the adjacency list
    void build_adjacency_list();
};

} // namespace Seldon