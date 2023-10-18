#include "models/DeGroot.hpp"
#include <cmath>
#include <iterator>

Seldon::DeGrootModel::DeGrootModel( int n_agents, Network & network )
        : Model<AgentT>( n_agents ), network( network ), agents_current_copy( std::vector<AgentT>( n_agents ) )
{
    for( size_t i = 0; i < agents.size(); i++ )
    {
        agents[i].data = double( i ) / double( agents.size() );
    }
}

void Seldon::DeGrootModel::iteration()
{
    Model<AgentT>::iteration();

    auto neighbour_buffer = std::vector<size_t>();
    auto weight_buffer    = std::vector<double>();
    size_t j_index        = 0;
    double weight         = 0.0;

    for( size_t i = 0; i < agents.size(); i++ )
    {
        network.get_neighbours( i, neighbour_buffer );
        network.get_weights( i, weight_buffer );
        agents_current_copy[i].data = 0.0;
        for( size_t j = 0; j < neighbour_buffer.size(); j++ )
        {
            j_index = neighbour_buffer[j];
            weight  = weight_buffer[j];
            agents_current_copy[i].data += weight * agents[j_index].data;
        }
    }

    max_opinion_diff = 0;
    // Update the original agent opinions
    for( std::size_t i = 0; i < agents.size(); i++ )
    {
        max_opinion_diff = std::max( max_opinion_diff, std::abs( agents[i].data - agents_current_copy[i].data ) );
        agents[i]        = agents_current_copy[i];
    }
}

bool Seldon::DeGrootModel::finished()
{
    bool converged = max_opinion_diff < convergence_tol;
    return Model<AgentT>::finished() || converged;
}