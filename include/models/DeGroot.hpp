#pragma once
#include "agent.hpp"
#include "model.hpp"

namespace Seldon
{

class DeGrootModel : public Model
{
    using AgentT = Agent;

private:
    std::vector<AgentT> agents;
    std::vector<AgentT> agent_current_copy;

public:
    DeGrootModel( State & state );

    void run() override;
};

} // namespace Seldon