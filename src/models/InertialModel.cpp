#include "models/InertialModel.hpp"
#include "agents/inertial_agent.hpp"
#include "network.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <random>
#include <vector>

namespace Seldon
{

void InertialModel::iteration()
{
    Model<AgentT>::iteration();

    update_network();

    // Integrate the ODE using 4th order Runge-Kutta
    // k_1 =   hf(x_n,y_n)
    get_euler_slopes( k1_buffer, [this]( size_t i ) { return network.agents[i].data.opinion; } );
    // k_2  =   hf(x_n+1/2h,y_n+1/2k_1)
    get_euler_slopes(
        k2_buffer, [this]( size_t i ) { return network.agents[i].data.opinion + 0.5 * this->k1_buffer[i]; } );
    // k_3  =   hf(x_n+1/2h,y_n+1/2k_2)
    get_euler_slopes(
        k3_buffer, [this]( size_t i ) { return network.agents[i].data.opinion + 0.5 * this->k2_buffer[i]; } );
    // k_4  =   hf(x_n+h,y_n+k_3)
    get_euler_slopes( k4_buffer, [this]( size_t i ) { return network.agents[i].data.opinion + this->k3_buffer[i]; } );

    // Update the agent opinions
    for( size_t idx_agent = 0; idx_agent < network.n_agents(); ++idx_agent )
    {
        // y_(n+1) =   y_n+1/6k_1+1/3k_2+1/3k_3+1/6k_4+O(h^5)
        network.agents[idx_agent].data.opinion
            += ( k1_buffer[idx_agent] + 2 * k2_buffer[idx_agent] + 2 * k3_buffer[idx_agent] + k4_buffer[idx_agent] )
               / 6.0;
    }

    if( bot_present() )
    {
        for( size_t bot_idx = 0; bot_idx < n_bots; bot_idx++ )
        {
            network.agents[bot_idx].data.opinion = bot_opinion[bot_idx];
        }
    }
}

} // namespace Seldon