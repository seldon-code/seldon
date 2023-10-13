#pragma once
#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

namespace Seldon
{

// Function for getting a vector of k agents (corresponding to connections)
// drawing from n agents (without duplication)
// ignore_idx ignores the index of the agent itself, since we will later add the agent itself ourselves to prevent duplication
inline void draw_unique_k_from_n(
    std::size_t ignore_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer, std::mt19937 & gen )
{
    struct SequenceGenerator
    {
        /* An iterator that generates a sequence of integers 2, 3, 4 ...*/
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = size_t;
        using pointer           = size_t *; // or also value_type*
        using reference         = size_t &;

        SequenceGenerator( const size_t i_, const size_t ignore_idx ) : i( i_ ), ignore_idx( ignore_idx )
        {
            if( i == ignore_idx )
            {
                i++;
            }
        }
        size_t i;
        size_t ignore_idx;

        size_t & operator*()
        {
            return i;
        };
        bool operator==( const SequenceGenerator & it1 )
        {
            return i == it1.i;
        };
        SequenceGenerator & operator++()
        {
            i++;
            if( i == ignore_idx )
                i++;
            return *this;
        }
    };

    buffer.resize( k );
    std::sample( SequenceGenerator( 0, ignore_idx ), SequenceGenerator( n, ignore_idx ), buffer.begin(), k, gen );
}

} // namespace Seldon