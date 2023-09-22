#pragma once
#include <cstddef>
#include <tuple>
#include <vector>

namespace Seldon
{

class Network
{
public:
    using edgeT             = std::tuple<size_t, double>;
    using connectionVectorT = std::vector<edgeT>;

    Network() = default;
    Network( std::size_t n_agents, std::size_t n_connections );

    std::size_t n_agents() const
    {
        return adjacency_list.size();
    }

    void get_adjacencies( std::size_t agent_idx, std::vector<size_t> & buffer ) const;
    void get_edges( std::size_t agent_idx, connectionVectorT & buffer ) const;

private:
    std::vector<connectionVectorT> adjacency_list; // Adjacency list for the connections

    // Function for constructing the adjacency list
    void build_adjacency_list();
};

} // namespace Seldon