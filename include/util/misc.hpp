#pragma once
#include "fmt/format.h"
#include <fmt/ostream.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace Seldon
{

inline std::string get_file_contents( const std::string & filename )
{
    auto path = std::filesystem::path( filename );
    if( !std::filesystem::exists( path ) )
    {
        throw std::runtime_error( fmt::format( "Canot read from {}. File does not exist!", fmt::streamed( path ) ) );
    }

    std::ifstream in( filename, std::ios::in | std::ios::binary );
    if( in )
    {
        std::string contents;
        in.seekg( 0, std::ios::end );
        contents.resize( in.tellg() );
        in.seekg( 0, std::ios::beg );
        in.read( &contents[0], contents.size() );
        in.close();
        return ( contents );
    }
    throw( std::runtime_error( "Cannot read file." ) );
}

/*
Executes `callback` for each substring in a comma separated list.
If the input is "a_d, b, 1", it would call the callback function like 
so:
    callback(0, "a d")
    callback(1, " b")
    callback(2, " 1")
*/
template<typename CallbackT>
void parse_comma_separated_list( const std::string & str, CallbackT & callback )
{
    int idx_entry = 0;
    auto pos_cur  = -1; // Have to initialize from -1, because we start looking one past pos_cur
    while( true )
    {
        auto pos_next = str.find( ',', pos_cur + 1 );
        auto substr   = str.substr( pos_cur + 1, pos_next );
        callback( idx_entry, substr );

        pos_cur = pos_next;
        idx_entry++;
        if( pos_next == std::string::npos )
        {
            break;
        }
    }
}

} // namespace Seldon