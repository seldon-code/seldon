#pragma once

#include "agent.hpp"
#include "agent_io.hpp"
#include <util/misc.hpp>

namespace Seldon
{

struct InertialAgentData
{
    double opinion    = 0;   // x_i
    double activity   = 0;   // alpha_i
    double reluctance = 1.0; // m_i
    double velocity   = 0.0; // d(x_i)/dt
};

using InertialAgent = Agent<InertialAgentData>;

template<>
inline std::string agent_to_string<InertialAgent>( const InertialAgent & agent )
{
    return fmt::format(
        "{}, {}, {}, {}", agent.data.opinion, agent.data.velocity, agent.data.activity, agent.data.reluctance );
}

template<>
inline std::string opinion_to_string<InertialAgent>( const InertialAgent & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<>
inline InertialAgent agent_from_string<InertialAgent>( const std::string & str )
{
    InertialAgent res{};

    auto callback = [&]( int idx_list, std::string & substr )
    {
        if( idx_list == 0 )
        {
            res.data.opinion = std::stod( substr );
        }
        else if( idx_list == 1 )
        {
            res.data.velocity = std::stod( substr );
        }
        else if( idx_list == 2 )
        {
            res.data.activity = std::stod( substr );
        }
        else if( idx_list == 3 )
        {
            res.data.reluctance = std::stod( substr );
        }
    };

    Seldon::parse_comma_separated_list( str, callback );

    return res;
};

template<>
inline std::vector<std::string> agent_to_string_column_names<InertialAgent>()
{
    return { "opinion", "velocity", "activity", "reluctance" };
}
} // namespace Seldon