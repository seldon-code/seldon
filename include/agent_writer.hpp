#pragma once
#include <fmt/format.h>

namespace Seldon
{

template<typename AgentT>
std::string agent_to_string( const AgentT & agent )
{
    static_assert( false, "Base implementation not valid" );
    return "";
}

template<typename AgentT>
std::string opinion_to_string( const AgentT & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

} // namespace Seldon