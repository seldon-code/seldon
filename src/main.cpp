#include <fmt/format.h>
#include <fmt/ostream.h>
#include <argparse/argparse.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include "main.hpp"
#include "run_model.hpp"

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

    run_model(config_file_path, agent_file ,network_file, output_dir_path);

    return 0;
}
