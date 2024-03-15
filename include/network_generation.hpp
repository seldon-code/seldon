#pragma once
#include "network.hpp"
#include <cstddef>
#include <random>
#include <util/math.hpp>
#include <util/misc.hpp>

namespace Seldon::NetworkGeneration
{
/* Constructs a new network with n_connections per agent
   If self_interaction=true, a connection of the agent with itself is included, which is *not* counted in n_connections
*/
template<typename AgentType>
Network<AgentType>
generate_n_connections( size_t n_agents, size_t n_connections, bool self_interaction, std::mt19937 & gen )
{
    using NetworkT = Network<AgentType>;
    using WeightT  = typename NetworkT::WeightT;

    std::vector<std::vector<size_t>> neighbour_list;  // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;    // List for the interaction weights of each connection
    std::uniform_real_distribution<> dis( 0.0, 1.0 ); // Values don't matter, will be normalized
    auto incoming_neighbour_buffer
        = std::vector<size_t>(); // for the j_agents indices connected to i_agent (adjacencies/neighbours)
    auto incoming_neighbour_weights = std::vector<WeightT>(); // Vector of weights of the j neighbours of i
    WeightT outgoing_norm_weight    = 0;

    // Loop through all the agents and create the neighbour_list and weight_list
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        outgoing_norm_weight = 0.0;

        incoming_neighbour_buffer.clear();
        incoming_neighbour_weights.clear();

        // Get the vector of sorted adjacencies, excluding i (include i later)
        // TODO: option for making the n_connections variable
        draw_unique_k_from_n( i_agent, n_connections, n_agents, incoming_neighbour_buffer, gen );

        incoming_neighbour_weights.resize( incoming_neighbour_buffer.size() );
        for( size_t j = 0; j < incoming_neighbour_buffer.size(); ++j )
        {
            incoming_neighbour_weights[j] = dis( gen ); // Draw the weight
            outgoing_norm_weight += incoming_neighbour_weights[j];
        }

        if( self_interaction )
        {
            // Put the self-interaction as the last entry
            auto self_interaction_weight = dis( gen );
            outgoing_norm_weight += self_interaction_weight;
            // outgoing_norm_weights += self_interaction_weight;
            incoming_neighbour_buffer.push_back( i_agent ); // Add the agent itself
            incoming_neighbour_weights.push_back( self_interaction_weight );
        }

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        for( size_t j = 0; j < incoming_neighbour_buffer.size(); ++j )
        {
            incoming_neighbour_weights[j] /= outgoing_norm_weight;
        }

        // Add the neighbour vector for i_agent to the neighbour list
        neighbour_list.push_back( incoming_neighbour_buffer );
        // Add the weight interactions for the neighbours of i_agent
        weight_list.push_back( incoming_neighbour_weights );

    } // end of loop through n_agents

    return NetworkT( std::move( neighbour_list ), std::move( weight_list ), NetworkT::EdgeDirection::Incoming );
}

template<typename AgentType>
Network<AgentType> generate_fully_connected( size_t n_agents, typename Network<AgentType>::WeightT weight = 0.0 )
{
    using NetworkT = Network<AgentType>;
    using WeightT  = typename NetworkT::WeightT;

    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
    auto incoming_neighbour_buffer
        = std::vector<size_t>( n_agents ); // for the j_agents indices connected to i_agent (adjacencies/neighbours)
    auto incoming_neighbour_weights
        = std::vector<WeightT>( n_agents, weight ); // Vector of weights of the j neighbours of i

    // Create the incoming_neighbour_buffer once. This will contain all agents, including itself
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        incoming_neighbour_buffer[i_agent] = i_agent;
    }

    // Loop through all the agents and update the neighbour_list and weight_list
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        // Add the neighbour vector for i_agent to the neighbour list
        neighbour_list.push_back( incoming_neighbour_buffer );
        // Add the weight interactions for the neighbours of i_agent
        weight_list.push_back( incoming_neighbour_weights );

    } // end of loop through n_agents

    return NetworkT( std::move( neighbour_list ), std::move( weight_list ), NetworkT::EdgeDirection::Incoming );
}

