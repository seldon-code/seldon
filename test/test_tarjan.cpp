#include <catch2/catch_test_macros.hpp>

#include "connectivity.hpp"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <set>
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

TEST_CASE( "Test for Tarjan's algorithm for strongly connected networks", "[tarjan]" )
{

    // clang-format off
    std::vector<std::vector<size_t>> neighbour_list = std::vector<std::vector<size_t>>{
        {1},
        {2,3},
        {0},
        {4},
        {5},
        {4},
        {4,7},
        {5,8},
        {9},
        {6,7}
    };
    // clang-format on

    auto tarjan_scc = Seldon::TarjanConnectivityAlgo( neighbour_list );

    // List of SCC
    // [[5, 4], [3], [2, 1, 0], [9, 8, 7, 6]]
    fmt::print( "SCC = {}\n", tarjan_scc.scc_list );

    std::set<std::set<size_t>> expected_scc{ { 5, 4 }, { 3 }, { 2, 1, 0 }, { 9, 8, 7, 6 } };

    for( const auto & scc : tarjan_scc.scc_list )
    {
        std::set<size_t> temp_set;
        temp_set.insert( scc.begin(), scc.end() );
        REQUIRE( expected_scc.contains( temp_set ) );
    }

    // There should be 4 strongly connected components
    REQUIRE( tarjan_scc.scc_list.size() == 4 );
}