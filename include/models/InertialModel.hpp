#pragma once

#include "agents/activity_agent.hpp"
#include "agents/inertial_agent.hpp"
#include "config_parser.hpp"
#include "model.hpp"
#include "models/ActivityDrivenModel.hpp"
#include "network.hpp"
#include "network_generation.hpp"
#include <cstddef>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Seldon
{

class InertialModel : public ActivityDrivenModelAbstract<InertialAgent>
{
public:
    using AgentT   = InertialAgent;
    using NetworkT = Network<AgentT>;
    using WeightT  = typename NetworkT::WeightT;

    void iteration() override;

private:
    double friction_coefficient = 1.0;
};

} // namespace Seldon