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

    // We need an output path for Simulation, but we won't write anything out there?
    fs::path output_dir_path = proj_root_path / fs::path( "test/output" );
    fs::create_directories( output_dir_path );

    // Zero step
    auto filename = fmt::format( "opinions_{}.txt", 0 );
    Seldon::IO::opinions_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );

    simulation.run( output_dir_path );
}