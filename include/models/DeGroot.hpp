#pragma once
#include "agents/simple_agent.hpp"
#include "config_parser.hpp"
#include "model.hpp"
#include "network.hpp"
#include <optional>
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model<SimpleAgent>
{
public:
    using AgentT   = SimpleAgent;
    using NetworkT = Network<AgentT>;

    DeGrootModel( Config::DeGrootSettings settings, NetworkT & network );

    void iteration() override;
    bool finished() override;

private:
    double convergence_tol{};
    std::optional<double> max_opinion_diff = std::nullopt;
    NetworkT & network;
    std::vector<AgentT> agents_current_copy;
};

} // namespace Seldon