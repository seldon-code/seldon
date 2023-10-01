#pragma once
#include "model_base.hpp"
#include "util/misc.hpp"
#include <optional>

namespace Seldon
{

/* Model<T> is a base class from which the acutal models would derive. They have efficient access to a vector of AgentT,
 * without any pointer indirections */
template<typename AgentT_>
class Model : public ModelBase
{
public:
    using AgentT = AgentT_;
    std::vector<AgentT> agents;

    std::optional<int> max_iterations = std::nullopt;
    Model( size_t n_agents ) : agents( std::vector<AgentT>( int( n_agents ), AgentT() ) ) {}
    Model( std::vector<AgentT> && agents ) : agents( agents ) {}

    void Agents_from_File( const std::string & file )
    {
        agents.clear();

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
            if( line.size() == 0 )
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
    }

    void iteration() override = 0;

    bool finished() override
    {
        if( max_iterations.has_value() )
        {
            return max_iterations.value() <= n_iterations;
        }
        else
        {
            return false;
        }
    };

    AgentBase * get_agent( int idx ) override // For this to work AgentT needs to be a subclass of AgentBase
    {
        return &agents[idx];
    }
};

template<typename AgentT_>
void Seldon::Model<AgentT_>::iteration()
{
    n_iterations++;
};

} // namespace Seldon