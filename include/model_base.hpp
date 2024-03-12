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
    int n_iterations                         = 0;
    virtual AgentBase * get_agent( int idx ) = 0; // Use this to get an abstract representation of the agent at idx

    template<typename AgentT>
    AgentT * get_agent_as( int idx )
    {
        return static_cast<AgentT *>( get_agent( idx ) );
    }

    virtual void iteration()                                  = 0;
    virtual bool finished()                                   = 0;
    virtual void agents_from_file( const std::string & file ) = 0;
    virtual ~ModelBase()                                      = default;
};

} // namespace Seldon