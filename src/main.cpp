#include "config_parser.hpp"
#include "models/DeGroot.hpp"
#include "simulation.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <argparse/argparse.hpp>
#include <filesystem>
#include <memory>
#include <models/ActivityDrivenModel.hpp>
#include <string>
namespace fs = std::filesystem;

int main( int argc, char * argv[] )
{
    argparse::ArgumentParser program( "seldon" );

    program.add_argument( "config_file" ).help( "The config file to be used. Has to be in TOML format." );
    program.add_argument( "-o", "--output" )
        .help( "Specify the output directory. Defaults to `path/to/config_file/output`" );
    program.add_argument( "-a", "--agents" )
        .help( "Specify initial agent opinions in a file. Overwrites TOML config." );
    program.add_argument( "-n", "--network" ).help( "Specify initial network in a file. Overwrites TOML config." );

    try
    {
        program.parse_args( argc, argv );
    }
    catch( const std::runtime_error & err )
    {
        fmt::print( stderr, "{}\n{}", err.what(), fmt::streamed( program ) );
        return 1;
    }

    fs::path config_file_path                      = program.get<std::string>( "config_file" );
    std::optional<std::string> agent_file          = program.present<std::string>( "-a" );
    std::optional<std::string> network_file        = program.present<std::string>( "-n" );
    std::optional<std::string> output_dir_path_cli = program.present<std::string>( "-o" );
    fs::path output_dir_path                       = output_dir_path_cli.value_or( fs::path( "./output" ) );

    fmt::print( "=================================================================\n" );

    fmt::print( "Using input file: {}\n", config_file_path.string() );
    fmt::print( "Output directory path set to: {}\n", output_dir_path.string() );

    fs::create_directories( output_dir_path ); // Create the output directory

    auto simulation_options = Seldon::Config::parse_config_file( config_file_path.string() );
    Seldon::Config::validate_settings( simulation_options );
    Seldon::Config::print_settings( simulation_options );

    if( network_file.has_value() )
    {
        fmt::print( "Reading network from file {}\n", network_file.value() );
    }
    if( agent_file.has_value() )
    {
        fmt::print( "Reading agents from file {}\n", agent_file.value() );
    }

    std::unique_ptr<Seldon::SimulationInterface> simulation;

    if( simulation_options.model == Seldon::Config::Model::DeGroot )
    {
        simulation = std::make_unique<Seldon::Simulation<Seldon::DeGrootModel::AgentT>>(
            simulation_options, network_file, agent_file );
    }
    else if( simulation_options.model == Seldon::Config::Model::ActivityDrivenModel )
    {
        simulation = std::make_unique<Seldon::Simulation<Seldon::ActivityDrivenModel::AgentT>>(
            simulation_options, network_file, agent_file );
    }

    simulation->run( output_dir_path );

    return 0;
}
