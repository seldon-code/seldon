#include "agent.hpp"
#include "agents/discrete_vector_agent.hpp"
#include "models/DeffuantModel.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <random>
#include <vector>

namespace Seldon
{

template<>
void DeffuantModelAbstract<DiscreteVectorAgent>::initialize_agents()
{
    std::uniform_int_distribution<int> dist( 0, 1 );
    int dim = 5;

    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        auto & opinion = network.agents[i].data.opinion;
        opinion.resize( dim );
        for( auto & o : opinion )
        {
            o = dist( gen );
        }
    }
}

template<>
void DeffuantModelAbstract<DiscreteVectorAgent>::update_rule( AgentT & agent1, AgentT & agent2 )
{
    size_t dim      = agent1.data.opinion.size();
    auto & opinion1 = agent1.data.opinion;
    auto & opinion2 = agent2.data.opinion;

    auto distance = hamming_distance( std::span( opinion1 ), std::span( opinion2 ) );

    std::uniform_int_distribution<> dist_pair( 0, 1 );
    std::uniform_real_distribution<> dist_convince( 0, 1 );

    // Update rule
    // Only if less than homophily_threshold
    if( distance < homophily_threshold )
    {
        for( size_t idx_opinion = 0; idx_opinion < dim; idx_opinion++ )
        {
            if( opinion1[idx_opinion] != opinion2[idx_opinion] )
            {
                // randomly select one of the
                auto idx_selected = dist_pair( gen );
                if( idx_selected == 0 && mu < dist_convince( gen ) )
                {
                    opinion1[idx_opinion] = opinion2[idx_opinion];
                }
                else
                {
                    opinion2[idx_opinion] = opinion1[idx_opinion];
                }
            }
        }
    }
}

template class DeffuantModelAbstract<DiscreteVectorAgent>;

} // namespace Seldon