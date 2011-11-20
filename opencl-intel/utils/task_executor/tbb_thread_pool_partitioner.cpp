
    //Copyright 2005-2011 Intel Corporation.  All Rights Reserved.

    //The source code contained or described herein and all documents related
    //to the source code ("Material") are owned by Intel Corporation or its
    //suppliers or licensors.  Title to the Material remains with Intel
    //Corporation or its suppliers and licensors.  The Material is protected
    //by worldwide copyright laws and treaty provisions.  No part of the
    //Material may be used, copied, reproduced, modified, published, uploaded,
    //posted, transmitted, distributed, or disclosed in any way without
    //Intel's prior express written permission.

    //No license under any patent, copyright, trade secret or other
    //intellectual property right is granted to or conferred upon you by
    //disclosure or delivery of the Materials, either expressly, by
    //implication, inducement, estoppel or otherwise.  Any license under such
    //intellectual property rights must be express and approved by Intel in
    //writing.


#include <cassert>
#include <cl_utils.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sched.h>
#endif

#include <tbb/task_scheduler_init.h>
#include <tbb/atomic.h>
#include <tbb/task.h>

#include "cl_utils.h"
#include "tbb_executor.h"

using namespace Intel::OpenCL::TaskExecutor;
namespace Intel { namespace OpenCL { namespace TaskExecutor {

class SpinBarrier
{
    unsigned numThreads;
    tbb::atomic<unsigned> numThreadsFinished; /* threads reached barrier in this epoch */
    tbb::atomic<unsigned> epoch;   /* how many times this barrier used - XXX move to a separate cache line */

    struct DummyCallback {
        void operator() () const {}
    };

    SpinBarrier( const SpinBarrier& );    // no copy ctor
    void operator=( const SpinBarrier& ); // no assignment 
public:
    SpinBarrier( unsigned nthreads = 0 ) { initialize(nthreads); };

    void initialize( unsigned nthreads ) {
        numThreads = nthreads;
        numThreadsFinished = 0;
        epoch = 0;
    };

    // onOpenBarrierCallback is called by last thread arrived on a barrier
    template<typename Callback>
    bool wait(const Callback &onOpenBarrierCallback)
    { // return true if last thread
        unsigned myEpoch = epoch;
        int threadsLeft = numThreads - numThreadsFinished.fetch_and_increment() - 1;
        assert((threadsLeft>=0) && "Broken barrier");
        if (threadsLeft > 0) {
            /* not the last threading reaching barrier, wait until epoch changes & return 0 */
            tbb::internal::spin_wait_while_eq(epoch, myEpoch);
            return false;
        }
        /* No more threads left to enter, so I'm the last one reaching this epoch;
           reset the barrier, increment epoch, and return non-zero */
        onOpenBarrierCallback();
        numThreadsFinished = 0;
        epoch = myEpoch+1; /* wakes up threads waiting to exit this epoch */
        return true;
    }
    bool wait()
    {
        return wait(DummyCallback());
    }
};

class TbbWorkersTrapper {
    tbb::task *my_root;
    tbb::task_group_context my_context;
    tbb::atomic<int> m_idIndex;
	SpinBarrier my_barrier;
	unsigned int* m_legalIDs;
	unsigned int  m_workerCount;
	IAffinityChangeObserver* m_pObserver;

    friend class TrapperTask;

    class TrapperTask : public tbb::task {
        TbbWorkersTrapper& my_owner;

        tbb::task* execute () 
		{
            my_owner.TrappedCallback();
            my_owner.my_barrier.wait();
            my_owner.my_root->wait_for_all();
            my_owner.my_barrier.wait();
            return NULL;
        }
    public:
        TrapperTask ( TbbWorkersTrapper& owner ) : my_owner(owner) {}
    };

public:
    TbbWorkersTrapper ( unsigned int numThreads, unsigned int* IDs , IAffinityChangeObserver* pObserver)
        : my_context(tbb::task_group_context::bound, 
                     tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait), m_legalIDs(IDs), m_workerCount(numThreads), m_pObserver(pObserver)
    {
    }

	void Activate()
	{
        //Starting with 1 because slot 0 of the legal IDs will be used by subdevice dispatcher thread
        m_idIndex = 1;
        my_root = new ( tbb::task::allocate_root(my_context) ) tbb::empty_task;
        my_root->set_ref_count(2);
        my_barrier.initialize(m_workerCount);
        for ( unsigned int i = 1; i < m_workerCount; ++i )
		{
            tbb::task::spawn( *new(tbb::task::allocate_root()) TrapperTask(*this) );
		}
        my_barrier.wait(); // Wait util all workers are ready
	}

	void Deactivate()
	{
        my_root->decrement_ref_count();
        my_barrier.wait(); // Make sure no tasks are referencing us
        tbb::task::destroy(*my_root);
	}

        ~TbbWorkersTrapper () {}

protected:
        void TrappedCallback()      
        {
            int myId = m_idIndex.fetch_and_increment();
	    m_pObserver->NotifyAffinity(ThreadIDAssigner::GetWorkerID(), m_legalIDs[myId]);
        }

}; // TbbWorkersTrapper

class MyObserver
{
public:
	MyObserver( int numWorkers, unsigned int* legalCoreIDs, IAffinityChangeObserver* pObserver )
		: m_numWorkers(numWorkers), m_legalCoreIDs(legalCoreIDs), m_init(tbb::task_scheduler_init::deferred), m_pObserver(pObserver)
    {

		m_trapper = new TbbWorkersTrapper((int)numWorkers, legalCoreIDs, m_pObserver);
	}

    virtual ~MyObserver() 
	{

		if (NULL != m_trapper)
		{
			delete m_trapper;
		}
	}

    bool Activate()
    {
        m_init.initialize(m_numWorkers);
#ifndef WIN32
		if (NULL == m_trapper)
		{
			return false;
		}
		m_trapper->Activate();


#endif
		return true;
    }
    void Deactivate()
    {
#ifndef WIN32
		if (NULL != m_trapper)
		{
    		m_trapper->Deactivate();
		}
#endif
        m_init.terminate();
    }

protected:
    int m_numWorkers;
	unsigned int* m_legalCoreIDs;
    tbb::task_scheduler_init m_init;
    TbbWorkersTrapper* m_trapper;
	IAffinityChangeObserver* m_pObserver;
};

TBBThreadPoolPartitioner::TBBThreadPoolPartitioner(int numThreads, unsigned int* legalCoreIDs, IAffinityChangeObserver* pObserver)
{
    m_observer = new MyObserver(numThreads, legalCoreIDs, pObserver);
    assert(NULL != m_observer);
}

TBBThreadPoolPartitioner::~TBBThreadPoolPartitioner()
{
    delete m_observer;
}

bool TBBThreadPoolPartitioner::Activate()
{
    return m_observer->Activate();
}
void TBBThreadPoolPartitioner::Deactivate()
{
    m_observer->Deactivate();
}
}}}
