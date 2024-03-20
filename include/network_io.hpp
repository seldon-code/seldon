#pragma once
#include "fstream"
#include "network.hpp"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <cstddef>
#include <string>
namespace Seldon
{

template<typename AgentT>
void network_to_dot_file( const Network<AgentT> & network, const std::string & file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    size_t n_agents = network.n_agents();
    fmt::print( fs, "digraph G {{\n" );

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        auto buffer = network.get_neighbours( idx_agent );

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

template<typename AgentT>
void network_to_file( const Network<AgentT> & network, const std::string & file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    size_t n_agents = network.n_agents();

    fmt::print( fs, "# idx_agent, n_neighbours_in, indices_neighbours_in[...], weights_in[...]\n" );

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        auto buffer_neighbours = network.get_neighbours( idx_agent );
        auto buffer_weights    = network.get_weights( idx_agent );

        std::string row = fmt::format( "{:>5}, {:>5}", idx_agent, buffer_neighbours.size() );

        if( buffer_neighbours.empty() )
        {
            row += "\n";
        }
        else
        {
            row += ", ";
        }

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

} // namespace Seldon