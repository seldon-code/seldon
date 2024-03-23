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

    InertialModel( const Config::InertialSettings & settings, NetworkT & network, std::mt19937 & gen );

    void iteration() override;

private:
    double friction_coefficient = 1.0;
    std::vector<double> drift_t_buffer{};
    std::vector<double> drift_next_t_buffer{};

    void calc_velocity();
    void calc_position();
};

} // namespace Seldon