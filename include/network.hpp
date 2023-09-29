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
    Network( std::size_t n_agents, std::size_t n_connections, std::optional<int> seed = std::nullopt );

    std::size_t n_agents() const
    {
        return neighbour_list.size();
    }

    // Seed only once
    // TODO: make this thread-safe??
    void initialize_rng( const std::optional<int> seed )
    {
        gen = std::mt19937( seed.value_or( std::random_device()() ) );
    }

    void get_adjacencies( std::size_t agent_idx, std::vector<size_t> & buffer ) const;
    void get_weights( std::size_t agent_idx, std::vector<double> & buffer ) const;

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<double>> weight_list;    // List for the interaction weights of each connection
    std::mt19937 gen;

    // Function for getting a vector of k agents (corresponding to connections)
    // drawing from n agents (without duplication)
    // We later add the agent itself
    void draw_unique_k_from_n( std::size_t agent_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer );
};

} // namespace Seldon