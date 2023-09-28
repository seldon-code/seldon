#pragma once
#include "model_base.hpp"

namespace Seldon
{

/* Model<T> is a base class from which the acutal models would derive. They have efficient access to a vector of AgentT,
 * without any pointer indirections */
template<typename AgentT_>
class Model : public ModelBase
{
public:
    using AgentT = AgentT_;
    std::vector<AgentT> agents;
    Model( size_t n_agents )
    {
        agents = std::vector<AgentT>( int( n_agents ), AgentT() );
    }

    AgentBase * get_agent( int idx ) override // For this to work AgentT needs to be a subclass of AgentBase
    {
        return &agents[idx];
    }
};

} // namespace Seldon