#pragma once
#include "agents/simple_agent.hpp"
#include "config_parser.hpp"
#include "directed_network.hpp"
#include "model.hpp"
#include <optional>
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model<SimpleAgent>
{
public:
    using AgentT           = SimpleAgent;
    using DirectedNetworkT = DirectedNetwork<AgentT>;

    DeGrootModel( Config::DeGrootSettings settings, DirectedNetworkT & network );

    void iteration() override;
    bool finished() override;

private:
    double convergence_tol{};
    std::optional<double> max_opinion_diff = std::nullopt;
    DirectedNetworkT & network;
    std::vector<AgentT> agents_current_copy;
};

} // namespace Seldon