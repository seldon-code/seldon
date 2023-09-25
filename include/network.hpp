#pragma once
#include <cstddef>
#include <optional>
#include <random>
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

    // // Seed only once
    // // TODO: make this thread-safe??
    // std::mt19937 & RNG(const std::optional<int> seed=std::nullopt )
    // {
    //      std::random_device rd;
    //             std::mt19937 gen( rd() );
    //             return gen;

    //     // // if (gen.has_value()){ return gen.value();}
    //     // // else{
    //     //     if ( seed.has_value() )
    //     //     {
    //     //         gen = seed( seed.value() );
    //     //     } // initialize with seed value
    //     //     else{
    //     //         std::random_device rd;
    //     //         std::mt19937 gen( rd() );
    //     //     } // no seed provided

    //     // // }
    // }

    void get_adjacencies( std::size_t agent_idx, std::vector<size_t> & buffer ) const;
    void get_edges( std::size_t agent_idx, connectionVectorT & buffer ) const;

private:
    std::vector<connectionVectorT> adjacency_list; // Adjacency list for the connections
    // std::mt19937 gen;

    // Function for getting a vector of k agents (corresponding to connections)
    // drawing from n agents (without duplication)
    // also includes agent_idx, the agent itself
    void draw_unique_k_from_n(
        std::size_t agent_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer ) const;
};

} // namespace Seldon