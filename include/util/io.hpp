#pragma once
#include "fstream"
#include "network.hpp"
#include "simulation.hpp"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <cstddef>
#include <string>
namespace Seldon
{
namespace IO
{

void network_to_dot_file( const Network & network, std::string file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    size_t n_agents = network.n_agents();
    auto buffer     = std::vector<size_t>();
    fmt::print( fs, "digraph G {{\n" );

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        network.get_neighbours( idx_agent, buffer );

        std::string row = fmt::format( "{} -> {{", idx_agent );
        for( size_t i = 0; i < buffer.size() - 1; i++ )
        {
            row += fmt::format( "{}, ", buffer[i] );
        }
        row += fmt::format( "{} }}\n", buffer[buffer.size() - 1] );

        fs << row;
    }
    fmt::print( fs, "}}\n" );
    fs.close();
}

void simulation_state_to_file( Simulation & simulation, std::string file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    auto & network  = simulation.network;
    auto & model    = simulation.model;
    size_t n_agents = network.n_agents();

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        std::string row = fmt::format( "{} {}\n", idx_agent, model->get_agent( idx_agent )->to_string() );
        fs << row;
    }
    fs.close();
}

} // namespace IO
} // namespace Seldon