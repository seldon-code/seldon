#include "catch2/matchers/catch_matchers.hpp"
#include "models/DeffuantModel.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <config_parser.hpp>
#include <cstddef>
#include <filesystem>
#include <network_io.hpp>
#include <optional>
#include <simulation.hpp>

namespace fs = std::filesystem;

TEST_CASE( "Test the basic deffuant model for two agents", "[deffuantTwoAgents]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;
    using AgentT = DeffuantModel::AgentT;

    auto proj_root_path = fs::current_path();
    auto input_file     = proj_root_path / fs::path( "test/res/deffuant_2agents.toml" );

    auto options = Config::parse_config_file( input_file.string() );

    auto simulation = Simulation<AgentT>( options, std::nullopt, std::nullopt );

    // We need an output path for Simulation, but we won't write anything out there
    fs::path output_dir_path = proj_root_path / fs::path( "test/output_deffuant" );

    auto model_settings      = std::get<Seldon::Config::DeffuantSettings>( options.model_settings );
    auto mu                  = model_settings.mu;
    auto homophily_threshold = model_settings.homophily_threshold;

    // references to agent opinion
    auto & agent1_opinion = simulation.network.agents[0].data.opinion;
    auto & agent2_opinion = simulation.network.agents[1].data.opinion;

    // agents are too far apart, we dont expect any change with the iterations
    double agent1_init = homophily_threshold * 1.1;
    double agent2_init = 0;

    agent1_opinion = agent1_init;
    agent2_opinion = agent2_init;

    simulation.run( output_dir_path );

    REQUIRE_THAT( agent1_opinion, WithinRel( agent1_init ) );
    REQUIRE_THAT( agent2_opinion, WithinRel( agent2_init ) );

    // agents are close enough, they should converge
    agent1_init = homophily_threshold * 0.9;
    agent2_init = 0;

    agent1_opinion = agent1_init;
    agent2_opinion = agent2_init;

    simulation.run( output_dir_path );

    auto n_iterations    = model_settings.max_iterations.value();
    double expected_diff = std::pow( 1.0 - 2.0 * mu, 2 * n_iterations ) * ( agent1_init - agent2_init );

    REQUIRE_THAT( agent1_opinion - agent2_opinion, WithinRel( expected_diff ) );
}

TEST_CASE( "Test the lattice deffuant model for 16x16 agents", "[deffuantLattice16x16]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;
    using AgentT = DeffuantModel::AgentT;

    auto proj_root_path = fs::current_path();
    auto input_file     = proj_root_path / fs::path( "test/res/deffuant_16x16_agents.toml" );

    auto options = Config::parse_config_file( input_file.string() );

    auto simulation = Simulation<AgentT>( options, std::nullopt, std::nullopt );

    // We need an output path for Simulation, but we won't write anything out there
    fs::path output_dir_path = proj_root_path / fs::path( "test/output_deffuant" );

    auto model_settings      = std::get<Seldon::Config::DeffuantSettings>( options.model_settings );
    auto homophily_threshold = model_settings.homophily_threshold;

    auto n_agents = simulation.network.n_agents();

    // first half with low opinions
    size_t n_agents_half = n_agents / 2;
    double avg_opinion   = 0;
    for( size_t idx_agent = 0; idx_agent < n_agents_half; idx_agent++ )
    {
        auto op = -homophily_threshold - 0.5 * idx_agent / n_agents * homophily_threshold;
        avg_opinion += op / double( n_agents_half );
        simulation.network.agents[idx_agent].data.opinion = op;
    }

    // second half with low opinions
    for( size_t idx_agent = n_agents_half; idx_agent < n_agents; idx_agent++ )
    {
        auto op = homophily_threshold + 0.5 * ( idx_agent - n_agents_half ) / n_agents * homophily_threshold;
        simulation.network.agents[idx_agent].data.opinion = op;
    }

    // The two halves are so far apart that they should not interact an therefore form two stable clusters.
    simulation.run( output_dir_path );

    for( size_t idx_agent = 0; idx_agent < n_agents_half; idx_agent++ )
    {
        REQUIRE_THAT( simulation.network.agents[idx_agent].data.opinion, WithinRel( avg_opinion ) );
    }

    for( size_t idx_agent = n_agents_half; idx_agent < n_agents; idx_agent++ )
    {
        REQUIRE_THAT( simulation.network.agents[idx_agent].data.opinion, WithinRel( -avg_opinion ) );
    }
}