#include "models/DeGroot.hpp"
#include <iterator>

Seldon::DeGrootModel::DeGrootModel( int n_agents, Network & network ) : network( network )
{
    agents             = std::vector<AgentT>( n_agents );
    agent_current_copy = std::vector<AgentT>( n_agents );
}

void Seldon::DeGrootModel::run()
{
    auto edge_buffer = Network::connectionVectorT();

    for( std::size_t i = 0; i < agents.size(); i++ )
    {
        network.get_edges( i, edge_buffer );

        agent_current_copy[i].opinion = 0.0;

        for( auto & edge : edge_buffer )
        {
            int j;
            double weight;
            std::tie( j, weight ) = edge;
            agent_current_copy[i].opinion += weight * agents[j].opinion;
        }
    }

    // Update the original agent opinions
    for( std::size_t i = 0; i < agents.size(); i++ )
    {
        agents[i] = agent_current_copy[i];
    }
}