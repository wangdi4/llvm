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

#include <assert.h>

#include <alloca.h>
#include <stdarg.h>
#include <tbb/task_scheduler_init.h>

#include <uneven/parallel_for.h>
#include <uneven/blocked_range.h>
#include <opencl_partitioner.h>

#include <vector>
#include "libomp2tbb.h"
#include "threading.h"

extern "C" void __kmpc_barrier (ident_t* loc, kmp_int32 global_tid);
// GlobalVariable

// Thread variables
        THREAD_LOCAL kmp_int32              t_tgid              = -1;        // No global index
        THREAD_LOCAL kmp_int32              t_thread_id         = -1;
extern  THREAD_LOCAL int                    t_max_concurency;

unsigned int omp2tbb::get_current_team_concurency()
{
    kmp_int32 global_tid = t_tgid;
    if ( -1 == global_tid )
        return 1;

    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
    return (0 != entry->my_concurrency) ? entry->my_concurrency : 1;
}

extern "C"  LIBOMP2TBB_API void __kmpc_push_num_threads (ident_t* loc, kmp_int32 global_tid, kmp_int32 num_threads )
{
    assert ( global_tid == t_tgid && "This function MUST be called in a context of current master thread" );
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
    entry->my_concurrency = num_threads;
}

extern "C" LIBOMP2TBB_API void __kmpc_pop_num_threads(ident_t *loc, kmp_int32 global_tid )
{
    assert ( global_tid == t_tgid && "This function MUST be called in a context of current master thread" );
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
    entry->my_concurrency = 0;
}

extern "C" int __kmp_invoke_microtask( kmpc_micro pkfn, int gtid, int npr, int argc, void *argv[] );

class fork_runner
{
    bool            enable_zero_slot;
    unsigned int    n_thr;
    kmpc_micro*     pkfn;
    int             gtid;
    int             argc;
    void*           *argv;
    mutable int     master_slot_id;

public:
    fork_runner(bool zero_slot, unsigned int _n_thr, kmpc_micro _pkfn, int _gtid, int _argc, void* _argv[]) :
      enable_zero_slot(zero_slot), n_thr(_n_thr), pkfn(_pkfn), gtid(_gtid), argc(_argc), argv(_argv), master_slot_id(-1) {}

    void operator()(const tbb::uneven::blocked_range<unsigned int>& r) const
    {
        __TBB_ASSERT(r.size() == 1, "Expecting to execute range of 1");
#if 0
        int local_slot_id = tbb::task_arena::current_slot();

        int local_id = local_slot_id - master_slot_id;
        if ( local_id < 0 ) {
            local_id += n_thr;
        }
#else
        int local_id = r.begin();
#endif
        // Need to set global(master) thread_id for each worker
        // required by omp_xxx runtime functions
        if ( local_id > 0 ){
          t_tgid = gtid;
        }
        t_thread_id = local_id;
        __kmp_invoke_microtask(pkfn, gtid, local_id, argc, argv);
        // Currently we request all workers join execution. In the future we can relax this requirement by counting of executed threads
        // Some kind of predicted partitioner
        // TODO: check if required?
        __kmpc_barrier((ident_t *)(-1), gtid);
        t_thread_id = -1;
        if ( local_id > 0 ){
            t_tgid = -1;
        }
    }

    void operator()(void) const
    {
        master_slot_id = tbb::task_arena::current_slot();
#ifdef __MIC__
        int cores = (tbb::internal::get_initial_auto_partitioner_divisor() / 4) / 4; // Assume 4 threads / core
#endif
        tbb::uneven::parallel_for(tbb::uneven::blocked_range<unsigned int>(0,n_thr,1),
            *this,
#ifndef __MIC__
            tbb::opencl_partitioner(master_slot_id, n_thr, enable_zero_slot)
#else
            tbb::opencl_partitioner(master_slot_id, n_thr, enable_zero_slot, cores, 4)
#endif
        );
    }
};

