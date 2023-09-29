#include "models/DeGroot.hpp"
#include <iterator>

Seldon::DeGrootModel::DeGrootModel( int n_agents, Network & network )
        : Model<AgentT>( n_agents ), network( network ), agents_current_copy( std::vector<AgentT>( n_agents ) )
{
    for( size_t i = 0; i < agents.size(); i++ )
    {
        agents[i].opinion = double( i ) / double( agents.size() );
    }
}

void Seldon::DeGrootModel::run()
{
    auto neighbour_buffer = std::vector<size_t>();
    auto weight_buffer    = std::vector<double>();
    size_t j_index;
    double weight;

    for( size_t i = 0; i < agents.size(); i++ )
    {
        network.get_adjacencies( i, neighbour_buffer );
        network.get_weights( i, weight_buffer );
        agents_current_copy[i].opinion = 0.0;
        for( size_t j = 0; j < neighbour_buffer.size(); j++ )
        {
            j_index = neighbour_buffer[j];
            weight  = weight_buffer[j];
            agents_current_copy[i].opinion += weight * agents[j_index].opinion;
        }
    }

    // Update the original agent opinions
    for( std::size_t i = 0; i < agents.size(); i++ )
    {
        agents[i] = agents_current_copy[i];
    }
}