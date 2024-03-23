#pragma once

#include "agents/activity_agent.hpp"
#include "agents/inertial_agent.hpp"
#include "config_parser.hpp"
#include "model.hpp"
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

template<typename AgentT_>
class ActivityDrivenModelAbstract : public Model<AgentT_>
{
public:
    using AgentT   = AgentT_;
    using NetworkT = Network<AgentT>;
    using WeightT  = typename NetworkT::WeightT;

    ActivityDrivenModelAbstract(
        const Config::ActivityDrivenSettings & settings, NetworkT & network, std::mt19937 & gen )
            : Model<AgentT>( settings.max_iterations ),
              network( network ),
              contact_prob_list( std::vector<std::vector<WeightT>>( network.n_agents() ) ),
              gen( gen ),
              dt( settings.dt ),
              m( settings.m ),
              eps( settings.eps ),
              gamma( settings.gamma ),
              alpha( settings.alpha ),
              homophily( settings.homophily ),
              reciprocity( settings.reciprocity ),
              K( settings.K ),
              mean_activities( settings.mean_activities ),
              mean_weights( settings.mean_weights ),
              use_reluctances( settings.use_reluctances ),
              reluctance_mean( settings.reluctance_mean ),
              reluctance_sigma( settings.reluctance_sigma ),
              reluctance_eps( settings.reluctance_eps ),
              n_bots( settings.n_bots ),
              bot_m( settings.bot_m ),
              bot_activity( settings.bot_activity ),
              bot_opinion( settings.bot_opinion ),
              bot_homophily( settings.bot_homophily )
    {
        get_agents_from_power_law();

        if( mean_weights )
        {
            auto agents_copy = network.agents;
            network          = NetworkGeneration::generate_fully_connected<AgentT>( network.n_agents() );
            network.agents   = agents_copy;
        }
    }

    void iteration() override {};

protected:
    NetworkT & network;

private:
    std::vector<std::vector<WeightT>> contact_prob_list; // Probability of choosing i in 1 to m rounds
    // Random number generation
    std::mt19937 & gen; // reference to simulation Mersenne-Twister engine
    std::set<std::pair<size_t, size_t>> reciprocal_edge_buffer{};

protected:
    // Model-specific parameters
    double dt{}; // Timestep for the integration of the coupled ODEs
    // Various free parameters
    int m{};            // Number of agents contacted, when the agent is active
    double eps{};       // Minimum activity epsilon; a_i belongs to [epsilon,1]
    double gamma{};     // Exponent of activity power law distribution of activities
    double alpha{};     // Controversialness of the issue, must be greater than 0.
    double homophily{}; // aka beta. if zero, agents pick their interaction partners at random
    // Reciprocity aka r. probability that when agent i contacts j via weighted reservoir sampling
    // j also sends feedback to i. So every agent can have more than m incoming connections
    double reciprocity{};
    double K{}; // Social interaction strength; K>0

    bool mean_activities = false;
    bool mean_weights    = false;

    bool use_reluctances = false;
    double reluctance_mean{};
    double reluctance_sigma{};
    double reluctance_eps{};
    double covariance_factor{};

    size_t n_bots                     = 0; // The first n_bots agents are bots
    std::vector<int> bot_m            = std::vector<int>( 0 );
    std::vector<double> bot_activity  = std::vector<double>( 0 );
    std::vector<double> bot_opinion   = std::vector<double>( 0 );
    std::vector<double> bot_homophily = std::vector<double>( 0 );

    // Buffers for RK4 integration
    std::vector<double> k1_buffer{};
    std::vector<double> k2_buffer{};
    std::vector<double> k3_buffer{};
    std::vector<double> k4_buffer{};

private:
    void get_agents_from_power_law()
    {
        std::uniform_real_distribution<> dis_opinion( -1, 1 ); // Opinion initial values
        power_law_distribution<> dist_activity( eps, gamma );
        truncated_normal_distribution<> dist_reluctance( reluctance_mean, reluctance_sigma, reluctance_eps );

        auto mean_activity = dist_activity.mean();

        // Initial conditions for the opinions, initialize to [-1,1]
        // The activities should be drawn from a power law distribution
        for( size_t i = 0; i < network.agents.size(); i++ )
        {
            network.agents[i].data.opinion = dis_opinion( gen ); // Draw the opinion value

            if( !mean_activities )
            { // Draw from a power law distribution (1-gamma)/(1-eps^(1-gamma)) * a^(-gamma)
                network.agents[i].data.activity = dist_activity( gen );
            }
            else
            {
                network.agents[i].data.activity = mean_activity;
            }

            if( use_reluctances )
            {
                network.agents[i].data.reluctance = dist_reluctance( gen );
            }
        }

        if( bot_present() )
        {
            for( size_t bot_idx = 0; bot_idx < n_bots; bot_idx++ )
            {
                network.agents[bot_idx].data.opinion  = bot_opinion[bot_idx];
                network.agents[bot_idx].data.activity = bot_activity[bot_idx];
            }
        }
    }

