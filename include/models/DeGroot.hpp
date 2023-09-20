#pragma once
#include "agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model
{
    using AgentT = Agent;

private:
    std::vector<AgentT> agents;
    std::vector<AgentT> agent_current_copy;
    Network & network;

public:
    DeGrootModel( int n_agents, Network & network );

    void run() override;
};

} // namespace Seldon