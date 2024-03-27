#pragma once

#include "agents/activity_agent.hpp"
#include "agents/inertial_agent.hpp"
#include "config_parser.hpp"
#include "directed_network.hpp"
#include "model.hpp"
#include "models/ActivityDrivenModel.hpp"
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
    using AgentT           = InertialAgent;
    using DirectedNetworkT = DirectedNetwork<AgentT>;
    using WeightT          = typename DirectedNetworkT::WeightT;

    InertialModel(
        const Config::ActivityDrivenInertialSettings & settings, DirectedNetworkT & network, std::mt19937 & gen );

    void iteration() override;

private:
    double friction_coefficient = 1.0;
    std::vector<double> drift_t_buffer{};
    std::vector<double> drift_next_t_buffer{};

    void calc_velocity();
    void calc_position();
};

} // namespace Seldon