    // The weight for contact between two agents
    double homophily_weight( size_t idx_contacter, size_t idx_contacted )
    {
        double homophily = this->homophily;

        if( idx_contacted == idx_contacter )
            return 0.0;

        if( bot_present() && idx_contacter < n_bots )
            homophily = this->bot_homophily[idx_contacter];

        constexpr double tolerance = 1e-10;
        auto opinion_diff
            = std::abs( network.agents[idx_contacter].data.opinion - network.agents[idx_contacted].data.opinion );
        opinion_diff = std::max( tolerance, opinion_diff );

        return std::pow( opinion_diff, -homophily );
    }

    void update_network_probabilistic()
    {
        network.switch_direction_flag();

        std::uniform_real_distribution<> dis_activation( 0.0, 1.0 );
        std::uniform_real_distribution<> dis_reciprocation( 0.0, 1.0 );
        std::vector<size_t> contacted_agents{};
        reciprocal_edge_buffer.clear(); // Clear the reciprocal edge buffer
        for( size_t idx_agent = 0; idx_agent < network.n_agents(); idx_agent++ )
        {
            // Test if the agent is activated
            bool activated = dis_activation( gen ) < network.agents[idx_agent].data.activity;

            if( activated )
            {
                // Implement the weight for the probability of agent `idx_agent` contacting agent `j`
                // Not normalised since this is taken care of by the reservoir sampling

                int m_temp = this->m;

                if( bot_present() && idx_agent < n_bots )
                {
                    m_temp = bot_m[idx_agent];
                }

                reservoir_sampling_A_ExpJ(
                    m_temp, network.n_agents(), [&]( int j ) { return homophily_weight( idx_agent, j ); },
                    contacted_agents, gen );

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

        network.toggle_incoming_outgoing(); // switch direction, so that we have incoming edges
    }

    void update_network_mean()
    {
        using WeightT = NetworkT::WeightT;
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

            double normalization = 0;
            for( size_t k = 0; k < network.n_agents(); k++ )
            {
                normalization += homophily_weight( idx_agent, k );
            }

            // Go through all the neighbours of idx_agent
            // Calculate the probability of i contacting j (in 1 to m rounds, assuming
            // the agent is activated
            int m_temp = m;
            if( bot_present() && idx_agent < n_bots )
            {
                m_temp = bot_m[idx_agent];
            }

            double activity = std::max( 1.0, network.agents[idx_agent].data.activity );
            for( size_t j = 0; j < network.n_agents(); j++ )
            {
                double omega = homophily_weight( idx_agent, j ) / normalization;
                // We can calculate the probability of i contacting j ( i->j )
                // Update contact prob_list (outgoing)
                contact_prob_list[idx_agent][j] = activity * probability_helper( omega, m_temp );
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

protected:
    [[nodiscard]] bool bot_present() const
    {
        return n_bots > 0;
    }

    void update_network()
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

    template<typename Opinion_Callback>
    void get_euler_slopes( std::vector<double> & k_buffer, Opinion_Callback opinion )
    {
        // h is the timestep
        size_t j_index = 0;

        k_buffer.resize( network.n_agents() );

        for( size_t idx_agent = 0; idx_agent < network.n_agents(); ++idx_agent )
        {
            auto neighbour_buffer = network.get_neighbours( idx_agent ); // Get the incoming neighbours
            auto weight_buffer    = network.get_weights( idx_agent );    // Get incoming weights
            k_buffer[idx_agent]   = -opinion( idx_agent );
            // Loop through neighbouring agents
            for( size_t j = 0; j < neighbour_buffer.size(); j++ )
            {
                j_index = neighbour_buffer[j];
                k_buffer[idx_agent] += 1.0 / network.agents[idx_agent].data.reluctance * K * weight_buffer[j]
                                       * std::tanh( alpha * opinion( j_index ) );
            }
            // Here, we won't multiply by the timestep.
            // Instead multiply in the update rule
        }
    }
};

using ActivityDrivenModel = ActivityDrivenModelAbstract<ActivityAgent>;

} // namespace Seldon