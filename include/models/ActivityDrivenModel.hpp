#pragma once

#include "agent.hpp"
#include "model.hpp"
#include "network.hpp"
#include <random>
#include <stdexcept>
#include <vector>

namespace Seldon
{

struct ActivityAgentData
{
    double opinion  = 0; // x_i
    double activity = 0; // a_i
};

template<>
inline std::string Agent<ActivityAgentData>::to_string() const
{
    return fmt::format( "{}, {}", data.opinion, data.activity );
};

template<>
inline void Agent<ActivityAgentData>::from_string( const std::string & str )
{
    // TODO
    throw std::runtime_error( "Agent<ActivityAgentData>::from_string not implemented yet" );
};

class ActivityAgentModel : public Model<Agent<ActivityAgentData>>
{
    using AgentT = Agent<ActivityAgentData>;

private:
    double max_opinion_diff = 0;
    Network & network;
    std::vector<AgentT> agents_current_copy;
    // Random number generation
    std::mt19937 & gen; // reference to simulation Mersenne-Twister engine

public:
    // Model-specific parameters
    double dt = 0.01; // Timestep for the integration of the coupled ODEs
    // Various free parameters
    int m            = 10;   // Number of agents contacted, when the agent is active
    double eps       = 0.01; // Minimum activity epsilon; a_i belongs to [epsilon,1]
    double gamma     = 2.1;  // Exponent of activity power law distribution of activities
    double alpha     = 3.0;  // Controversialness of the issue, must be greater than 0.
    double homophily = 0.5;  // aka beta. if zero, agents pick their interaction partners at random
    // Reciprocity aka r. probability that when agent i contacts j via weighted reservoir sampling
    // j also sends feedback to i. So every agent can have more than m incoming connections
    double reciprocity = 0.5;
    double K           = 3.0; // Social interaction strength; K>0

    double convergence_tol = 1e-12; // TODO: ??

    ActivityAgentModel( int n_agents, Network & network, std::mt19937 & gen );

    void get_agents_from_power_law(); // This needs to be called after eps and gamma have been set

    void iteration() override;

    // bool finished() overteration() override;
    // bool finished() override;
};

} // namespace Seldon