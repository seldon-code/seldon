#pragma once
#include "network.hpp"
#include <cstddef>
#include <memory>
#include <random>

namespace Seldon::NetworkGeneration
{
/* Constructs a new network with n_connections per agent
   If self_interaction=true, a connection of the agent with itself is included, which is *not* counted in n_connections
*/
std::unique_ptr<Network>
generate_n_connections( size_t n_agents, size_t n_connections, bool self_interaction, std::mt19937 & gen );
std::unique_ptr<Network> generate_fully_connected( size_t n_agents, Network::WeightT weight = 0.0 );
std::unique_ptr<Network> generate_fully_connected( size_t n_agents, std::mt19937 & gen );
std::unique_ptr<Network> generate_from_file( const std::string & file );
} // namespace Seldon::NetworkGeneration