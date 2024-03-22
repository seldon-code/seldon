#pragma once

#include "agent.hpp"
#include "agent_io.hpp"
#include "util/misc.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace Seldon
{

struct DiscreteVectorAgentData
{
    std::vector<int> opinion{};
};

using DiscreteVectorAgent = Agent<DiscreteVectorAgentData>;

template<>
inline std::string agent_to_string<DiscreteVectorAgent>( const DiscreteVectorAgent & agent )
{
    if( agent.data.opinion.empty() )
        return "";

    auto res = fmt::format( "{}", agent.data.opinion[0] );
    for( size_t i = 1; i < agent.data.opinion.size(); i++ )
    {
        res += fmt::format( ", {}", agent.data.opinion[i] );
    }
    return res;
}

template<>
inline std::string opinion_to_string<DiscreteVectorAgent>( const DiscreteVectorAgent & agent )
{
    return agent_to_string( agent );
}

template<>
inline DiscreteVectorAgent agent_from_string<DiscreteVectorAgent>( const std::string & str )
{
    DiscreteVectorAgent res{};

    auto callback = [&]( int idx_list [[maybe_unused]], const auto & substring )
    { res.data.opinion.push_back( std::stoi( substring ) ); };

    parse_comma_separated_list( str, callback );
    return res;
};

// template<>
// inline std::vector<std::string> agent_to_string_column_names<ActivityAgent>()
// {
//     return { "opinion", "activity", "reluctance" };
// }

} // namespace Seldon