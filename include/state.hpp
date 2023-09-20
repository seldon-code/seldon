#pragma once

#include "agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <string>
#include <vector>

namespace Seldon
{

class State
{
public:
    int n_agents;
    Model * model;

    State( std::string toml_file );

    // Holds all the information needed in the simulation run, e.g a reference to
    // a model object, time, the vector of agent opiniond
    Network network;
};

} // namespace Seldon