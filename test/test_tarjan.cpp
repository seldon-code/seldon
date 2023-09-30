#include <catch2/catch_test_macros.hpp>

#include "connectivity.hpp"
#include <vector>

// Create the vector of vectors containing the neighbour indices
// Example from:
// https://www.baeldung.com/cs/scc-tarjans-algorithm
// Here's some ASCII art of the neighbours (made using ASCIIFlow Infinity)
//         +--------+
//     +---+        |
//   +>+ B +-+    +-v-+   +---+-->+---+
//   | +---+ |    | D +-->+ E |   | F |
//   |       |    +---+   +-+-+<--+-+-+
//   |       v              ^       ^
// +-+-+   +-+-+            |       |
// | A +<--+ C |          +-+-+   +-+-+
// +---+   +---+          | G +-->+ H |
//                        +---+   +-+-+
//                        ^       ^ |
//                        |  +----+ |
//                        |  |      v
//                        +--++   +-+-+
//                        | J +<--+ I |
//                        +---+   +---+

void create_neighbour_list( std::vector<std::vector<size_t>> & neighbour_list ) {
    
    auto buffer = std::vector<size_t>(); // contains indices
    // // --- Create the list
    buffer.resize( 1, 1 );
    neighbour_list.push_back( buffer ); // A or 0
    buffer.resize( 2 );
    buffer[0] = 2;
    buffer[1] = 3;
    neighbour_list.push_back( buffer ); // B or 1
    buffer.resize( 1 );
    buffer[0] = 0;
    neighbour_list.push_back( buffer ); // C or 2
    buffer[0] = 4;
    neighbour_list.push_back( buffer ); // D or 3
    buffer[0] = 5;
    neighbour_list.push_back( buffer ); // E or 4
    buffer[0] = 4;
    neighbour_list.push_back( buffer ); // F or 5
    // Second tree (G,H,I,J)
    buffer.resize( 2 );
    buffer[0] = 4;
    buffer[1] = 7;
    neighbour_list.push_back( buffer ); // G or 6
    buffer[0] = 5;
    buffer[1] = 8;
    neighbour_list.push_back( buffer ); // H or 7
    buffer.resize( 1 );
    buffer[0] = 9;
    neighbour_list.push_back( buffer ); // I or 8
    buffer.resize( 2 );
    buffer[0] = 6;
    buffer[1] = 7;
    neighbour_list.push_back( buffer ); // J or 9
}

TEST_CASE( "Test for Tarjan's algorithm for strongly connected networks", "[tarjan]" ) {
    
    std::vector<std::vector<size_t>> neighbour_list;
    create_neighbour_list( neighbour_list );

    auto tarjan_scc = Seldon::TarjanConnectivityAlgo( neighbour_list );

    // List of SCC
    // [[5, 4], [3], [2, 1, 0], [9, 8, 7, 6]]

    // There should be 4 strongly connected components 
    REQUIRE( tarjan_scc.scc_list.size() == 4 );
    
}