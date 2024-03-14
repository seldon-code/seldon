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

} // namespace Seldon