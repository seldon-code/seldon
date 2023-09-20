#pragma once

#include "agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Seldon
{

class State
{
public:
    int n_agents;
    std::unique_ptr<Model> model;

    Network network;
    State( std::string toml_file );

    // Holds all the information needed in the simulation run, e.g a reference to
    // a model object, time, the vector of agent opiniond
};

} // namespace Seldon