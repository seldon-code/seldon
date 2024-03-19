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

    double homophily_threshold = 0.2; // d in paper
    double mu                  = 0.5; // convergence parameter

    DeffuantModel( NetworkT & network, std::mt19937 & gen );

    void iteration() override;
    // bool finished() override;

private:
    NetworkT & network;
    std::mt19937 & gen; // reference to simulation Mersenne-Twister engine
};

} // namespace Seldon