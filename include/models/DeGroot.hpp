#pragma once
#include "agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model<Agent<double>>
{
public:
    using AgentT           = Agent<double>;
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