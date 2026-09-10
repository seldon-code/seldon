#pragma once

#include "agent.hpp"
#include "agent_io.hpp"
#include <util/misc.hpp>

namespace Seldon
{

/// An agent that remembers where it started.
///
/// DeGroot agents carry only their current opinion, because averaging is all
/// that happens to them and the past is gone once it has been averaged in. A
/// Friedkin-Johnsen agent is pulled toward what it thought at the outset for
/// as long as the simulation runs, so its initial opinion is state rather than
/// an initial condition.
struct FriedkinJohnsenAgentData
{
    double opinion = 0; // x_i(t)
    /// Where this agent started, x_i(0). The anchor the susceptibility pulls
    /// against, held for the whole run rather than consumed by the first step.
    double initial_opinion = 0;
    /// lambda_i in [0, 1]: how much of the next opinion comes from listening.
    /// The rest, 1 - lambda_i, is stubbornness. At 1 an agent is a DeGroot
    /// agent, at 0 it never moves.
    double susceptibility = 0.5;
};

using FriedkinJohnsenAgent = Agent<FriedkinJohnsenAgentData>;

template<>
inline std::string agent_to_string<FriedkinJohnsenAgent>( const FriedkinJohnsenAgent & agent )
{
    return fmt::format( "{}, {}, {}", agent.data.opinion, agent.data.initial_opinion, agent.data.susceptibility );
}

template<>
inline std::string opinion_to_string<FriedkinJohnsenAgent>( const FriedkinJohnsenAgent & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<>
inline FriedkinJohnsenAgent agent_from_string<FriedkinJohnsenAgent>( const std::string & str )
{
    FriedkinJohnsenAgent res{};
    bool anchored = false;

    auto callback = [&]( int idx_list, std::string & substr )
    {
        if( idx_list == 0 )
        {
            res.data.opinion = std::stod( substr );
        }
        else if( idx_list == 1 )
        {
            res.data.initial_opinion = std::stod( substr );
            anchored                 = true;
        }
        else if( idx_list == 2 )
        {
            res.data.susceptibility = std::stod( substr );
        }
    };

    Seldon::parse_comma_separated_list( str, callback );

    // A file that gives only an opinion is giving a starting point, so that is
    // what the agent is anchored to. Reading the anchor as zero would make
    // every such agent stubborn about an opinion it never held.
    if( !anchored )
    {
        res.data.initial_opinion = res.data.opinion;
    }

    return res;
};

template<>
inline std::vector<std::string> agent_to_string_column_names<FriedkinJohnsenAgent>()
{
    return { "opinion", "initial_opinion", "susceptibility" };
}
} // namespace Seldon