template<typename AgentType>
Network<AgentType> generate_fully_connected( size_t n_agents, std::mt19937 & gen )
{
    using NetworkT = Network<AgentType>;
    using WeightT  = typename NetworkT::WeightT;

    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
    auto incoming_neighbour_buffer
        = std::vector<size_t>( n_agents ); // for the j_agents indices connected to i_agent (adjacencies/neighbours)
    std::uniform_real_distribution<> dis( 0.0, 1.0 );                   // Values don't matter, will be normalized
    auto incoming_neighbour_weights = std::vector<WeightT>( n_agents ); // Vector of weights of the j neighbours of i
    WeightT outgoing_norm_weight    = 0;

    // Create the incoming_neighbour_buffer once. This will contain all agents, including itself
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {
        incoming_neighbour_buffer[i_agent] = i_agent;
    }

    // Loop through all the agents and create the neighbour_list and weight_list
    for( size_t i_agent = 0; i_agent < n_agents; ++i_agent )
    {

        outgoing_norm_weight = 0.0;

        // Initialize the weights
        for( size_t j = 0; j < n_agents; ++j )
        {
            incoming_neighbour_weights[j] = dis( gen ); // Draw the weight
            outgoing_norm_weight += incoming_neighbour_weights[j];
        }

        // ---------
        // Normalize the weights so that the row sums to 1
        // Might be specific to the DeGroot model?
        for( size_t j = 0; j < incoming_neighbour_buffer.size(); ++j )
        {
            incoming_neighbour_weights[j] /= outgoing_norm_weight;
        }

        // Add the neighbour vector for i_agent to the neighbour list
        neighbour_list.push_back( incoming_neighbour_buffer );
        // Add the weight interactions for the neighbours of i_agent
        weight_list.push_back( incoming_neighbour_weights );

    } // end of loop through n_agents

    return NetworkT( std::move( neighbour_list ), std::move( weight_list ), NetworkT::EdgeDirection::Incoming );
}

template<typename AgentType>
Network<AgentType> generate_from_file( const std::string & file )
{
    using NetworkT = Network<AgentType>;
    using WeightT  = typename NetworkT::WeightT;
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection

    std::string file_contents = get_file_contents( file );

    // bool finished = false;
    size_t start_of_line = 0;
    bool finished        = false;
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

        // Parse the columns
        neighbour_list.emplace_back( 0 );
        weight_list.emplace_back( 0 );

        size_t start_of_column = 0;
        bool finished_row      = false;
        size_t idx_column      = 0;
        size_t n_neighbours    = 0;
        while( !finished_row )
        {
            auto end_of_column = line.find( ',', start_of_column );

            if( end_of_column == std::string::npos )
            {
                finished_row = true;
            }

            auto column_substring = line.substr( start_of_column, end_of_column - start_of_column );
            start_of_column       = end_of_column + 1;

            // First column is idx_agent (does not get used)
            if( idx_column == 1 ) // Second column contains the number of incoming neighbours
            {
                n_neighbours = std::stoi( column_substring );
            }
            else if(
                ( idx_column >= 2 )
                && ( idx_column < 2 + n_neighbours ) ) // The next n_neighbours columsn contain the neighbour indices
            {
                const auto idx_neighbour = std::stoi( column_substring );
                neighbour_list.back().push_back( idx_neighbour );
            }
            else if( idx_column >= 2 + n_neighbours )
            { // The rest of the columns are the weights
                const auto weight = std::stod( column_substring );
                weight_list.back().push_back( weight );
            }
            idx_column++;
        }
    }

    return NetworkT( std::move( neighbour_list ), std::move( weight_list ), NetworkT::EdgeDirection::Incoming );
}
} // namespace Seldon::NetworkGeneration