#include "models/DeffuantModel.hpp"
#include <cmath>
#include <cstddef>
#include <vector>

namespace Seldon
{

template<>
void DeffuantModelAbstract<SimpleAgent>::initialize_agents( size_t )
{
    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        network.agents[i].data.opinion = double( i ) / double( network.agents.size() );
    }
}

template<>
void DeffuantModelAbstract<SimpleAgent>::update_rule( AgentT & agent1, AgentT & agent2 )
{
    // Update rule
    auto opinion_diff = agent1.data.opinion - agent2.data.opinion;
    // Only if less than homophily_threshold
    if( std::abs( opinion_diff ) < homophily_threshold )
    {
        agent1.data.opinion -= mu * opinion_diff;
        agent2.data.opinion += mu * opinion_diff;
    }
}

template class DeffuantModelAbstract<SimpleAgent>;

} // namespace Seldon