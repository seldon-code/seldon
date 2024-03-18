#pragma once
#include "agent.hpp"
#include "agent_writer.hpp"
#include "model.hpp"
#include "network.hpp"
#include <vector>

namespace Seldon
{

struct DeGrootAgentData
{
    double opinion = 0; // x_i
};

template<>
inline std::string agent_to_string<Agent<DeGrootAgentData>>( const Agent<DeGrootAgentData> & agent )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<>
inline std::string opinion_to_string<Agent<DeGrootAgentData>>( const Agent<DeGrootAgentData> & agent )
{
    return agent_to_string( agent );
}

template<>
inline Agent<DeGrootAgentData> agent_from_string<Agent<DeGrootAgentData>>( const std::string & str )
{
    Agent<DeGrootAgentData> res{};
    res.data.opinion = std::stod( str );
    return res;
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
    double max_opinion_diff = 0;
    NetworkT & network;
    std::vector<AgentT> agents_current_copy;
};

} // namespace Seldon