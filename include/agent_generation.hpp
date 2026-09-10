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

/// Anchor each agent to the opinion it now holds.
///
/// Friedkin-Johnsen agents are pulled toward where they started for the whole
/// run, so the anchor is initial-condition state and belongs beside the
/// opinions rather than inside a model. An agent file that supplied both is
/// left alone: this runs only where the opinions were seeded here.
template<typename AgentT>
void anchor( std::vector<AgentT> & agents ) requires requires( AgentT agent )
{
    agent.data.initial_opinion = agent.data.opinion;
}
{
    for( auto & agent : agents )
    {
        agent.data.initial_opinion = agent.data.opinion;
    }
}

/// Agents with nothing to anchor, which is every model but one.
template<typename AgentT>
void anchor( std::vector<AgentT> & )
{
}

/// Agents whose opinion is not one number have nothing to spread over an
/// interval, and the models that use them carry their own initial condition.
///
/// The overload exists because the caller decides per model rather than per
/// agent type, so a call that is never taken still has to compile.
template<typename AgentT>
void ramp( std::vector<AgentT> & )
{
}

} // namespace Seldon::AgentGeneration
