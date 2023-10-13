#include "models/ActivityDrivenModel.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <random>
#include <stdexcept>

Seldon::ActivityAgentModel::ActivityAgentModel( int n_agents, Network & network, std::mt19937 & gen )
        : Model<Seldon::ActivityAgentModel::AgentT>( n_agents ),
          network( network ),
          agents_current_copy( std::vector<AgentT>( n_agents ) ),
          gen( gen )
{
    // Initial conditions for the opinions, initialize to [-1,1]
}

void Seldon::ActivityAgentModel::get_agents_from_power_law()
{
    std::uniform_real_distribution<> dis_opinion( -1, 1 );  // Opinion initial values
    std::uniform_real_distribution<> dist_activity( 0, 1 ); // Uniform random variable for activities

    // The activities should be drawn from a power law distribution
    for( size_t i = 0; i < agents.size(); i++ )
    {
        agents[i].data.opinion = dis_opinion( gen ); // Draw the opinion value

        // Draw from a power law distribution (1-gamma)/(1-eps^(1-gamma)) * a^(-gamma)
        agents[i].data.activity = std::pow(
            ( 1 - std::pow( eps, ( 1 - gamma ) ) ) * dist_activity( gen ) + std::pow( eps, ( 1 - gamma ) ),
            ( 1 / ( 1 - gamma ) ) );
    }
}

void Seldon::ActivityAgentModel::iteration()
{
    Model<AgentT>::iteration();

    std::uniform_real_distribution<> dis_activation( 0.0, 1.0 ); // Opinion initial values
    std::vector<size_t> contacted_agents{};

    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        // Test if the agent is activated
        bool activated = dis_activation( gen ) < agents[idx_agent].data.activity;

        if( activated )
        {
            // Contact m_agents according to the homophily distribution
            // TODO: use homophily stuff instead of uniform
            Seldon::draw_unique_k_from_n(
                idx_agent, m, network.n_agents(), contacted_agents,
                gen ); // now contacted_agents contains the indices of the contacted agents
            // Set the *outgoing* edges
            network.set_neighbours_and_weights( idx_agent, contacted_agents, 1.0 );
        }
        else
        {
            network.set_neighbours_and_weights( idx_agent, {}, {} );
        }

        // TODO: implement reciprocity
        // ...
    }

    network.transpose(); // transpose the network, so that we have incoming edges

    // Integrate the ODE
    auto neighbour_buffer = std::vector<size_t>();
    size_t j_index        = 0;

    agents_current_copy = agents; // Set the copy to the current state of agents. TODO: this is somewhat wasteful since
                                  // activities are not changing

    for( size_t i = 0; i < network.n_agents(); i++ )
    {
        network.get_neighbours( i, neighbour_buffer ); // Get the incoming neighbours
        for( size_t j = 0; j < neighbour_buffer.size(); j++ )
        {
            // TODO: currently this uses an euler integration -> use RK4
            j_index = neighbour_buffer[j];
            agents_current_copy[i].data.opinion
                += dt * ( -agents[i].data.opinion + K * ( std::tanh( alpha * agents[j_index].data.opinion ) ) );
        }
    }

    // Update the agents from the copy
    for( std::size_t i = 0; i < agents.size(); i++ )
    {
        max_opinion_diff
            = std::max( max_opinion_diff, std::abs( agents[i].data.opinion - agents_current_copy[i].data.opinion ) );
        agents[i] = agents_current_copy[i];
    }
}