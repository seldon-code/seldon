#pragma once
#include <fmt/format.h>
#include <vector>

namespace Seldon
{

template<typename AgentT>
std::string agent_to_string( const AgentT & agent )
{
    return "";
}

template<typename AgentT>
std::string opinion_to_string( const AgentT & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<typename AgentT>
AgentT agent_from_string( const std::string & str )
{
    return AgentT{};
}

template<typename AgentT>
std::vector<std::string> agent_to_string_column_names()
{
    return { "agent_data[...]" };
}

} // namespace Seldon