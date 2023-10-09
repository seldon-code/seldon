#include "models/ActivityDrivenModel.hpp"
#include <random>

Seldon::ActivityAgentModel::ActivityAgentModel( int n_agents, Network & network )
        : Model<AgentT>( n_agents ), network( network ), agents_current_copy( std::vector<AgentT>( n_agents ) )
{   
    // TODO: take this from simulation 
    int rng_seed = std::random_device()();
    auto gen = std::mt19937( rng_seed ); // TODO
    std::uniform_int_distribution<> dis( -1, 1 ); // Opinion initial values 
    std::uniform_int_distribution<> dist_activity( 0, 1 ); // Uniform random variable for activities 

    // Initial conditions for the opinions, initialize to [-1,1]
    // The activities should be drawn from a power law distribution
    for( size_t i = 0; i < agents.size(); i++ )
    {
        agents[i].data.opinion = dis( gen ); // Draw the opinion value 
        // Draw from a power law distribution (1-gamma)/(1-eps^(1-gamma)) * a^(-gamma)
    }

     
}

