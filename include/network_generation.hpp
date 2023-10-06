#pragma once
#include "network.hpp"
#include <memory>
#include <random>

namespace Seldon
{
// Returns a unique pointer to a new network with n_connections per agent
std::unique_ptr<Network> generate_n_connections( int n_agents, int n_connections, std::mt19937 & gen );
std::unique_ptr<Network> generate_from_file( const std::string & file );
} // namespace Seldon