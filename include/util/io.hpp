#pragma once
#include "fstream"
#include "network.hpp"
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <cstddef>
namespace Seldon
{
namespace IO
{

void network_to_dot_file( const Network & network, std::string file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::app );
    size_t n_agents = network.n_agents();
    auto buffer     = std::vector<size_t>();
    fmt::print( fs, "digraph G {{\n" );

    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        network.get_adjacencies( idx_agent, buffer );

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



} // namespace IO
} // namespace Seldon