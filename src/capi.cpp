#include "seldon_capi.h"
#include <cmath>
#include <vector>

int seldon_degroot_settle( size_t n, const size_t * n_in, const size_t * neigh,
                           const double * weight, double * opinions, double tol,
                           int max_iter, int * rounds_out )
{
    if( n == 0 || n_in == nullptr || opinions == nullptr || max_iter < 1 )
    {
        return -1;
    }
    size_t edges = 0;
    for( size_t i = 0; i < n; i++ )
    {
        edges += n_in[i];
    }
    if( edges > 0 && ( neigh == nullptr || weight == nullptr ) )
    {
        return -1;
    }

    std::vector<double> nxt( n, 0.0 );
    int rounds = 0;
    for( int r = 1; r <= max_iter; r++ )
    {
        size_t off = 0;
        for( size_t i = 0; i < n; i++ )
        {
            double acc = 0.0;
            for( size_t j = 0; j < n_in[i]; j++ )
            {
                size_t k = neigh[off + j];
                if( k >= n )
                {
                    return -1;
                }
                acc += weight[off + j] * opinions[k];
            }
            nxt[i] = acc;
            off += n_in[i];
        }
        double err = 0.0;
        for( size_t i = 0; i < n; i++ )
        {
            err = std::max( err, std::abs( nxt[i] - opinions[i] ) );
            opinions[i] = nxt[i];
        }
        rounds = r;
        if( err < tol )
        {
            break;
        }
    }
    if( rounds_out != nullptr )
    {
        *rounds_out = rounds;
    }
    return 0;
}
