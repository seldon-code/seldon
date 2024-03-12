#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <config_parser.hpp>
#include <filesystem>
#include <optional>
#include <simulation.hpp>
namespace fs = std::filesystem;

#include <fmt/format.h>
#include <fmt/ostream.h>

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