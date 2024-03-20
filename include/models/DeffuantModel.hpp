#pragma once

#include "agents/simple_agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <cstddef>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Seldon
{

class DeffuantModel : public Model<SimpleAgent>
{
public:
    using AgentT   = SimpleAgent;
    using NetworkT = Network<AgentT>;

    double homophily_threshold = 0.2;   // d in paper
    double mu                  = 0.5;   // convergence parameter
    bool use_network           = false; // for the basic Deffuant model

    DeffuantModel( NetworkT & network, std::mt19937 & gen, bool use_network );

    void iteration() override;
    // bool finished() override;

private:
    NetworkT & network;
    std::mt19937 & gen; // reference to simulation Mersenne-Twister engine

    // Select interacting agents
    std::vector<std::size_t> select_interacting_agent_pair();
};

} // namespace Seldon