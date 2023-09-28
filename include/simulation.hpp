#pragma once

#include "model_base.hpp"
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
    std::unique_ptr<ModelBase> model;

    Network network;
    Simulation( std::string toml_file );
};

} // namespace Seldon