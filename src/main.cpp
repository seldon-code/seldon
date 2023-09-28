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

    // Output directory location
    program.add_argument( "-o", "--output" ).help( "Specify the output directory." );

    try
    {
        program.parse_args( argc, argv );
    }
    catch( const std::runtime_error & err )
    {
        fmt::print( stderr, "{}\n{}", err.what(), fmt::streamed( program ) );
        return 1;
    }

    fs::path config_file_path = program.get<std::string>( "config_file" );
    fmt::print( "Using input file: {}\n", config_file_path.string() );
    fs::path output_dir_path; // All outputs will be saved to a directory named output

    // If the output directory has not been specified,
    // set it to the parent directory where the config file is present
    if( auto fn = program.present( "-o" ) )
    {
        output_dir_path = *fn;
    } // set the output directory path
    else
    {
        output_dir_path = config_file_path.parent_path() / fs::path( "output" );
        fmt::print( "Output directory path set to: {}\n", output_dir_path.string() );
    } // if the output directory has not been provided

    // Get the output directory path
    fs::create_directories( output_dir_path ); // Create the output directory

    auto simulation = Seldon::Simulation( config_file_path.string() );

    Seldon::IO::network_to_dot_file( simulation.network, ( output_dir_path / fs::path( "network.dot" ) ).string() );

    auto filename = fmt::format( "iteration_{}.txt", 0 );
    Seldon::IO::simulation_state_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );
    for( size_t i = 1; i < 10; i++ )
    {
        simulation.model->run();
        filename = fmt::format( "iteration_{}.txt", i );
        Seldon::IO::simulation_state_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );
    }

    return 0;
}