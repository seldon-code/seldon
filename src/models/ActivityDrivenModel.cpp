#include "models/ActivityDrivenModel.hpp"
#include "network.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <random>
#include <vector>

Seldon::ActivityAgentModel::ActivityAgentModel( int n_agents, Network & network, std::mt19937 & gen )
        : Model<Seldon::ActivityAgentModel::AgentT>( n_agents ),
          network( network ),
          contact_prob_list( std::vector<std::vector<Network::WeightT>>( n_agents ) ),
          gen( gen )
{
}

void Seldon::ActivityAgentModel::get_agents_from_power_law()
{
    std::uniform_real_distribution<> dis_opinion( -1, 1 ); // Opinion initial values
    power_law_distribution<> dist_activity( eps, gamma );
    auto mean_activity = dist_activity.mean();

    // Initial conditions for the opinions, initialize to [-1,1]
    // The activities should be drawn from a power law distribution
    for( size_t i = 0; i < agents.size(); i++ )
    {
        agents[i].data.opinion = dis_opinion( gen ); // Draw the opinion value

        if( !mean_activities )
        { // Draw from a power law distribution (1-gamma)/(1-eps^(1-gamma)) * a^(-gamma)
            agents[i].data.activity = dist_activity( gen );
        }
        else
        {
            agents[i].data.activity = mean_activity;
        }
    }

    if( bot_present )
    {
        for( size_t bot_idx = 0; bot_idx < n_bots; bot_idx++ )
        {
            agents[bot_idx].data.opinion  = bot_opinion[bot_idx];
            agents[bot_idx].data.activity = bot_activity[bot_idx];
        }
    }
}

void Seldon::ActivityAgentModel::update_network_probabilistic()
{
    std::uniform_real_distribution<> dis_activation( 0.0, 1.0 );
    std::uniform_real_distribution<> dis_reciprocation( 0.0, 1.0 );
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
            auto weight_callback = [idx_agent, this]( size_t j )
            {
                if( idx_agent == j ) // The agent does not contact itself
                    return 0.0;
                return std::pow(
                    std::abs( this->agents[idx_agent].data.opinion - this->agents[j].data.opinion ), -this->homophily );
            };

            int m_temp = this->m;

            if( bot_present && idx_agent < n_bots )
            {
                m_temp = bot_m[idx_agent];
            }

            Seldon::reservoir_sampling_A_ExpJ( m_temp, network.n_agents(), weight_callback, contacted_agents, gen );

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
        auto contacted_agents = network.get_neighbours( idx_agent );
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
}

void Seldon::ActivityAgentModel::update_network_mean()
{
    using WeightT = Network::WeightT;
    std::vector<WeightT> weights( network.n_agents(), 0.0 );

    // Set all weights to zero in the beginning
    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        network.set_weights( idx_agent, weights );
        contact_prob_list[idx_agent] = weights; // set to zero
    }

    auto probability_helper = []( double omega, size_t m )
    {
        double p = 0;
        for( size_t i = 1; i <= m; i++ )
            p += ( std::pow( -omega, i + 1 ) + omega ) / ( omega + 1 );
        return p;
    };

    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        // Implement the weight for the probability of agent `idx_agent` contacting agent `j`
        // Not normalised since this is taken care of by the reservoir sampling
        auto weight_callback = [idx_agent, this]( size_t j )
        {
            constexpr double tolerance = 1e-16;
            auto opinion_diff = std::abs( this->agents[idx_agent].data.opinion - this->agents[j].data.opinion );
            if( opinion_diff < tolerance )
                return 0.0;
            return std::pow( opinion_diff, -this->homophily );
        };

        double normalization = 0;
        for( size_t k = 0; k < network.n_agents(); k++ )
        {
            normalization += weight_callback( k );
        }

        // Go through all the neighbours of idx_agent
        // Calculate the probability of i contacting j (in 1 to m rounds, assuming
        // the agent is activated
        int m_temp = m;
        if( bot_present && idx_agent < n_bots )
        {
            m_temp = bot_m[idx_agent];
        }

        for( size_t j = 0; j < network.n_agents(); j++ )
        {
            double omega = weight_callback( j ) / normalization;
            // We can calculate the probability of i contacting j ( i->j )
            // Update contact prob_list (outgoing)
            contact_prob_list[idx_agent][j] = agents[idx_agent].data.activity * probability_helper( omega, m_temp );
        }
    }

    for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
    {
        // Calculate the actual weights and reciprocity
        for( size_t j = 0; j < network.n_agents(); j++ )
        {
            double prob_contact_ij = contact_prob_list[idx_agent][j]; // outgoing probabilites
            double prob_contact_ji = contact_prob_list[j][idx_agent];

            // Set the incoming agent weight, j-i in weight list
            double & win_ji = network.get_weights( j )[idx_agent];
            win_ji += prob_contact_ij;

            // Handle the reciprocity for j->i
            // Update incoming weight i-j
            double & win_ij = network.get_weights( idx_agent )[j];

            // The probability of reciprocating is
            win_ij += ( 1.0 - prob_contact_ji ) * reciprocity * prob_contact_ij;
        }
    }
}

void Seldon::ActivityAgentModel::update_network()
{

    if( !mean_weights )
    {
        update_network_probabilistic();
    }
    else
    {
        update_network_mean();
    }
}

void Seldon::ActivityAgentModel::iteration()
{
    Model<AgentT>::iteration();

    update_network();

    // Integrate the ODE using 4th order Runge-Kutta
    // k_1 =   hf(x_n,y_n)
    get_euler_slopes( k1_buffer, [this]( size_t i ) { return this->agents[i].data.opinion; } );
    // k_2  =   hf(x_n+1/2h,y_n+1/2k_1)
    get_euler_slopes(
        k2_buffer, [this]( size_t i ) { return this->agents[i].data.opinion + 0.5 * this->k1_buffer[i]; } );
    // k_3  =   hf(x_n+1/2h,y_n+1/2k_2)
    get_euler_slopes(
        k3_buffer, [this]( size_t i ) { return this->agents[i].data.opinion + 0.5 * this->k2_buffer[i]; } );
    // k_4  =   hf(x_n+h,y_n+k_3)
    get_euler_slopes( k4_buffer, [this]( size_t i ) { return this->agents[i].data.opinion + this->k3_buffer[i]; } );

    // Update the agent opinions
    for( size_t idx_agent = 0; idx_agent < network.n_agents(); ++idx_agent )
    {
        // y_(n+1) =   y_n+1/6k_1+1/3k_2+1/3k_3+1/6k_4+O(h^5)
        agents[idx_agent].data.opinion
            += ( k1_buffer[idx_agent] + 2 * k2_buffer[idx_agent] + 2 * k3_buffer[idx_agent] + k4_buffer[idx_agent] )
               / 6.0;
    }

    if( bot_present )
    {
        for( size_t bot_idx = 0; bot_idx < n_bots; bot_idx++ )
        {
            agents[bot_idx].data.opinion = bot_opinion[bot_idx];
        }
    }
}