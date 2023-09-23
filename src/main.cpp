#include "models/DeGroot.hpp"
#include "simulation.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <argparse/argparse.hpp>
#include <string>
#include <util/io.hpp>

int main( int argc, char * argv[] )
{

    argparse::ArgumentParser program( "seldon" );

    program.add_argument( "config_file" ).help( "The config file to be used. Has to be in TOML format." );

    try
    {
        program.parse_args( argc, argv );
    }
    catch( const std::runtime_error & err )
    {
        fmt::print( stderr, "{}\n{}", err.what(), fmt::streamed( program ) );
        return 1;
    }

    auto config_file_path = program.get<std::string>( "config_file" );
    fmt::print( "Using input file: {}\n", config_file_path );

    auto simulation = Seldon::Simulation( config_file_path );

    Seldon::IO::network_to_dot_file( simulation.network, "network.dot" );

    Seldon::IO::simulation_state_to_file( simulation, fmt::format( "simulation{}.txt", 0 ) );
    for( size_t i = 1; i < 10; i++ )
    {
        simulation.model->run();
        Seldon::IO::simulation_state_to_file( simulation, fmt::format( "simulation{}.txt", i ) );
    }

    return 0;
}