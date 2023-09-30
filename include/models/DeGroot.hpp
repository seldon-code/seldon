#pragma once
#include "model.hpp"
#include "network.hpp"
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model<Agent<double>>
{

private:
    double max_opinion_diff = 0;
    Network & network;
    std::vector<AgentT> agents_current_copy;

public:
    double convergence_tol = 1e-12;

    DeGrootModel( int n_agents, Network & network );

    void iteration() override;
    bool finished() override;
};

} // namespace Seldon