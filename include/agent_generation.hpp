#pragma once
#include "util/misc.hpp"
#include <cstddef>
#include <vector>

namespace Seldon::AgentGeneration
{

/// Spread opinions evenly over [0, 1) in agent order.
///
/// An initial condition is a property of the run, not of the update rule, so
/// it belongs here rather than inside a model's constructor. A model that
/// assigns opinions on construction silently discards whatever the network or
/// an agent file supplied, and the caller has no way to see that it happened:
/// the run is well formed, the numbers are plausible, and they are not the
/// ones that were given.
///
/// The ramp itself is the same deterministic spread the models used to apply
/// on their own, so a run over a generated network gives what it always gave.
template<typename AgentT>
void ramp( std::vector<AgentT> & agents ) requires requires( AgentT agent )
{
    agent.data.opinion = 0.0;
}
{
    for( size_t i = 0; i < agents.size(); i++ )
    {
        agents[i].data.opinion = double( i ) / double( agents.size() );
    }
}

} // namespace Seldon::AgentGeneration
