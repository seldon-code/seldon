#pragma once

#include "agents/discrete_vector_agent.hpp"
#include "agents/simple_agent.hpp"
#include "config_parser.hpp"
#include "model.hpp"
#include "network.hpp"
#include "util/math.hpp"
#include <cstddef>
#include <random>

#include "network_generation.hpp"

#include <vector>

namespace Seldon
{

template<typename AgentT_>
class DeffuantModelAbstract : public Model<AgentT_>
{
public:
    using AgentT   = AgentT_;
    using NetworkT = Network<AgentT>;

    DeffuantModelAbstract( const Config::DeffuantSettings & settings, NetworkT & network, std::mt19937 & gen )
            : Model<AgentT>( settings.max_iterations ),
              homophily_threshold( settings.homophily_threshold ),
              mu( settings.mu ),
              use_network( settings.use_network ),
              network( network ),
              gen( gen )
    {
        // Generate the network as a square lattice if use_network is true
        if( use_network )
        {
            size_t n_edge = std::sqrt( network.n_agents() );
            if( n_edge * n_edge != network.n_agents() )
            {
                throw std::runtime_error( "Number of agents is not a square number." );
            }
            network = NetworkGeneration::generate_square_lattice<AgentT>( n_edge );
        }

        initialize_agents();
    }

    std::vector<std::size_t> select_interacting_agent_pair()
    {
        auto interacting_agents = std::vector<std::size_t>();
        // If the basic model is being used, then search from all possible agents
        if( !use_network )
        {

            // Pick any two agents to interact, randomly, without repetition
            draw_unique_k_from_n( std::nullopt, 2, network.n_agents(), interacting_agents, gen );

            return interacting_agents;
        }
        else
        {
            // First select an agent randomly
            auto dist       = std::uniform_int_distribution<size_t>( 0, network.n_agents() - 1 );
            auto agent1_idx = dist( gen );
            interacting_agents.push_back( agent1_idx );

            // Choose a neighbour randomly from the neighbour list of agent1_idx
            auto neighbours     = network.get_neighbours( agent1_idx );
            auto n_neighbours   = neighbours.size();
            auto dist_n         = std::uniform_int_distribution<size_t>( 0, n_neighbours - 1 );
            auto index_in_neigh = dist_n( gen ); // Index inside neighbours list
            auto agent2_idx     = neighbours[index_in_neigh];
            interacting_agents.push_back( agent2_idx );

            return interacting_agents;
        }
    }

    void iteration() override
    {
        Model<AgentT>::iteration(); // Update n_iterations

        // Although the model defines each iteration as choosing *one*
        // pair of agents, we will define each iteration as sampling
        // n_agents pairs (similar to the time unit in evolution plots in the paper)
        for( size_t i = 0; i < network.n_agents(); i++ )
        {

            auto interacting_agents = select_interacting_agent_pair();

            auto & agent1 = network.agents[interacting_agents[0]];
            auto & agent2 = network.agents[interacting_agents[1]];

            update_rule( agent1, agent2 );
        }
    }

    // template<typename T>
    void update_rule( AgentT & agent1, AgentT & agent2 );
    void initialize_agents();

    // void iteration() override;
    // bool finished() override;

private:
    double homophily_threshold{}; // d in paper
    double mu{};                  // convergence parameter
    bool use_network{};           // for the basic Deffuant model
    NetworkT & network;
    std::mt19937 & gen; // reference to simulation Mersenne-Twister engine
};

using DeffuantModel       = DeffuantModelAbstract<SimpleAgent>;
using DeffuantModelVector = DeffuantModelAbstract<DiscreteVectorAgent>;

} // namespace Seldon