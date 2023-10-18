#include "models/DeGroot.hpp"
#include "simulation.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <argparse/argparse.hpp>
#include <filesystem>
#include <string>
#include <util/io.hpp>
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

    fmt::print( "Using input file: {}\n", config_file_path.string() );
    fmt::print( "Output directory path set to: {}\n", output_dir_path.string() );

    fs::create_directories( output_dir_path ); // Create the output directory

    auto simulation = Seldon::Simulation( config_file_path.string(), network_file, agent_file );

    fmt::print( "{}", simulation.model->get_agent( 0 )->to_string() );

    Seldon::IO::network_to_dot_file( *simulation.network, ( output_dir_path / fs::path( "network.dot" ) ).string() );
    Seldon::IO::network_to_file( simulation, ( output_dir_path / fs::path( "network.txt" ) ).string() );
    auto filename = fmt::format( "opinions_{}.txt", 0 );
    Seldon::IO::opinions_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );

    do
    {
        simulation.model->iteration();
        filename = fmt::format( "opinions_{}.txt", simulation.model->n_iterations );
        Seldon::IO::opinions_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );
    } while( !simulation.model->finished() );

    return 0;
}