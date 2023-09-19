#include <fmt/core.h>
#include <iostream>

#include "state.hpp"

#include <argparse/argparse.hpp>
#include <string>

#include "models/DeGroot.hpp"

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
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    auto config_file_path = program.get<std::string>( "config_file" );
    fmt::print( "Using input file: {}\n", config_file_path );

    // auto state = Seldon::State("conf.toml");
    // auto model = Seldon::DeGrootModel(state);
    // model.run();

    return 0;
}