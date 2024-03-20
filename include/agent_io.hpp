#pragma once
#include "fstream"
#include "network.hpp"
#include "util/misc.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <vector>

namespace Seldon
{

template<typename AgentT>
[[nodiscard]] std::string agent_to_string( const AgentT & agent [[maybe_unused]] )
{
    return "";
}

template<typename AgentT>
[[nodiscard]] std::string opinion_to_string( const AgentT & agent [[maybe_unused]] )
{
    return fmt::format( "{}", agent.data.opinion );
}

template<typename AgentT>
[[nodiscard]] AgentT agent_from_string( const std::string & str [[maybe_unused]] )
{
    return AgentT{};
}

template<typename AgentT>
[[nodiscard]] std::vector<std::string> agent_to_string_column_names()
{
    return { "agent_data[...]" };
}

template<typename AgentT>
void agents_to_file( const Network<AgentT> & network, const std::string & file_path )
{
    std::fstream fs;
    fs.open( file_path, std::fstream::in | std::fstream::out | std::fstream::trunc );

    size_t n_agents = network.n_agents();

    auto column_names = agent_to_string_column_names<AgentT>();

    std::string header = "# idx_agent";
    for( auto col : column_names )
    {
        header += ", " + col;
    }
    header += "\n";

    fmt::print( fs, "{}", header );
    for( size_t idx_agent = 0; idx_agent < n_agents; idx_agent++ )
    {
        std::string row = fmt::format( "{:>5}, {:>25}\n", idx_agent, agent_to_string( network.agents[idx_agent] ) );
        fs << row;
    }
    fs.close();
}

template<typename AgentT>
std::vector<AgentT> agents_from_file( const std::string & file )
{
    std::vector<AgentT> agents{};

    std::string file_contents = get_file_contents( file );
    bool finished             = false;
    size_t start_of_line      = 0;

    while( !finished )
    {
        // Find the end of the current line
        auto end_of_line = file_contents.find( '\n', start_of_line );
        if( end_of_line == std::string::npos )
        {
            finished = true;
        }
        // Get the current line as a substring
        auto line     = file_contents.substr( start_of_line, end_of_line - start_of_line );
        start_of_line = end_of_line + 1;

        if( line.empty() )
        {
            break;
        }
        if( line[0] == '#' )
        {
            continue;
        }

        // First column is the index of the agent
        auto end_of_first_column = line.find( ',', 0 );
        auto opinion_substring   = line.substr( end_of_first_column + 1, end_of_line );

        agents.push_back( agent_from_string<AgentT>( opinion_substring ) );
    }

    return agents;
}

} // namespace Seldon