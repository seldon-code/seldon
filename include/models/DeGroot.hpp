#pragma once
#include "model.hpp"
#include "network.hpp"
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model<Agent<double>>
{

private:
    Network & network;
    std::vector<AgentT> agents_current_copy;

public:
    DeGrootModel( int n_agents, Network & network );

    void run() override;
};

} // namespace Seldon