#include "models/InertialModel.hpp"
#include "agents/inertial_agent.hpp"
#include "network.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <random>
#include <vector>

namespace Seldon
{

InertialModel::InertialModel( const Config::InertialSettings & settings, NetworkT & network, std::mt19937 & gen )
        : ActivityDrivenModelAbstract<InertialAgent>( settings, network, gen ),
          friction_coefficient( settings.friction_coefficient )
{
}

// X(t+dt)
// Also updates the position
void InertialModel::calc_position()
{
    // Calculating 'drift' = a(t)-friction
    get_euler_slopes( drift_t_buffer, [this]( size_t i ) { return network.agents[i].data.opinion; } );

    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        auto & agent_data    = network.agents[idx_agent].data;
        auto accleration     = drift_t_buffer[idx_agent] - friction_coefficient * agent_data.velocity;
        double next_position = agent_data.opinion + agent_data.velocity * dt + 0.5 * (accleration)*dt * dt;

        // Update the position to the new position
        agent_data.opinion = next_position;
    }
}

// V(t+dt)
// Also updates the velocity
void InertialModel::calc_velocity()
{
    // Calculating new 'drift'
    get_euler_slopes( drift_next_t_buffer, [this]( size_t i ) { return network.agents[i].data.opinion; } );

    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        auto & agent_data    = network.agents[idx_agent].data;
        double next_velocity = agent_data.velocity
                               + 0.5 * dt
                                     * ( drift_t_buffer[idx_agent] - friction_coefficient * agent_data.velocity
                                         + drift_next_t_buffer[idx_agent] );
        next_velocity /= 1.0 + 0.5 * friction_coefficient * dt;

        // Update velocity
        agent_data.velocity = next_velocity;
    }
}

void InertialModel::iteration()
{
    Model<AgentT>::iteration();

    update_network();

    // Use Velocity Verlet algorithm
    calc_position();
    calc_velocity();

    if( bot_present() )
    {
        for( size_t bot_idx = 0; bot_idx < n_bots; bot_idx++ )
        {
            network.agents[bot_idx].data.opinion = bot_opinion[bot_idx];
        }
    }
}

} // namespace Seldon