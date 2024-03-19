#include "models/DeffuantModel.hpp"
#include "network.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <optional>
#include <random>
#include <vector>

namespace Seldon
{

DeffuantModel::DeffuantModel( NetworkT & network, std::mt19937 & gen )
        : Model<DeffuantModel::AgentT>(), network( network ), gen( gen )
{
    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        network.agents[i].data.opinion = double( i ) / double( network.agents.size() );
    }
}

void DeffuantModel::iteration()
{
    Model<AgentT>::iteration(); // Update n_iterations

    // Although the model defines each iteration as choosing *one*
    // pair of agents, we will define each iteration as sampling
    // n_agents pairs (similar to the time unit in evolution plots in the paper)
    for( size_t i = 0; i < network.n_agents(); i++ )
    {

        auto interacting_agents = std::vector<std::size_t>();

        // Pick any two agents to interact, randomly, without repetition
        draw_unique_k_from_n( std::nullopt, 2, network.n_agents(), interacting_agents, gen );

        auto & agent1 = network.agents[interacting_agents[0]];
        auto & agent2 = network.agents[interacting_agents[1]];

        // Update rule
        auto opinion_diff = agent1.data.opinion - agent2.data.opinion;

        // Only if less than homophily_threshold
        if( std::abs( opinion_diff ) < homophily_threshold )
        {
            agent1.data.opinion -= mu * opinion_diff;
            agent2.data.opinion += mu * opinion_diff;
        }
    }
}
} // namespace Seldon