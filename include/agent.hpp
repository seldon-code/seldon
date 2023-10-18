#pragma once
#include "agent_base.hpp"
#include <fmt/format.h>
namespace Seldon
{

/* Agent<T> is a base class for actual agents with data type T
(which contains an opinion and perhaps some other things),
it needs to implement to_string and from_string*/
template<typename T>
class Agent : public AgentBase
{
public:
    using data_t = T;
    data_t data;
    Agent() = default;
    Agent( data_t data ) : data( data ) {}

    void from_string( const std::string & str );

    std::string to_string() const override
    {
        return fmt::format( "{:.16f}", data );
    }
};

template<>
inline void Agent<double>::from_string( const std::string & str )
{
    data = std::stod( str );
}

} // namespace Seldon