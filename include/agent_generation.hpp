#pragma once
#include "util/misc.hpp"
#include <cstddef>
#include <vector>

namespace Seldon::Agents
{

template<typename AgentT>
std::vector<AgentT> generate_from_file( const std::string & file )
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
        // TODO: check if empty or comment
        if( line.empty() )
        {
            break;
        }
        if( line[0] == '#' )
        {
            continue;
        }

        agents.push_back( AgentT() );

        // First column is the index of the agent
        auto end_of_first_column = line.find( ',', 0 );
        auto opinion_substring   = line.substr( end_of_first_column + 1, end_of_line );
        agents.back().from_string( opinion_substring );
    }

    return agents;
}

} // namespace Seldon::Agents