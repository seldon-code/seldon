#pragma once
#include "fmt/core.h"
#include "util/erfinv.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Seldon
{

// Function for getting a vector of k agents (corresponding to connections)
// drawing from n agents (without duplication)
// ignore_idx ignores the index of the agent itself, since we will later add the agent itself ourselves to prevent duplication
inline void draw_unique_k_from_n(
    std::optional<size_t> ignore_idx, std::size_t k, std::size_t n, std::vector<std::size_t> & buffer,
    std::mt19937 & gen )
{
    struct SequenceGenerator
    {
        /* An iterator that generates a sequence of integers 2, 3, 4 ...*/
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = size_t;
        using pointer           = size_t *; // or also value_type*
        using reference         = size_t &;

        SequenceGenerator( const size_t i_, std::optional<size_t> ignore_idx ) : i( i_ ), ignore_idx( ignore_idx )
        {
            if( ignore_idx.has_value() && i == ignore_idx.value() )
            {
                i++;
            }
        }
        size_t i;
        std::optional<size_t> ignore_idx;

        size_t & operator*()
        {
            return i;
        };
        bool operator==( const SequenceGenerator & it1 ) const
        {
            return i == it1.i;
        };
        SequenceGenerator & operator++()
        {
            i++;
            if( ignore_idx.has_value() && i == ignore_idx )
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
    if( k == 0 )
        return;

    std::uniform_real_distribution<double> distribution( 0.0, 1.0 );

    std::vector<size_t> reservoir( k );
    using QueueItemT = std::pair<size_t, double>;

    auto compare = []( const QueueItemT & item1, const QueueItemT & item2 ) { return item1.second > item2.second; };
    std::priority_queue<QueueItemT, std::vector<QueueItemT>, decltype( compare )> H;

    size_t idx = 0;
    while( ( idx < n ) && ( H.size() < k ) )
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
 * A continuous random distribution on the range [eps, infty)
 * with p(x) ~ x^(-gamma)
 * Including normalization the PDF is
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
        return inverse_cumulative_probability( dist( gen ) );
    }

    ScalarT inverse_cumulative_probability( ScalarT x )
    {
        return std::pow(
            ( 1.0 - std::pow( eps, ( 1.0 - gamma ) ) ) * x + std::pow( eps, ( 1.0 - gamma ) ),
            ( 1.0 / ( 1.0 - gamma ) ) );
    }

    ScalarT mean()
    {
        return -( 1.0 - gamma ) / ( 2.0 - gamma ) * std::pow( eps, 2.0 - gamma )
               / ( 1.0 - std::pow( eps, 1.0 - gamma ) );
    }
};

/**
 * @brief Truncated normal distribution
 * A continuous random distribution on the range [eps, infty)
 * with p(x) ~ e^(-(x-mean)^2/(2 sigma^2))
 */
template<typename ScalarT = double>
class truncated_normal_distribution
{
private:
    ScalarT mean{};
    ScalarT sigma{};
    ScalarT eps{};
    std::normal_distribution<ScalarT> normal_dist{};
    size_t max_tries = 5000;

    ScalarT inverse_cum_gauss( ScalarT y )
    {
        return erfinv( 2.0 * y - 1 ) * std::sqrt( 2.0 ) * sigma + mean;
    }

    ScalarT cum_gauss( ScalarT x )
    {
        return 0.5 * ( 1 + std::erf( ( x - mean ) / ( sigma * std::sqrt( 2.0 ) ) ) );
    }

public:
    truncated_normal_distribution( ScalarT mean, ScalarT sigma, ScalarT eps )
            : mean( mean ), sigma( sigma ), eps( eps ), normal_dist( std::normal_distribution<ScalarT>( mean, sigma ) )
    {
    }

    template<typename Generator>
    ScalarT operator()( Generator & gen )
    {
        for( size_t i = 0; i < max_tries; i++ )
        {
            auto sample = normal_dist( gen );
            if( sample > eps )
                return sample;
        }
        return eps;
    }

    ScalarT inverse_cumulative_probability( ScalarT y )
    {
        return inverse_cum_gauss(
            y * ( 1.0 - cum_gauss( eps, sigma, mean ) ) + cum_gauss( eps, sigma, mean ), sigma, mean );
    }
};

/**
 * @brief Bivariate normal distribution
 * with mean mu = [0,0]
 * and covariance matrix Sigma = [[1, cov], [cov, 1]]
 * |cov| < 1 is required
 */
template<typename ScalarT = double>
class bivariate_normal_distribution
{
private:
    ScalarT covariance;
    std::normal_distribution<ScalarT> normal_dist{};

public:
    bivariate_normal_distribution( ScalarT covariance ) : covariance( covariance ) {}

    template<typename Generator>
    std::array<ScalarT, 2> operator()( Generator & gen )
    {
        ScalarT n1 = normal_dist( gen );
        ScalarT n2 = normal_dist( gen );

        ScalarT r1 = n1;
        ScalarT r2 = covariance * n1 + std::sqrt( 1 - covariance * covariance );

        return { r1, r2 };
    }
};

template<typename ScalarT, typename dist1T, typename dist2T>
class bivariate_gaussian_copula
{
private:
    ScalarT covariance;
    bivariate_normal_distribution<ScalarT> bivariate_normal_dist{};
    // std::normal_distribution<ScalarT> normal_dist{};

    // Cumulative probability function for gaussian with mean 0 and variance 1
    ScalarT cum_gauss( ScalarT x )
    {
        return 0.5 * ( 1 + std::erf( ( x ) / std::sqrt( 2.0 ) ) );
    }

    dist1T dist1;
    dist2T dist2;

public:
    bivariate_gaussian_copula( ScalarT covariance, dist1T dist1, dist2T dist2 )
            : covariance( covariance ),
              dist1( dist1 ),
              dist2( dist2 ),
              bivariate_normal_dist( bivariate_normal_dist( covariance ) )
    {
    }

    template<typename Generator>
    std::array<ScalarT, 2> operator()( Generator & gen )
    {
        // 1. Draw from bivariate gaussian
        auto z = bivariate_normal_dist( gen );
        // 2. Transform marginals to unit interval
        std::array<ScalarT, 2> z_unit = { cum_gauss( z[0] ), cum_gauss( z[1] ) };
        // 3. Apply inverse transform sampling
        std::array<ScalarT, 2> res
            = { dist1.inverse_cumulative_probability( z_unit[0] ), dist2.inverse_cumulative_probability( z_unit[1] ) };
        return res;
    }
};

template<typename T>
int hamming_distance( std::span<T> v1, std::span<T> v2 )
{
    if( v1.size() != v2.size() )
    {
        throw std::runtime_error( "v1 and v2 need to have the same size" );
    }

    int distance = 0;
    for( size_t i = 0; i < v2.size(); i++ )
    {
        if( v1[i] != v2[i] )
            distance++;
    }
    return distance;
}

} // namespace Seldon