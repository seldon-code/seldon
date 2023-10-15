#pragma once
#include <algorithm>
#include <cstddef>
#include <queue>
#include <random>
#include <utility>
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

template<typename WeightCallbackT>
void reservoir_sampling_A_ExpJ(
    size_t k, size_t n, WeightCallbackT weight, std::vector<std::size_t> & buffer, std::mt19937 & mt )
{
    std::uniform_real_distribution<double> distribution( 0.0, 1.0 );

    std::vector<size_t> reservoir( k );
    using QueueItemT = std::pair<size_t, double>;

    auto compare = []( const QueueItemT & item1, const QueueItemT & item2 ) { return item1.second > item2.second; };
    std::priority_queue<QueueItemT, std::vector<QueueItemT>, decltype( compare )> H;

    size_t idx = 0;
    while( idx < n & H.size() < k )
    {
        double r = std::pow( distribution( mt ), 1.0 / weight( idx ) );
        H.push( { idx, r } );
        idx++;
    }

    auto X = std::log( distribution( mt ) ) / std::log( H.top().second );
    while( idx < n )
    {
        auto w = weight( idx );
        X -= w;
        if( X <= 0 )
        {
            auto t                     = std::pow( H.top().second, w );
            auto uniform_from_t_to_one = distribution( mt ) * ( 1.0 - t ) + t; // Random number in interval [t, 1.0]
            auto r                     = std::pow( uniform_from_t_to_one, 1.0 / w );
            H.pop();
            H.push( { idx, r } );
            X = std::log( distribution( mt ) ) / std::log( H.top().second );
        }
        idx++;
    }

    buffer.resize( H.size() );

    for( size_t i = 0; i < k; i++ )
    {
        buffer[i] = H.top().first;
        H.pop();
    }
}

/**
 * @brief Power law distribution for random numbers.
 * A continuous random distribution on the range [eps, infty) with equal
 * distribution
 * p(x) = (1-gamma)/(1-eps^(1-gamma)) * x^(-gamma)
 */
template<typename ScalarT = double>
class power_law_distribution
{
private:
    ScalarT eps;
    ScalarT gamma;
    std::uniform_real_distribution<ScalarT> dist
        = std::uniform_real_distribution<ScalarT>( 0.0, 1.0 ); // Uniform random variable for activities

public:
    power_law_distribution( ScalarT eps, ScalarT gamma ) : eps( eps ), gamma( gamma ) {}

    template<typename Generator>
    ScalarT operator()( Generator & gen )
    {
        return std::pow(
            ( 1.0 - std::pow( eps, ( 1.0 - gamma ) ) ) * dist( gen ) + std::pow( eps, ( 1.0 - gamma ) ),
            ( 1.0 / ( 1.0 - gamma ) ) );
    }
};

} // namespace Seldon