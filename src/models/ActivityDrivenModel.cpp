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

    std::uniform_real_distribution<> dis_activation( 0.0, 1.0 );    // Opinion initial values
    std::uniform_real_distribution<> dis_reciprocation( 0.0, 1.0 ); // Opinion initial values
    std::vector<size_t> contacted_agents{};

    reciprocal_edge_buffer.clear(); // Clear the reciprocal edge buffer

    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        // Test if the agent is activated
        bool activated = dis_activation( gen ) < agents[idx_agent].data.activity;

        if( activated )
        {

            // Implement the weight for the probability of agent `idx_agent` contacting agent `j`
            // Not normalised since this is taken care of by the reservoir sampling
            auto weight_callback = [idx_agent, this]( size_t j ) {
                if( idx_agent == j ) // The agent does not contact itself
                    return 0.0;
                return std::pow(
                    std::abs( this->agents[idx_agent].data.opinion - this->agents[j].data.opinion ), -this->homophily );
            };

            Seldon::reservoir_sampling_A_ExpJ( m, network.n_agents(), weight_callback, contacted_agents, gen );

            // Fill the outgoing edges into the reciprocal edge buffer
            for( const auto & idx_outgoing : contacted_agents )
            {
                reciprocal_edge_buffer.insert(
                    { idx_agent, idx_outgoing } ); // insert the edge idx_agent -> idx_outgoing
            }

            // Set the *outgoing* edges
            network.set_neighbours_and_weights( idx_agent, contacted_agents, 1.0 );
        }
        else
        {
            network.set_neighbours_and_weights( idx_agent, {}, {} );
        }
    }

    // Reciprocity check
    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        // Get the outgoing edges
        network.get_neighbours( idx_agent, contacted_agents );
        // For each outgoing edge we check if the reverse edge already exists
        for( const auto & idx_outgoing : contacted_agents )
        {
            // If the edge is not reciprocated
            if( !reciprocal_edge_buffer.contains( { idx_outgoing, idx_agent } ) )
            {
                if( dis_reciprocation( gen ) < reciprocity )
                {
                    network.push_back_neighbour_and_weight( idx_outgoing, idx_agent, 1.0 );
                }
            }
        }
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