#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* One DeGroot sweep of x <- W x until ||dx||_inf < tol or max_iter.
 * For each agent i, n_in[i] incoming edges start at off = sum_{k<i} n_in[k].
 * neigh[off+j] and weight[off+j] are the neighbour and the incoming weight.
 * opinions is length n, overwritten in place.
 * Returns 0 on success, -1 on a bad argument.
 * This is the same iteration as Seldon::DeGrootModel::iteration. */
int seldon_degroot_settle( size_t n, const size_t * n_in, const size_t * neigh,
                           const double * weight, double * opinions, double tol,
                           int max_iter, int * rounds_out );

#ifdef __cplusplus
}
#endif
