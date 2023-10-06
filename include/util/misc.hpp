#pragma once
#include <fstream>
#include <string>

namespace Seldon
{

inline std::string get_file_contents( const std::string & filename )
{
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
    throw( std::runtime_error( "Cannot read in file." ) );
}

} // namespace Seldon