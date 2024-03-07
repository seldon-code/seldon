#pragma once
#include "network.hpp"
#include <cstddef>
#include <memory>
#include <random>

namespace Seldon
{
// Returns a unique pointer to a new network with n_connections per agent
std::unique_ptr<Network> generate_n_connections( size_t n_agents, int n_connections, std::mt19937 & gen );
std::unique_ptr<Network> generate_fully_connected( size_t n_agents, Network::WeightT weight = 0.0 );
std::unique_ptr<Network> generate_fully_connected( size_t n_agents, std::mt19937 & gen );
std::unique_ptr<Network> generate_from_file( const std::string & file );
} // namespace Seldon