#include "catch2/matchers/catch_matchers.hpp"
#include "models/DeffuantModel.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <config_parser.hpp>
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