extern "C" LIBOMP2TBB_API void __kmpc_fork_call (ident_t* loc, kmp_int32 argc, kmpc_micro microtask, ...)
{
    kmp_int32 gtid = t_tgid;
    __TBB_ASSERT( -1 != gtid, "Global Thread ID should be allocated for this master first");

    void** argv = (void**)alloca(sizeof(void*)*argc);
    va_list task_args;

    va_start(task_args, microtask);
    for(int i=0;i<argc;++i)
    {
        argv[i] = va_arg(task_args, void*);
    }
    va_end(task_args);

    // Reset MT execution control flags
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( gtid );
    entry->my_last_single_location = nullptr;
    entry->my_single_count = 0;
    entry->my_barrier_count = 0;
    entry->my_barrier_epoch = 0;

    int current_arena_concurency = -1;
    bool isExplicitArena = false;
    bool isMaster = false;
    const int  current_slot = tbb::task_arena::current_slot();
    // Is running within explicit arena
    if ( -1 != current_slot )
    {   
        isExplicitArena = true;
        isMaster = (current_slot == 0);
#ifdef __USE_WORKER_COUNTER__
        omp2tbb::arena_worker_counter currentArenaCount;
        current_arena_concurency = currentArenaCount.get_active_worker_count(); 
#else
        current_arena_concurency = t_max_concurency;
#endif
    }
    if ( current_arena_concurency <= 0 ) {
        // Effectivly get number of slots in arena
        current_arena_concurency = (int)( tbb::internal::get_initial_auto_partitioner_divisor() / 4 );
        //current_arena_concurency = tbb::task_scheduler_init::default_num_threads();
    }

    // Create parallel region
    if ( !isExplicitArena && (entry->my_concurrency > 0) && (entry->my_concurrency != current_arena_concurency) ) {
        assert(0);
        // Need to create local arena
        omp2tbb::active_arena local_arena(entry->my_concurrency);
        // If explicit arena was create use its concurrency for fork

        fork_runner runner(true, entry->my_concurrency, microtask, t_tgid, argc, argv);

        local_arena.execute(runner);
    }
    else {
        bool enable_zero_slot = isMaster || (current_arena_concurency == entry->my_concurrency);
        if ( 0 == entry->my_concurrency ) {
          entry->my_concurrency = current_arena_concurency;
        }

        fork_runner runner(enable_zero_slot, entry->my_concurrency, microtask, t_tgid, argc, argv);
        runner();
    }
}

extern "C" LIBOMP2TBB_API kmp_int32 omp_get_thread_num()
{
    kmp_int32 tbb_slot_number = t_thread_id;
    if ( -1 == tbb_slot_number ) {
        // Not inside an arena, is serialized execution ?
        omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( t_tgid );
        if ( 1 == entry->my_concurrency ) {
            tbb_slot_number = 0;
        }
    }
    assert( tbb_slot_number >= 0 && "Invalid slot number");
    return tbb_slot_number;
}

extern "C" LIBOMP2TBB_API kmp_int32 __kmpc_global_thread_num (ident_t* loc)
{
    if ( -1 != t_tgid )
    {
        // Thread id is already allocated
        assert ( (t_tgid < omp2tbb::g_thread_info->get_master_count()) &&  "Allocate thread index expected to be in range of THREAD info");
        return t_tgid;
    }
    
    // New thread is used, need allocate an and data structure
    kmp_int32 new_id = omp2tbb::g_thread_info->allocate_master_thread();
    t_tgid = new_id;

    return new_id;
}

extern "C" LIBOMP2TBB_API void __kmpc_serialized_parallel(ident_t* loc, kmp_int32 global_tid)
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
    entry->my_last_single_location = nullptr;
    entry->my_single_count = 0;
    entry->my_barrier_count = 0;
    entry->my_barrier_epoch = 0;
    entry->my_concurrency =1 ;
}

extern "C" LIBOMP2TBB_API void __kmpc_end_serialized_parallel (ident_t* loc, kmp_int32 global_tid)
{
    omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
    entry->my_concurrency = 0;
}
