#include "catch2/matchers/catch_matchers.hpp"
#include "models/ActivityDrivenModel.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <config_parser.hpp>
#include <filesystem>
#include <optional>
#include <simulation.hpp>
namespace fs = std::filesystem;

#include <util/io.hpp>

TEST_CASE(
    "Test that you can produce output for the probabilistic acitivity driven model, from a conf file",
    "[activityProb]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto proj_root_path = fs::current_path();

    auto input_file = proj_root_path / fs::path( "test/res/activity_probabilistic_conf.toml" );

    auto options = Config::parse_config_file( input_file.string() );

    auto simulation = Simulation( options, std::nullopt, std::nullopt );

    fs::path output_dir_path = proj_root_path / fs::path( "test/output" );

    // Create the output directory; this is done by main
    fs::remove_all( output_dir_path );
    fs::create_directories( output_dir_path );

    // This should be empty
    REQUIRE( fs::is_empty( output_dir_path ) );

    simulation.run( output_dir_path );

    // The output directory should have some output files now
    REQUIRE_FALSE( fs::is_empty( output_dir_path ) );

    // Cleanup
    fs::remove_all( output_dir_path );
}

TEST_CASE( "Test the probabilistic activity driven model for two agents", "[activityProbTwoAgents]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto proj_root_path = fs::current_path();
    auto input_file     = proj_root_path / fs::path( "test/res/2_agents_activity_prob.toml" );

    auto options = Config::parse_config_file( input_file.string() );

    auto simulation = Simulation( options, std::nullopt, std::nullopt );

    // We need an output path for Simulation, but we won't write anything out there
    fs::path output_dir_path = proj_root_path / fs::path( "test/output" );

    simulation.run( output_dir_path );

    auto model_settings = std::get<Seldon::Config::ActivityDrivenSettings>( options.model_settings );
    auto K              = model_settings.K;
    auto alpha          = model_settings.alpha;

    // Check that the parameters match our assumptions for the numerical solution
    REQUIRE_THAT( K, WithinAbs( 2.0, 1e-16 ) );
    REQUIRE_THAT( alpha, WithinAbs( 1.01, 1e-16 ) );

    // This is the solution of x = K tanh(alpha x)
    double analytical_x = 1.9187384098662013;

    fmt::print( "analytical_x = {}\n", analytical_x );

    for( size_t idx_agent = 0; idx_agent < simulation.network->n_agents(); idx_agent++ )
    {
        auto * agent = simulation.model->get_agent_as<ActivityAgentModel::AgentT>( idx_agent );
        fmt::print( "{} \n", agent->data.opinion );
        REQUIRE_THAT( agent->data.opinion, WithinAbs( analytical_x, 1e-4 ) );
    }
}

TEST_CASE( "Test the probabilistic activity driven model with one bot and one agent", "[activity1Bot1Agent]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto proj_root_path = fs::current_path();
    auto input_file     = proj_root_path / fs::path( "test/res/1bot_1agent_activity_prob.toml" );

    auto options = Config::parse_config_file( input_file.string() );

    auto simulation = Simulation( options, std::nullopt, std::nullopt );

    // We need an output path for Simulation, but we won't write anything out there
    fs::path output_dir_path = proj_root_path / fs::path( "test/output" );

    // Get the bot opinion (which won't change)
    auto * bot = simulation.model->get_agent_as<ActivityAgentModel::AgentT>( 0 );
    auto x_bot = bot->data.opinion; // Bot opinion
    // Get the initial agent opinion
    auto * agent        = simulation.model->get_agent_as<ActivityAgentModel::AgentT>( 1 );
    agent->data.opinion = 1.0;
    auto x_0            = agent->data.opinion;
    fmt::print( "We have set agent x_0 = {}\n", x_0 );

    simulation.run( output_dir_path );

    auto model_settings = std::get<Seldon::Config::ActivityDrivenSettings>( options.model_settings );
    auto K              = model_settings.K;
    auto alpha          = model_settings.alpha;
    auto iterations     = model_settings.max_iterations.value();
    auto dt             = model_settings.dt;
    auto time_elapsed   = iterations * dt;

    // Final agent and bot opinions after the simulation run
    auto x_t     = agent->data.opinion;
    auto x_t_bot = bot->data.opinion;

    // The bot opinion should not change during the simulation
    REQUIRE_THAT( x_t_bot, WithinAbs( x_bot, 1e-16 ) );

    // Test that the agent opinion matches the analytical solution for an agent with a bot
    // Analytical solution is:
    // x_t = [x(0) - Ktanh(alpha*x_bot)]e^(-t) + Ktanh(alpha*x_bot)
    auto x_t_analytical = ( x_0 - K * tanh( alpha * x_bot ) ) * exp( -time_elapsed ) + K * tanh( alpha * x_bot );

    REQUIRE_THAT( x_t, WithinAbs( x_t_analytical, 1e-6 ) );
}