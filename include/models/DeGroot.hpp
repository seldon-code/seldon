#pragma once
#include "agent.hpp"
#include "agent_io.hpp"
#include "model.hpp"
#include "network.hpp"
#include <optional>
#include <vector>

namespace Seldon
{

struct DeGrootAgentData
{
    double opinion = 0; // x_i
};

using AgentDeGroot = Agent<DeGrootAgentData>;

template<>
inline std::string agent_to_string<AgentDeGroot>( const AgentDeGroot & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<>
inline std::string opinion_to_string<AgentDeGroot>( const AgentDeGroot & agent )
{
    return agent_to_string( agent );
}

template<>
inline AgentDeGroot agent_from_string<AgentDeGroot>( const std::string & str )
{
    AgentDeGroot res{};
    res.data.opinion = std::stod( str );
    return res;
}

template<>
inline std::vector<std::string> agent_to_string_column_names<AgentDeGroot>()
{
    return { "opinion" };
}

class DeGrootModel : public Model<Agent<DeGrootAgentData>>
{
public:
    using AgentT           = Agent<DeGrootAgentData>;
    using NetworkT         = Network<AgentT>;
    double convergence_tol = 1e-12;

    DeGrootModel( NetworkT & network );

    void iteration() override;
    bool finished() override;

private:
    std::optional<double> max_opinion_diff = std::nullopt;
    NetworkT & network;
    std::vector<AgentT> agents_current_copy;
};

} // namespace Seldon