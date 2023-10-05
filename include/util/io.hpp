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

inline void network_to_dot_file( const Network & network, std::string file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    size_t n_agents = network.n_agents();
    auto buffer     = std::vector<size_t>();
    fmt::print( fs, "digraph G {{\n" );

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        network.get_neighbours( idx_agent, buffer );

        std::string row = fmt::format( "{} <- {{", idx_agent );
        for( size_t i = 0; i < buffer.size() - 1; i++ )
        {
            row += fmt::format( "{}, ", buffer[i] );
        }
        row += fmt::format( "{}}}\n", buffer[buffer.size() - 1] );

        fs << row;
    }
    fmt::print( fs, "}}\n" );
    fs.close();
}

inline void opinions_to_file( Simulation & simulation, std::string file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    auto & network  = simulation.network;
    auto & model    = simulation.model;
    size_t n_agents = network->n_agents();

    fmt::print( fs, "# idx_agent opinion[...]\n" );
    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        std::string row = fmt::format( "{:<5} {:>25}\n", idx_agent, model->get_agent( idx_agent )->to_string() );
        fs << row;
    }
    fs.close();
}

inline void network_to_file( Simulation & simulation, std::string file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    auto & network  = *simulation.network;
    size_t n_agents = network.n_agents();

    fmt::print( fs, "# idx_agent n_neighbours_in indices_neighbours_in[...] weights_in[...]\n" );
    auto buffer_neighbours = std::vector<size_t>();
    auto buffer_weights    = std::vector<Network::WeightT>();

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        network.get_neighbours( idx_agent, buffer_neighbours );
        network.get_weights( idx_agent, buffer_weights );

        std::string row = fmt::format( "{:>5}, {:>5}, ", idx_agent, buffer_neighbours.size() );

        for( const auto & idx_neighbour : buffer_neighbours )
        {
            row += fmt::format( "{:>5}, ", idx_neighbour );
        }

        const auto n_weights = buffer_weights.size();
        for( size_t i_weight = 0; i_weight < n_weights; i_weight++ )
        {
            const auto & weight = buffer_weights[i_weight];
            if( i_weight == n_weights - 1 ) // At the end of a row
            {
                if( idx_agent == n_agents - 1 ) // At the end of the file
                {
                    row += fmt::format( "{:>25}", weight );
                }
                else
                {
                    row += fmt::format( "{:>25}\n", weight );
                }
            }
            else
            {
                row += fmt::format( "{:>25}, ", weight );
            }
        }

        fs << row;
    }
    fs.close();
}

} // namespace IO
} // namespace Seldon