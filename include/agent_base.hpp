#pragma once
#include <string>
namespace Seldon
{

// AGENTS
/* Agent Base is the abstract interface, for every part of the code that is not the actual computation, IO etc. */
class AgentBase
{
public:
    virtual std::string to_string() const = 0;
    virtual ~AgentBase()                  = default;
    // TODO: eventually a from_string might also be needed
    // virtual void from_string()          = 0;
};

} // namespace Seldon