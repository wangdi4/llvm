// Copyright (c) 2006-2014 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

// Please look kmp_sched.cpp for reference
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

    int num_threads = (int)entry->my_concurrency;

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
