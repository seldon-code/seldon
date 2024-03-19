#pragma once

#include "agent.hpp"
#include "agent_io.hpp"

namespace Seldon
{

struct SimpleAgentData
{
    double opinion = 0; // x_i
};

using SimpleAgent = Agent<SimpleAgentData>;

template<>
inline std::string agent_to_string<SimpleAgent>( const SimpleAgent & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<>
inline std::string opinion_to_string<SimpleAgent>( const SimpleAgent & agent )
{
    return agent_to_string( agent );
}

template<>
inline SimpleAgent agent_from_string<SimpleAgent>( const std::string & str )
{
    SimpleAgent res{};
    res.data.opinion = std::stod( str );
    return res;
}

template<>
inline std::vector<std::string> agent_to_string_column_names<SimpleAgent>()
{
    return { "opinion" };
}
} // namespace Seldon