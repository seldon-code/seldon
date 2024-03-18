#include "models/DeGroot.hpp"
#include <cmath>
#include <iterator>

namespace Seldon
{

DeGrootModel::DeGrootModel( NetworkT & network )
        : Model<AgentT>(), network( network ), agents_current_copy( network.agents )
{
    // For a strongly connected network, the number of SCCs should be 1
    // Print a warning if this is not true
    auto n_components = network.strongly_connected_components().size();
    if( n_components != 1 )
    {
        fmt::print( "WARNING: You have {} strongly connected components in your network!\n", n_components );
    }

    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        network.agents[i].data.opinion = double( i ) / double( network.agents.size() );
    }
}

void DeGrootModel::iteration()
{
    Model<AgentT>::iteration();

    size_t j_index = 0;
    double weight  = 0.0;

    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        auto neighbour_buffer               = network.get_neighbours( i );
        auto weight_buffer                  = network.get_weights( i );
        agents_current_copy[i].data.opinion = 0.0;
        for( size_t j = 0; j < neighbour_buffer.size(); j++ )
        {
            j_index = neighbour_buffer[j];
            weight  = weight_buffer[j];
            agents_current_copy[i].data.opinion += weight * network.agents[j_index].data.opinion;
        }
    }

    max_opinion_diff = 0;
    // Update the original agent opinions
    for( std::size_t i = 0; i < network.agents.size(); i++ )
    {
        max_opinion_diff = std::max(
            max_opinion_diff, std::abs( network.agents[i].data.opinion - agents_current_copy[i].data.opinion ) );
        network.agents[i] = agents_current_copy[i];
    }
}

bool DeGrootModel::finished()
{
    bool converged = max_opinion_diff < convergence_tol;
    return Model<AgentT>::finished() || converged;
}

} // namespace Seldon