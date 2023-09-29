#pragma once
#include <cstddef>
#include <optional>
#include <random>
#include <vector>

namespace Seldon
{

class Network
{
public:
    Network() = default;
    Network( std::size_t n_agents, std::size_t n_connections, std::mt19937 & gen );

    std::size_t n_agents() const
    {
        return neighbour_list.size();
    }

    void get_neighbours( std::size_t agent_idx, std::vector<size_t> & buffer ) const;
    void get_weights( std::size_t agent_idx, std::vector<double> & buffer ) const;

private:
    std::mt19937 * gen = nullptr;
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<double>> weight_list;    // List for the interaction weights of each connection

    // Function for getting a vector of k agents (corresponding to connections)
    // drawing from n agents (without duplication)
    // We later add the agent itself
    void draw_unique_k_from_n( std::size_t agent_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer );
};

} // namespace Seldon