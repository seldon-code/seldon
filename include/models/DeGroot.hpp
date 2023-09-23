#pragma once
#include "model.hpp"
#include "network.hpp"
#include <vector>

namespace Seldon
{

class DeGrootModel : public Model
{

private:
    Network & network;

public:
    DeGrootModel( int n_agents, Network & network );

    void run() override;
};

} // namespace Seldon