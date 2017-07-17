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

#include "stdafx.h"

#include <tbb/tbb_thread.h>

#include "libomp2tbb.h"

extern "C" int omp_get_thread_num();

extern "C" LIBOMP2TBB_API void __kmpc_critical (ident_t* loc, kmp_int32 global_tid, kmp_critical_name* crit)
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );

    entry->my_critical->lock();
}

extern "C" LIBOMP2TBB_API void __kmpc_end_critical (ident_t* loc, kmp_int32 global_tid, kmp_critical_name* crit)
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );

    entry->my_critical->unlock();
}

extern "C" LIBOMP2TBB_API kmp_int32 __kmpc_single (ident_t* loc, kmp_int32 global_tid)
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
    ident_t* old_loc = nullptr;
    long prev = -1;

    // We assume implicit barrier after "omp single". This means that all threads will go through this point before reaching the barrier
    old_loc = entry->my_last_single_location.fetch_and_store(loc);
    prev = entry->my_single_count.fetch_and_increment();

    // When all threads reached this region we can reset appearance
    if ( prev == entry->my_concurrency ) {
        entry->my_last_single_location = nullptr;
    }

    // Return true only if previous location is difference then current
    return old_loc != loc;
}

extern "C" LIBOMP2TBB_API void __kmpc_end_single (ident_t* loc, kmp_int32 global_tid)
{
    // nothing to do here
    return;
}

extern "C" LIBOMP2TBB_API void __kmpc_barrier (ident_t* loc, kmp_int32 global_tid)
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );

    // Add efficient barrier implementation, currently we just do pooling

    const long epoch = entry->my_barrier_epoch;

    // First thread that reaches this point should set the value
    entry->my_barrier_count.compare_and_swap(entry->my_concurrency, 0);
    long prev = entry->my_barrier_count.fetch_and_decrement();
    if ( prev > 1 ) {
        tbb::internal::spin_wait_while_eq(entry->my_barrier_epoch, epoch);
        return;
    }
    // last worker reach arena, need to change epoch
    // Single thread does this operation, no need in atomic
    ++(entry->my_barrier_epoch);
}


extern "C" LIBOMP2TBB_API kmp_int32 __kmpc_master (ident_t* loc, kmp_int32 global_tid)
{
    return 0 == omp_get_thread_num();
}

extern "C" LIBOMP2TBB_API void __kmpc_end_master (ident_t* loc, kmp_int32 global_tid)
{
    // nothing to do here
    return;
}
