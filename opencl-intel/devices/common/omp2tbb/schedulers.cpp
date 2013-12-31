#include "stdafx.h"

#include <assert.h>

#include "libomp2tbb.h"

extern "C" int omp_get_thread_num();

template < typename T >
static void static_init( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype,
                        kmp_int32 *plastiter, T *plower, T *pupper,    T *pstride, T incr, T chunk )
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( gtid );
    kmp_int32 ltid = omp_get_thread_num();

    int num_threads = (int)entry->my_concurency;

    if ( num_threads == 1 ) {
        // If running on single thread, this is the single and the last iteration
        *plastiter = TRUE;
        return;
    }

    T iter_count;

    // Compute number of iterations
    if ( incr == 1 ) {
        iter_count = *pupper - *plower + 1;
    } else if (incr == -1) {
        iter_count = *plower - *pupper + 1;
    } else {
        if ( incr > 1 ) {
            iter_count = (*pupper - *plower) / incr + 1;
        } else {
            iter_count = (*plower - *pupper) / ( -incr ) + 1;
        }
    }

    switch ( schedtype )
    {
    case omp2tbb::kmp_sch_static: {
            T chunk = iter_count / num_threads;
            T leftover = iter_count % num_threads;
            *plower += incr * ( ltid * chunk + ( ltid < leftover ? ltid : leftover ) );
            *pupper = *plower + chunk * incr - ( ltid < leftover ? 0 : incr );
            *plastiter = ( ltid == num_threads - 1 );
        }
        break;

    case omp2tbb::kmp_sch_static_chunked: {
            if ( chunk < 1 ) {
                chunk = 1;
            }
            T range = chunk * incr;
            *pstride = range * num_threads;
            *plower = *plower + (range * ltid);
            *pupper = *plower + range - incr;
            kmp_int32 last_tid = ((iter_count - 1) / chunk) % num_threads;
            *plastiter = (ltid == last_tid);
        }
        break;
    default:
        assert(0 && "__kmpc_for_static_init: unknown scheduling type" );
        break;
    }
}

extern "C" LIBOMP2TBB_API void __kmpc_for_static_init_8( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype, kmp_int32 *plastiter,
                      kmp_int64 *plower, kmp_int64 *pupper,
                      kmp_int64 *pstride, kmp_int64 incr, kmp_int64 chunk )
{
    static_init<kmp_int64>(loc, gtid, schedtype, plastiter, plower, pupper, pstride, incr, chunk);
}
 
extern "C" LIBOMP2TBB_API void __kmpc_for_static_fini(ident_t* loc, kmp_int32 global_tid)
{
    // nothing to do here
    return;
}
