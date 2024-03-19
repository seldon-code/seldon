#pragma once

#include "agent.hpp"
#include "agent_io.hpp"

namespace Seldon
{

struct ActivityAgentData
{
    double opinion    = 0;   // x_i
    double activity   = 0;   // a_i
    double reluctance = 1.0; // m_i
};

using ActivityAgent = Agent<ActivityAgentData>;

template<>
inline std::string agent_to_string<ActivityAgent>( const ActivityAgent & agent )
{
    return fmt::format( "{}, {}, {}", agent.data.opinion, agent.data.activity, agent.data.reluctance );
}

template<>
inline std::string opinion_to_string<ActivityAgent>( const ActivityAgent & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<>
inline ActivityAgent agent_from_string<ActivityAgent>( const std::string & str )
{
    ActivityAgent res{};

    auto pos_comma      = str.find_first_of( ',' );
    auto pos_next_comma = str.find( ',', pos_comma + 1 );

    res.data.opinion = std::stod( str.substr( 0, pos_comma ) );

    if( pos_next_comma == std::string::npos )
    {
        res.data.activity = std::stod( str.substr( pos_comma + 1, str.size() ) );
    }
    else
    {
        res.data.activity = std::stod( str.substr( pos_comma + 1, pos_next_comma ) );
    }

    if( pos_next_comma != std::string::npos )
    {
        res.data.reluctance = std::stod( str.substr( pos_next_comma + 1, str.size() ) );
    }

    return res;
};

template<>
inline std::vector<std::string> agent_to_string_column_names<ActivityAgent>()
{
    return { "opinion", "activity", "reluctance" };
}
} // namespace Seldon