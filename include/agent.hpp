#pragma once
#include "agent_base.hpp"
#include <fmt/format.h>
namespace Seldon
{

/* Agent<T> is a base class for actual agents with opinion type T, it needs to implement to_string and from_string*/
template<typename T>
class Agent : public AgentBase
{
public:
    using opinion_t = T;
    opinion_t opinion;
    Agent() = default;
    Agent( opinion_t opinion ) : opinion( opinion ) {}

    // virtual void from_string() override
    // {
    // }

    virtual std::string to_string() const override
    {
        return fmt::format( "{}", opinion );
    }

    ~Agent() = default;
};

} // namespace Seldon