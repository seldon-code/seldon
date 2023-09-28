#pragma once
#include "agent.hpp"
#include <cstddef>
#include <vector>

#pragma once

namespace Seldon
{

// The abstract model interface
class ModelBase
{
public:
    virtual AgentBase * get_agent( int idx ) = 0; // Use this to get an abstract representation of the agent at idx
    virtual void run()                       = 0;
};

} // namespace Seldon