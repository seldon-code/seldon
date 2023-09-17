#pragma once

#include <vector>
#include <string>
#include "agent.hpp"
#include "network.hpp"

namespace Seldon {

class State
{
    public:
    State(std::string toml_file);

    // Holds all the information needed in the simulation run, e.g a reference to a model object, time, the vector of agent opiniond
    std::vector<Agent> agents;
    Network network;
};

}