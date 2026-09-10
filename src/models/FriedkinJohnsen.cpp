#include "models/FriedkinJohnsen.hpp"
#include "config_parser.hpp"
#include <algorithm>
#include <cmath>

namespace Seldon
{

FriedkinJohnsenModel::FriedkinJohnsenModel( Config::FriedkinJohnsenSettings settings, NetworkT & network )
        : Model<AgentT>( settings.max_iterations ),
          convergence_tol( settings.convergence_tol ),
          network( network ),
          agents_current_copy( network.agents )
{
    // Unlike DeGroot, a disconnected network is not a warning here. Components
    // that never hear each other settling on different opinions is a thing
    // this model is for, not a configuration mistake.
    // The anchor is not set here. It is part of the initial condition, so it
    // comes from the agent file that supplied it or from the seeding the
    // Simulation does for a generated network. A constructor that copied the
    // current opinion over it would destroy the anchor of any run resumed from
    // a state where the two had already diverged, which is every run of this
    // model past its first step.
    if( settings.susceptibility.has_value() )
    {
        for( auto & agent : network.agents )
        {
            agent.data.susceptibility = settings.susceptibility.value();
        }
    }
    agents_current_copy = network.agents;
}

void FriedkinJohnsenModel::iteration()
{
    Model<AgentT>::iteration();

    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        auto neighbour_buffer = network.get_neighbours( i );
        auto weight_buffer    = network.get_weights( i );

        double heard = 0.0;
        for( size_t j = 0; j < neighbour_buffer.size(); j++ )
        {
            heard += weight_buffer[j] * network.agents[neighbour_buffer[j]].data.opinion;
        }

        const double lambda = network.agents[i].data.susceptibility;
        agents_current_copy[i].data.opinion
            = lambda * heard + ( 1.0 - lambda ) * network.agents[i].data.initial_opinion;
    }

    max_opinion_diff = 0;
    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        max_opinion_diff = std::max(
            max_opinion_diff.value(),
            std::abs( network.agents[i].data.opinion - agents_current_copy[i].data.opinion ) );
        network.agents[i] = agents_current_copy[i];
    }
}

bool FriedkinJohnsenModel::finished()
{
    bool converged = false;

    if( max_opinion_diff.has_value() )
        converged = max_opinion_diff.value() < convergence_tol;

    return Model<AgentT>::finished() || converged;
}

double FriedkinJohnsenModel::opinion_spread() const
{
    if( network.agents.empty() )
    {
        return 0.0;
    }
    auto [low, high] = std::minmax_element(
        network.agents.begin(), network.agents.end(),
        []( const AgentT & one, const AgentT & two ) { return one.data.opinion < two.data.opinion; } );
    return high->data.opinion - low->data.opinion;
}

} // namespace Seldon
