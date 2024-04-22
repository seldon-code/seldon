#include "catch2/matchers/catch_matchers.hpp"
#include "models/InertialModel.hpp"
#include "util/math.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <complex>
#include <config_parser.hpp>
#include <filesystem>
#include <network_io.hpp>
#include <optional>
#include <simulation.hpp>

namespace fs = std::filesystem;

TEST_CASE( "Test the probabilistic inertial activity driven model with one bot and one agent", "[inertial1Bot1Agent]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;
    using AgentT = InertialModel::AgentT;

    auto proj_root_path = fs::current_path();
    auto input_file     = proj_root_path / fs::path( "test/res/1bot_1agent_inertial.toml" );

    auto options = Config::parse_config_file( input_file.string() );
    Config::print_settings( options );

    auto simulation = Simulation<AgentT>( options, std::nullopt, std::nullopt );

    // We need an output path for Simulation, but we won't write anything out there
    fs::path output_dir_path = proj_root_path / fs::path( "test/output_inertial" );

    // Get the bot opinion (which won't change)
    auto bot   = simulation.network.agents[0];
    auto x_bot = bot.data.opinion; // Bot opinion
    fmt::print( "We have set the bot opinion = {}\n", x_bot );

    // Get the initial agent opinion
    auto & agent = simulation.network.agents[1];
    auto x_0     = agent.data.opinion; // Agent opinion
    fmt::print( "We have set agent x_0 = {}\n", x_0 );

    simulation.run( output_dir_path );

    auto model_settings     = std::get<Seldon::Config::ActivityDrivenInertialSettings>( options.model_settings );
    auto K                  = model_settings.K;
    auto alpha              = model_settings.alpha;
    auto iterations         = model_settings.max_iterations.value();
    auto dt                 = model_settings.dt;
    std::complex<double> mu = model_settings.friction_coefficient;
    auto time_elapsed       = iterations * dt;

    // Final agent and bot opinions after the simulation run
    auto x_t        = agent.data.opinion;
    auto x_t_bot    = bot.data.opinion;
    auto reluctance = agent.data.reluctance;

    fmt::print( "Agent reluctance is = {}\n", reluctance );

    // The bot opinion should not change during the simulation
    REQUIRE_THAT( x_t_bot, WithinAbs( x_bot, 1e-16 ) );

    // C = K/m tanh (alpha*x_bot)
    auto C = K / reluctance * std::tanh( alpha * x_bot );

    fmt::print( "C =     {}\n", C );
    auto a1 = 0.5 * ( -std::sqrt( mu * mu - 4.0 ) - mu );
    auto a2 = 0.5 * ( std::sqrt( mu * mu - 4.0 ) - mu );
    auto c1 = ( x_0 - C ) / ( 1.0 - a1 / a2 );
    auto c2 = -c1 * a1 / a2;
    // Test that the agent opinion matches the analytical solution for an agent with a bot
    // Analytical solution is
    // x_t = c1 * exp(a1*t) + c2 *exp(a2*t) + C
    auto x_t_analytical = c1 * std::exp( a1 * time_elapsed ) + c2 * std::exp( a2 * time_elapsed ) + C;

    REQUIRE_THAT( x_t, WithinAbs( x_t_analytical.real(), 1e-5 ) );
}