#pragma once
#include "agent.hpp"
#include <cstddef>
#include <vector>

namespace Seldon
{

class Model
{
    // TODO: think about what to do for different types of opinion than just double
public:
    using AgentT = Agent;
    std::vector<AgentT> agents;
    std::vector<AgentT> agent_current_copy;
    Model(){};
    virtual void run() = 0;
};

} // namespace Seldon