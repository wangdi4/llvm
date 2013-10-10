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

#ifndef ASSERT
    #define ASSERT __TBB_ASSERT
    #define __TBB_UNDEF_ASSERT
#endif
#include "harness_barrier.h"

namespace tbb { namespace Harness {

class TbbWorkersTrapper {
    tbb::task *my_root;
    tbb::task_group_context my_context;
    Harness::SpinBarrier my_barrier;

    friend class TrapperTask;

    class TrapperTask : public tbb::task {
        TbbWorkersTrapper& my_owner;

        tbb::task* execute () {
            my_owner.my_barrier.wait(); // Wait util all workers are ready
            my_owner.my_root->wait_for_all();
            my_owner.my_barrier.wait();
            return NULL;
        }
    public:
        TrapperTask ( TbbWorkersTrapper& owner ) : my_owner(owner) {}
    };

    int           num_threads;
    volatile bool is_trapped;
public:
    TbbWorkersTrapper ( int _num_threads )
        : my_root(NULL),
          my_context(tbb::task_group_context::bound,
                     tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait),
          num_threads(_num_threads),
          is_trapped(false)
    {
    }

    ~TbbWorkersTrapper () {
        if ( !is_trapped )
            return;

        my_root->decrement_ref_count();
        my_barrier.wait(); // Make sure no tasks are referencing us
        tbb::task::destroy(*my_root);
    }

    bool  IsTrapped() const { return is_trapped;}

    void operator()(void)
    {
        my_root = new ( tbb::task::allocate_root(my_context) ) tbb::empty_task;
        my_root->set_ref_count(2);
        my_barrier.initialize(num_threads);
        for ( int i = 1; i < num_threads; ++i )
        {
            tbb::task::spawn( *new(tbb::task::allocate_root()) TrapperTask(*this) );
        }
        my_barrier.wait(); // Wait until all workers are ready
        is_trapped = true;
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
        r.wait_for_all(); // Wait util all workers are ready
        tbb::task::destroy(r);
    }
}; // TbbWorkersPrefetcher

} // namespace Harness
} // namespace tbb

#ifdef __TBB_UNDEF_ASSERT
    #undef ASSERT
#endif

