#include "models/DeGroot.hpp"
#include "simulation.hpp"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <argparse/argparse.hpp>
#include <chrono>
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

    // Seldon::IO::network_to_dot_file( *simulation.network, ( output_dir_path / fs::path( "network.dot" ) ).string() );

    Seldon::IO::network_to_file( simulation, ( output_dir_path / fs::path( "network_0.txt" ) ).string() );
    auto filename = fmt::format( "opinions_{}.txt", 0 );
    Seldon::IO::opinions_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );

    const std::optional<size_t> n_output_agents  = simulation.output_settings.n_output_agents;
    const std::optional<size_t> n_output_network = simulation.output_settings.n_output_network;

    typedef std::chrono::milliseconds ms;
    auto t_simulation_start = std::chrono::high_resolution_clock::now();
    do
    {
        auto t_iter_start = std::chrono::high_resolution_clock::now();

        simulation.model->iteration();

        auto t_iter_end = std::chrono::high_resolution_clock::now();
        auto iter_time  = std::chrono::duration_cast<ms>( t_iter_end - t_iter_start );

        // Print the iteration time?
        if( simulation.output_settings.print_progress )
        {
            fmt::print(
                "Iteration {}   iter_time = {:%Hh %Mm %Ss} \n", simulation.model->n_iterations,
                std::chrono::floor<ms>( iter_time ) );
        }

        // Write out the opinion?
        if( n_output_agents.has_value() && ( simulation.model->n_iterations % n_output_agents.value() == 0 ) )
        {
            filename = fmt::format( "opinions_{}.txt", simulation.model->n_iterations );
            Seldon::IO::opinions_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );
        }

        // Write out the network?
        if( n_output_network.has_value() && ( simulation.model->n_iterations % n_output_network.value() == 0 ) )
        {
            filename = fmt::format( "network_{}.txt", simulation.model->n_iterations );
            Seldon::IO::network_to_file( simulation, ( output_dir_path / fs::path( filename ) ).string() );
        }

    } while( !simulation.model->finished() );

    auto t_simulation_end = std::chrono::high_resolution_clock::now();
    auto total_time       = std::chrono::duration_cast<ms>( t_simulation_end - t_simulation_start );

    fmt::print( "-----------------------------------------------------------------\n" );
    fmt::print(
        "Finished after {} iterations, total time = {:%Hh %Mm %Ss}\n", simulation.model->n_iterations,
        std::chrono::floor<ms>( total_time ) );
    return 0;
}