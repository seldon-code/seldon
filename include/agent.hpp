#pragma once
#include <fmt/format.h>
namespace Seldon
{

/* Agent<T> is a base class for actual agents with data type T
(which contains an opinion and perhaps some other things),
it needs to implement to_string and from_string*/
template<typename T>
class Agent
{
public:
    using data_t = T;
    data_t data;
    Agent() = default;
    Agent( data_t data ) : data( data ) {}
    virtual ~Agent() = default;
};

} // namespace Seldon