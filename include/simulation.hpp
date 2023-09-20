#pragma once

#include "agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Seldon
{

class Simulation
{
public:
    int n_agents;
    std::unique_ptr<Model> model;

    Network network;
    Simulation( std::string toml_file );
};

} // namespace Seldon