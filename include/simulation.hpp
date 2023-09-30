#pragma once

#include "model_base.hpp"
#include "network.hpp"
#include <memory>
#include <random>
#include <string>

namespace Seldon
{

class Simulation
{
private:
    std::mt19937 gen;

public:
    int n_agents;
    std::unique_ptr<ModelBase> model;
    std::unique_ptr<Network> network;
    Simulation( std::string toml_file );
};

} // namespace Seldon