/*
    Copyright 2005-2013 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#include "tbb/atomic.h"
#include "tbb/task.h"
#include "tbb/task_arena.h"

#include "stdio.h"

#ifndef ASSERT
    #define ASSERT __TBB_ASSERT
    #define __TBB_UNDEF_ASSERT
#endif
#include "harness_barrier.h"

namespace tbb { namespace Harness {

class TbbWorkersTrapper {
    tbb::task *my_root;
    tbb::task_group_context* my_context;
    Harness::SpinBarrier* my_barrier;

    friend class TrapperTask;

    class TrapperTask : public tbb::task {
        tbb::task*                root_task;
        Harness::SpinBarrier*     barrier;
        int my_id;

        tbb::task* execute () {
            barrier->wait(); // Wait until all workers are ready
            root_task->wait_for_all();
            barrier->signal_nowait();
            return NULL;
        }
    public:
        TrapperTask ( TbbWorkersTrapper& owner, int id ) :
          root_task(owner.my_root), barrier(owner.my_barrier), my_id(id) {}
    };

    class TrapperReleaseTask : public tbb::task {
        tbb::task*                root_task;
        Harness::SpinBarrier*     barrier;
        tbb::task_group_context*  context;
    public:
        TrapperReleaseTask( TbbWorkersTrapper& owner) :
          root_task(owner.my_root), barrier(owner.my_barrier), context(owner.my_context) {}
        tbb::task* execute () {
            barrier->wait(); // Make sure no tasks are referencing us
            tbb::task::destroy(*root_task);
            delete barrier;
            delete context;
            return NULL;
        }
    };

    int           num_threads;
    bool          is_async;
    volatile bool is_trapped;
public:
    TbbWorkersTrapper ( int _num_threads, bool _is_async )
        : my_root(NULL),
          my_context(NULL),
          num_threads(_num_threads),
          is_async(_is_async),
          is_trapped(false)
    {
        my_barrier = new Harness::SpinBarrier;
        my_barrier->initialize(num_threads + (is_async ? 1 : 0));
        my_context = new tbb::task_group_context(tbb::task_group_context::bound,
            tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait);
    }

    ~TbbWorkersTrapper () {
        if ( !is_trapped )
            return;

        my_root->decrement_ref_count();
        if ( tbb::task_arena::current_slot() > 0 )
        {
            // executing by a worker, so we must enqueue a task that will destroy the root task
            tbb::task::enqueue( *new ( tbb::task::allocate_root() ) TrapperReleaseTask(*this) );
            return;
        } else
        {
            my_barrier->wait(); // Make sure no tasks are referencing us
            tbb::task::destroy(*my_root);
            delete my_barrier;
            delete my_context;
        }
    }

    bool  IsTrapped() const { return is_trapped;}

    int   GetTrappedThreadCount() const { return num_threads; }

    void  Wait() { my_barrier->wait(); }

    void operator()(void)
    {
        assert( (task_arena::current_slot() == 0 || is_async) && "Trapper must be executed from the master slot or be async" );
        my_root = new ( tbb::task::allocate_root(*my_context) ) tbb::empty_task;
        my_root->set_ref_count(2);
        for ( int i = 1; i < num_threads; ++i )
        {
            tbb::task::spawn( *new(tbb::task::allocate_root()) TrapperTask(*this, i) );
        }
        my_barrier->wait(); // Wait until all workers are ready
        is_trapped = true;
        // For async we need to trap this task as well
        if ( is_async )
        {
            my_root->wait_for_all();
            my_barrier->wait();
        }
    }

}; // TbbWorkersTrapper

class TbbWorkersPrefetcher {
    Harness::SpinBarrier my_barrier;

    friend class PrefetcherTask;

    class PrefetcherTask : public tbb::task {
        Harness::SpinBarrier& my_owner_barrier;

        tbb::task* execute () {
            my_owner_barrier.wait();
            return NULL;
        }
    public:
        PrefetcherTask ( Harness::SpinBarrier& bar ) : my_owner_barrier(bar) {}
    };

public:
    TbbWorkersPrefetcher ( int numThreads ) {
        my_barrier.initialize(numThreads);
        tbb::task_group_context ctx(tbb::task_group_context::isolated);
        tbb::task &r = *new ( tbb::task::allocate_root(ctx) ) tbb::empty_task;
        r.set_ref_count(numThreads + 1);
        for ( int i = 0; i < numThreads; ++i )
            tbb::task::spawn( *new(r.allocate_child()) PrefetcherTask(my_barrier) );
        r.wait_for_all(); // Wait until all workers are ready
        tbb::task::destroy(r);
    }
}; // TbbWorkersPrefetcher

} // namespace Harness
} // namespace tbb

#ifdef __TBB_UNDEF_ASSERT
    #undef ASSERT
#endif

