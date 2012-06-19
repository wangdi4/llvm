// Copyright (c) 2006-2012 Intel Corporation
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

// means config.h
#include "tbb_executor.h"
#include "Logger.h"

#include <stdafx.h>
#include <vector>
#include <cassert>
#ifdef WIN32
#include <Windows.h>
#endif
#include <cl_sys_defines.h>
#include <cl_sys_info.h>
#include <tbb/blocked_range.h>
#include <tbb/atomic.h>
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#include <tbb/concurrent_queue.h>
#include <tbb/task.h>
#include <tbb/enumerable_thread_specific.h>

using namespace Intel::OpenCL::Utils;

//#define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

#ifdef _DEBUG
AtomicBitField DEBUG_Thread_ID_running;
#endif

namespace Intel { namespace OpenCL { namespace TaskExecutor {

void RegisterReleaseSchedulerForMasterThread();
// Logger
DECLARE_LOGGER_CLIENT;

//! global TBB task scheduler objects
unsigned int gWorker_threads = 0;
AtomicCounter	glTaskSchedCounter;

#if defined(_WIN32)
  #if defined(__WIN_XP__)
    #error "Windows XP is not supported"
  #endif
__declspec(thread) ThreadIDAssigner::thread_local_data_t t_ThreadIDInfo = {INVALID_WORKER_ID, NULL, NULL};
#ifdef _DEBUG
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;
#endif
#else
__thread ThreadIDAssigner::thread_local_data_t t_ThreadIDInfo = {INVALID_WORKER_ID, NULL, NULL};
#endif

// -----------------------------------------------------------
ThreadIDAllocator::ThreadIDAllocator(unsigned int uiNumThreads) : m_uiNumThreads(uiNumThreads)
{
	m_aThreadAvailabilityMask.init(uiNumThreads, 1);
	m_aRefCount = 1;
}

unsigned int ThreadIDAllocator::GetNextAvailbleId()
{
	unsigned int uiWorkerId = 0;
	long canExit = false;
	while (uiWorkerId < m_uiNumThreads)
	{
		canExit = m_aThreadAvailabilityMask.bitTestAndReset(uiWorkerId);
		if (canExit) break;
		++uiWorkerId;
	}

	if ( uiWorkerId >= m_uiNumThreads )
	{
		assert(0 && "We always should be able to allocate ID for worker thread");
		return -1;
	}

	m_aRefCount++;
	return ++uiWorkerId;
}

void ThreadIDAllocator::ReleaseId(unsigned int uiId)
{
	assert((m_aRefCount>0) && "ReferenceCount > 0 is expected");
	if ( INVALID_WORKER_ID != uiId )
	{
		m_aThreadAvailabilityMask.bitTestAndSet(uiId-1);
	}
	long prev = m_aRefCount--;
	if ( 1 == prev )
	{
		delete this;
	}
}


// Initialize static value
Intel::OpenCL::Utils::AtomicPointer<ThreadIDAllocator> ThreadIDAssigner::m_spIdAllocator;
ThreadIDAssigner* ThreadIDAssigner::_this = NULL;
Intel::OpenCL::Utils::OclReaderWriterLock ThreadIDAssigner::m_IDAllocatorLock;

//Implementation of the interface to be notified on thread addition/removal from the working thread pool
ThreadIDAssigner::ThreadIDAssigner() : tbb::task_scheduler_observer()
{
	m_spIdAllocator = NULL;
	m_bUseTaskalyzer = false;
	_this = this;
}

ThreadIDAssigner::~ThreadIDAssigner()
{
}

unsigned int ThreadIDAssigner::GetWorkerID()
{
	return t_ThreadIDInfo.uiWorkerId;
}

void ThreadIDAssigner::SetWorkerID(unsigned int id)
{
	t_ThreadIDInfo.uiWorkerId = id;
}

void ThreadIDAssigner::ReleaseWorkerID()
{
	unsigned int uiWorkerId = t_ThreadIDInfo.uiWorkerId;
	assert((t_ThreadIDInfo.pIDAllocator!=NULL) && "IDAllocator must be set by this time");
	t_ThreadIDInfo.pIDAllocator->ReleaseId(uiWorkerId-1);
	t_ThreadIDInfo.pIDAllocator = NULL;
	t_ThreadIDInfo.uiWorkerId = INVALID_WORKER_ID;
}

tbb::task_scheduler_init* ThreadIDAssigner::GetScheduler()
{
	return t_ThreadIDInfo.pScheduler;
}

void ThreadIDAssigner::SetScheduler(tbb::task_scheduler_init* pSched)
{						
	t_ThreadIDInfo.pScheduler = pSched;
}

bool ThreadIDAssigner::IsWorkerScheduler()
{
	return (WORKER_SCHEDULER_ID == GetScheduler());
}

void ThreadIDAssigner::StopObservation()
{
	_this->observe(false);
}

void ThreadIDAssigner::StartObservation()
{
	_this->observe();
}

void ThreadIDAssigner::on_scheduler_entry( bool is_worker )
{
	unsigned int uiWorkerId = 0;

	//assert((NULL==t_ThreadIDInfo.pIDAllocator) && "IDAllocator expected to be NULL");

	if ( is_worker && (INVALID_WORKER_ID == GetWorkerID()))
	{
		m_IDAllocatorLock.EnterRead();
		ThreadIDAllocator* pAllocator = m_spIdAllocator;
		if (NULL == pAllocator)
		{
			m_IDAllocatorLock.LeaveRead();
			return;
		}
		t_ThreadIDInfo.pIDAllocator = pAllocator;
		uiWorkerId = pAllocator->GetNextAvailbleId();
		m_IDAllocatorLock.LeaveRead();
		assert((uiWorkerId>0) && "Worker ID shall be > 0");
		SetScheduler(WORKER_SCHEDULER_ID);
	}
	SetWorkerID(uiWorkerId);

#ifdef _EXTENDED_LOG
	LOG_INFO("------->%s %d was joined as %d\n", is_worker ? "worker" : "master",
		GetThreadId(GetCurrentThread()), uiWorkerId);
#endif
	
#if defined(WIN32) && defined (_DEBUG)
	if ( !is_worker )
	{
		return;
	}
	char szThreadName[100];
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = -1;
	info.dwFlags = 0;
	sprintf_s(szThreadName, 100, "Worker id=%d", uiWorkerId);
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
#endif
}

void ThreadIDAssigner::on_scheduler_exit( bool is_worker )
{
#ifdef _EXTENDED_LOG
	LOG_INFO("------->%s %d was left as %d\n", is_worker ? "worker" : "master",
		GetThreadId(GetCurrentThread()), GetWorkerID());
#endif
	if ( (is_worker)  && (INVALID_WORKER_ID != GetWorkerID()))
	{
		ReleaseWorkerID();
		SetScheduler(WORKER_SCHEDULER_ID);
	}

#if defined(WIN32) && defined (_DEBUG)
	if ( !is_worker )
	{
		return;
	}
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "TBB-worker";
	info.dwThreadID = -1;
	info.dwFlags = 0;
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
#endif

}

ThreadIDAllocator* ThreadIDAssigner::SetThreadIdAllocator(ThreadIDAllocator* pNewIdAllocator)
{
	ThreadIDAllocator* old = m_spIdAllocator.exchange(pNewIdAllocator);
	return old;
}

void InitSchedulerForMasterThread(int iNumeOfThreads = -1, bool bTBBExplicitScheduler = false)
{
	tbb::task_scheduler_init* pScheduler = ThreadIDAssigner::GetScheduler();
	if ( (NULL == pScheduler) && !((IMPLICIT_SCHEDULER_ID == pScheduler) || (ThreadIDAssigner::IsWorkerScheduler())) )
	{
		long prevVal = glTaskSchedCounter++;
		pScheduler = IMPLICIT_SCHEDULER_ID;
		if ( (0==prevVal)  || bTBBExplicitScheduler )
		{
			if ( -1 == iNumeOfThreads)
			{
				iNumeOfThreads = gWorker_threads;
			}
			pScheduler = new tbb::task_scheduler_init(iNumeOfThreads);
			assert(NULL!=pScheduler && "Failed to create TBB explicit scheduler");
			if (NULL == pScheduler )
			{
				glTaskSchedCounter--;
				return;
			}
			if ( 0==prevVal )
			{
				bool rc = static_cast<TBBTaskExecutor*>(GetTaskExecutor())->AllocateResources();
				if ( !rc )
				{
					delete pScheduler;
					glTaskSchedCounter--;
					return;
				}
				ThreadIDAssigner::StartObservation();
			}
		}

		ThreadIDAssigner::SetScheduler(pScheduler);
		
		RegisterReleaseSchedulerForMasterThread();
	}
}

void ReleaseSchedulerForMasterThread()
{
	tbb::task_scheduler_init* pScheduler = ThreadIDAssigner::GetScheduler();
	if ( (WORKER_SCHEDULER_ID == pScheduler) || (NULL == pScheduler) )
	{
		return;
	}

	glTaskSchedCounter--;
	ThreadIDAssigner::SetScheduler(NULL);

	if ( IMPLICIT_SCHEDULER_ID == pScheduler )
	{
		return;
	}

	if ( 0 != glTaskSchedCounter )
	{
		// Need to delete explicit scheduler and exit
		delete pScheduler;
		return;
	}

	static_cast<TBBTaskExecutor*>(GetTaskExecutor())->ReleaseResources();
    ThreadIDAllocator* prev;
    {
		OclAutoWriter CS(&ThreadIDAssigner::m_IDAllocatorLock);
	    prev = ThreadIDAssigner::SetThreadIdAllocator(NULL);
	}
	if (NULL == prev)
	{
	    assert(0 && "The old ThreadID allocator is NULL");
		return;
	}

	unsigned int uiTimeOutCount = 0;
	long refCount = prev->GetRefCount();

	delete pScheduler;

	// Wait for 100ms for workes shutdown
	while ( 1 < refCount && (uiTimeOutCount < 100))
	{
		clSleep(1);
		refCount = prev->GetRefCount();
		uiTimeOutCount++;
	}
  	//ThreadIDAssigner::SetThreadIdAllocator(prev);

	//assert((1==refCount) && "RefCount of 1 is expected");
	if ( 1 == refCount )
	{
		// We release resources only when all worker threads were shut down,
		// Otherwise resource leaks and memory corruption inside TBB
		ThreadIDAssigner::StopObservation();
	}
	if ( NULL != prev )
	{
		prev->ReleaseId(INVALID_WORKER_ID);
	}
}

#ifdef __LOCAL_RANGES__
	class Range1D {
	public:
		//! Type of a value
		/** Called a const_iterator for sake of algorithms that need to treat a blocked_range
		as an STL container. */
		typedef size_t const_iterator;

		//! Type for size of a range
		typedef std::size_t size_type;

		//! Construct range with default-constructed values for begin and end.
		/** Requires that Value have a default constructor. */
		Range1D() : my_begin(0), my_end(0) {}

		//! Construct range over half-open interval [begin,end), with the given grainsize.
		Range1D( size_t begin_, size_t end_) : 
		my_end(end_), my_begin(begin_)
		{
			my_grain_size = (unsigned int)((size()+(gWorker_threads-1)*16-1)/((gWorker_threads-1)*16));
		}

		//! Beginning of range.
		const_iterator begin() const {return my_begin;}

		//! One past last value in range.
		const_iterator end() const {return my_end;}

		//! Size of the range
		/** Unspecified if end()<begin(). */
		size_type size() const {
			__TBB_ASSERT( !(end()<begin()), "size() unspecified if end()<begin()" );
			return size_type(my_end-my_begin);
		}

		//------------------------------------------------------------------------
		// Methods that implement Range concept
		//------------------------------------------------------------------------

		//! True if range is empty.
		bool empty() const {return !(my_begin<my_end);}

		//! True if range is divisible.
		/** Unspecified if end()<begin(). */
		bool is_divisible() const
			{ return  my_grain_size < size();}

		//! Split range.  
		/** The new Range *this has the second half, the old range r has the first half. 
		Unspecified if end()<begin() or !is_divisible(). */
		Range1D( Range1D& r, tbb::split ) : 
		my_end(r.my_end),
			my_begin(do_split(r))
		{
			my_grain_size = (unsigned int)((size()+(gWorker_threads-1)*16-1)/((gWorker_threads-1)*16));
		}

	private:
		/** NOTE: my_end MUST be declared before my_begin, otherwise the forking constructor will break. */
		size_t my_end;
		size_t my_begin;
		size_type	my_grain_size;

		//! Auxiliary function used by forking constructor.
		/** Using this function lets us not require that Value support assignment or default construction. */
		static unsigned int do_split( Range1D& r ) {
			__TBB_ASSERT( r.is_divisible(), "cannot split blocked_range that is not divisible" );
			unsigned int middle = r.my_begin + (r.my_end-r.my_begin)/2u;
			r.my_end = middle;
			return middle;
		}

		friend class Range3D;
		friend class Range2D;
	};

	// A 3-dimensional range that models the Range concept.
	class Range2D {
	public:
	private:
		Range1D				my_rows;
		Range1D				my_cols;
		Range1D::size_type	my_grain_size;
	public:

		Range2D( unsigned int range[2]) : 
			  my_rows(0,range[1]),
			  my_cols(0,range[0])
		  {
			  __int64 n = 1;
			  for(int i =0; i<2; ++i)
				  n *= range[i];
			  my_grain_size = (unsigned int)((n+(gWorker_threads-1)*16-1)/((gWorker_threads-1)*16));
		  }

		  //! True if range is empty
		  bool empty() const {
			  // Yes, it is a logical OR here, not AND.
			  return my_rows.empty() || my_cols.empty();
		  }

		  //! True if range is divisible into two pieces.
		  bool is_divisible() const {
			  return  my_grain_size < (my_rows.size()*my_cols.size());
		  }

		  Range2D( Range2D& r, tbb::split ) : 
			  my_rows(r.my_rows),
			  my_cols(r.my_cols)
		  {
			  if ( my_rows.size() < my_cols.size() ) {
				  my_cols.my_begin = Range1D::do_split(r.my_cols);
			  } else {
				  my_rows.my_begin = Range1D::do_split(r.my_rows);
			  }

			  // Recalculate grain size
			  __int64 n = my_rows.size()*my_cols.size();
			  my_grain_size = (unsigned int)((n+(gWorker_threads-1)*16-1)/((gWorker_threads-1)*16));
		  }

		  //! The rows of the iteration space 
		  const Range1D& rows() const {return my_rows;}

		  //! The columns of the iteration space 
		  const Range1D& cols() const {return my_cols;}

	};

	// A 3-dimensional range that models the Range concept.
	class Range3D {
	public:
	private:
		Range1D				my_pages;
		Range1D				my_rows;
		Range1D				my_cols;
		Range1D::size_type	my_grain_size;
	public:

		Range3D( unsigned int range[3]) : 
				my_pages(0,range[2]),
				my_rows(0,range[1]),
				my_cols(0,range[0])
		{
			__int64 n = 1;
			for(int i =0; i<3; ++i)
				n *= range[i];
			my_grain_size = (unsigned int)((n+(gWorker_threads-1)*16-1)/((gWorker_threads-1)*16));
		}

		//! True if range is empty
		bool empty() const {
			// Yes, it is a logical OR here, not AND.
			return my_pages.empty() || my_rows.empty() || my_cols.empty();
		}

		//! True if range is divisible into two pieces.
		bool is_divisible() const {
			return  my_grain_size < (my_pages.size()*my_rows.size()*my_cols.size());
		}

		Range3D( Range3D& r, tbb::split ) : 
		my_pages(r.my_pages),
			my_rows(r.my_rows),
			my_cols(r.my_cols)
		{
			if( my_pages.size() < my_rows.size() ) {
				if ( my_rows.size() < my_cols.size() ) {
					my_cols.my_begin = Range1D::do_split(r.my_cols);
				} else {
					my_rows.my_begin = Range1D::do_split(r.my_rows);
				}
			} else {
				if ( my_pages.size() < my_cols.size() ) {
					my_cols.my_begin = Range1D::do_split(r.my_cols);
				} else {
					my_pages.my_begin = Range1D::do_split(r.my_pages);
				}
			}

			// Recalculate grain size
			__int64 n = my_pages.size()*my_rows.size()*my_cols.size();
			my_grain_size = (unsigned int)((n+(gWorker_threads-1)*16-1)/((gWorker_threads-1)*16));
		}

		//! The pages of the iteration space 
		const Range1D& pages() const {return my_pages;}

		//! The rows of the iteration space 
		const Range1D& rows() const {return my_rows;}

		//! The columns of the iteration space 
		const Range1D& cols() const {return my_cols;}

	};
#endif
	struct TaskLoopBody3D {
		ITaskSet &task;
		TaskLoopBody3D(ITaskSet &t) : task(t) {}
		void operator()(const tbb::blocked_range3d<int>& r) const {
			unsigned int uiWorkerId;
			size_t uiNumberOfWorkGroups;
			uiWorkerId = ThreadIDAssigner::GetWorkerID();

            size_t firstWGID[3] = {r.pages().begin(), r.rows().begin(),r.cols().begin()}; 
			size_t lastWGID[3] = {r.pages().end(),r.rows().end(),r.cols().end()}; 

			uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

#ifdef _DEBUG
			if ( 0 != uiWorkerId )
			{
				long prev = DEBUG_Thread_ID_running.bitTestAndReset(uiWorkerId-1);
				assert( prev == 1 && "Worker with same ID is already running");
			}
#endif

			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
            for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
				for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
					for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
						task.ExecuteIteration(k, j, i, uiWorkerId);
			task.DetachFromThread(uiWorkerId);

#ifdef _DEBUG
			DEBUG_Thread_ID_running.bitTestAndSet(uiWorkerId-1);
#endif
		}
	};

	struct TaskLoopBody2D {
		ITaskSet &task;
		TaskLoopBody2D(ITaskSet &t) : task(t) {}
		void operator()(const tbb::blocked_range2d<int>& r) const {
			unsigned int uiWorkerId;
			size_t uiNumberOfWorkGroups;
			uiWorkerId = ThreadIDAssigner::GetWorkerID();

            size_t firstWGID[2] = {r.rows().begin(),r.cols().begin()}; 
			size_t lastWGID[2] = {r.rows().end(),r.cols().end()}; 

			uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

#ifdef _DEBUG
			if ( 0 != uiWorkerId )
			{
				long prev = DEBUG_Thread_ID_running.bitTestAndReset(uiWorkerId-1);
				assert( prev == 1 && "Worker with same ID is already running");
			}
#endif

			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
			for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
				for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
					task.ExecuteIteration(k, j, 0, uiWorkerId);
			task.DetachFromThread(uiWorkerId);
#ifdef _DEBUG
			DEBUG_Thread_ID_running.bitTestAndSet(uiWorkerId-1);
#endif
		}
	};

	struct TaskLoopBody1D {
		ITaskSet &task;
		TaskLoopBody1D(ITaskSet &t) : task(t) {}
		void operator()(const tbb::blocked_range<int>& r) const {
			unsigned int uiWorkerId;
			size_t uiNumberOfWorkGroups;
			uiWorkerId = ThreadIDAssigner::GetWorkerID();

			size_t firstWGID[1] = {r.begin()}; 
			size_t lastWGID[1] = {r.end()}; 

			uiNumberOfWorkGroups = r.size();
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);
#ifdef _DEBUG
			if ( 0 != uiWorkerId )
			{
				long prev = DEBUG_Thread_ID_running.bitTestAndReset(uiWorkerId-1);
				assert( prev == 1 && "Worker with same ID is already running");
			}
#endif
			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;

			for(size_t k = r.begin(), f = r.end(); k < f; k++ )
					task.ExecuteIteration(k, 0, 0, uiWorkerId);
			task.DetachFromThread(uiWorkerId);

#ifdef _DEBUG
			DEBUG_Thread_ID_running.bitTestAndSet(uiWorkerId-1);
#endif

		}
	};

	// TODO: replace by lambda when available
	struct TaskSetBody {
		ITaskSet &task;
		TaskSetBody(ITaskSet &t) : task(t) {}
		void operator()() { 
			// TODO: How iterations in task sets can differ in workload? Define gs appropriately.
			size_t dim[3];
			unsigned int dimCount;
			int res = task.Init(dim, dimCount);
			__TBB_ASSERT(res==0, "Init failed");
			if (res != 0)
			{
				task.Finish(FINISH_INIT_FAILED);
				return;
			}
					if (1 == dimCount)
					{
						assert(dim[0] <= CL_MAX_INT32);
						tbb::parallel_for(tbb::blocked_range<int>(0, (int)dim[0]), TaskLoopBody1D(task), tbb::auto_partitioner());
					} else if (2 == dimCount)
					{
						assert(dim[0] <= CL_MAX_INT32);
						assert(dim[1] <= CL_MAX_INT32);
						tbb::parallel_for(tbb::blocked_range2d<int>(0, (int)dim[1],
																	0, (int)dim[0]),
																	TaskLoopBody2D(task), tbb::auto_partitioner());
					} else
					{
						assert(dim[0] <= CL_MAX_INT32);
						assert(dim[1] <= CL_MAX_INT32);
						assert(dim[2] <= CL_MAX_INT32);
						tbb::parallel_for(tbb::blocked_range3d<int>(0, (int)dim[2],
																	0, (int)dim[1],
																	0, (int)dim[0]),
																	TaskLoopBody3D(task), tbb::auto_partitioner());
					}
			task.Finish(FINISH_COMPLETED);
			task.Release();
		}
	};

	// TODO: replace by lambda when available
	struct TaskBody {
		ITask &task;
		TaskBody(ITask &t) : task(t)
		{
			LOG_DEBUG("Task Created %0X", &t);
		}
		void operator()() { 
			LOG_DEBUG("Task Executed %0X", &task);
			task.Execute();
			task.Release();
		}
	};

// Master thread syncronization task
// This task used to mark when any master thread requested synchronization point
// Internal variable is set when queue reached the sync. point
// Execute return false, due to execution should be interrupted
class SyncTask : public ITask
{
public:
	SyncTask() {m_bFired = false;}
	void	Reset() { m_bFired = false;}

	// ITask interface
	bool	SetAsSyncPoint() {return false;}
	bool	CompleteAndCheckSyncPoint() { return m_bFired;}
	bool	IsCompleted() const {return m_bFired;}
	bool	Execute() { m_bFired = true; return true;}
	long	Release() { return 0; }	//Persistent member, don't release

protected:
	volatile bool	m_bFired;
};

typedef OclNaiveConcurrentQueue<ITaskBase*> ConcurrentTaskQueue;
typedef std::vector<ITaskBase*>           TaskVector;

template<class cExecTaskClass> class base_command_list : public ITaskList
{
public:
	base_command_list(bool subdevice, TBBTaskExecutor* pTBBExec) :
		m_subdevice(subdevice), m_pTBBExecutor(pTBBExec)
	{
		m_pListRootTask = new (tbb::task::allocate_root(*m_pTBBExecutor->GetTBBGroupContext())) base_list_root_task();
		m_pListRootTask->set_ref_count(1);

		m_execTaskRequests = 0;
		m_bMasterRunning = false;
		m_refCount = 1;
	}

	virtual ~base_command_list()
	{
	}

	unsigned int Enqueue(ITaskBase* pTask)
	{
        if (!m_subdevice)
        {
            InitSchedulerForMasterThread();
        }
		m_quIncomingWork.PushBack(pTask);
		return 0;
	}

	te_wait_result WaitForCompletion(ITaskBase* pTaskToWait)
	{
		// Request processing task to stop
		if ( NULL != pTaskToWait )
		{
			bool completed = pTaskToWait->SetAsSyncPoint();
			// If already completed no need to wait
			if ( completed )
			{
				return TE_WAIT_COMPLETED;
			}
		}

		InitSchedulerForMasterThread();

		if ( ThreadIDAssigner::GetScheduler() == WORKER_SCHEDULER_ID )
		{
			return TE_WAIT_NOT_SUPPORTED;
		}

		bool bVal = m_bMasterRunning.compare_and_swap(true, false);
		if (bVal)
		{
			// When another master is running we can't block this thread
			return TE_WAIT_MASTER_THREAD_BLOCKING;
		}

		++m_refCount;

		if ( NULL == pTaskToWait )
		{
			m_MasterSync.Reset();
			Enqueue(&m_MasterSync);
		}

		do
		{
			unsigned int ret = InternalFlush(true);
			if ( ret > 0)
			{
				// Someone else started the task, need to wait
				m_pListRootTask->wait_for_all();
			}
		} while ( !(m_MasterSync.IsCompleted() || ((NULL != pTaskToWait) && (pTaskToWait->IsCompleted()))) );
		
		// Current master is not in charge for the work
		m_bMasterRunning = false;

		// If there's some incoming work during operation, try to flush it
		if ( HaveIncomingWork() )
		{
			Flush();
		}

		Release();

		return TE_WAIT_COMPLETED;
	}

	bool Flush()
	{
		InternalFlush(false);
		return true;
	}

    void Retain() 
    {
        ++m_refCount;
    }

	void Release() 
	{
		long newRef = --m_refCount;
		if ( 0 != newRef )
		{
			return;
		}

		assert(!m_bMasterRunning && "Master not supposed to run");

		if ( !ThreadIDAssigner::IsWorkerScheduler() )
		{
			m_pListRootTask->wait_for_all();
			tbb::task::destroy(*m_pListRootTask);
		}
		else
		{
			tbb::task::enqueue(*m_pListRootTask);
		}

		m_pListRootTask = NULL;
		delete this;
	}

	ConcurrentTaskQueue* GetExecutingContainer()
	{
		return &m_quIncomingWork;
	}

protected:
	friend class in_order_executor_task;
	friend class unorder_executor_task;

	// Lets us know whether there's (still) incoming work
	bool HaveIncomingWork()
	{
		return !m_quIncomingWork.IsEmpty();
	}

	unsigned int LaunchExecutorTask(bool blocking)
	{
		m_refCount++;
		if (!m_subdevice)
		{
			InitSchedulerForMasterThread();

			m_pListRootTask->increment_ref_count();
			tbb::task* executor = new (m_pListRootTask->allocate_child()) cExecTaskClass(this);
			assert(executor && "Can't create executor task");
			if ( NULL == executor )
			{
				m_pListRootTask->decrement_ref_count();
				m_execTaskRequests = 0;
				return 0;
			}
			
			if (!blocking)
			{
				tbb::task::enqueue(*executor);
				return 1;
			}

			m_pListRootTask->spawn_and_wait_for_all(*executor);
		}
		else
		{
			cExecTaskClass	queueTask(this);
			queueTask.execute();
		}

		return 0;
	}

	inline unsigned int InternalFlush(bool blocking)
	{
		unsigned int runningTaskRequests = m_execTaskRequests++;
		if ( 0 == runningTaskRequests )
		{
			//We need to launch the task to handle the existing input
			return LaunchExecutorTask(blocking);
		}
		//Otherwise, the task is already running and will see our input in the next epoch
		return runningTaskRequests;
	}

    bool                    m_subdevice;
	TBBTaskExecutor*		m_pTBBExecutor;
    tbb::task*              m_pListRootTask;

	tbb::atomic<unsigned int>	m_refCount;			// reference counter on command list

	ConcurrentTaskQueue		m_quIncomingWork;

	tbb::atomic<unsigned int>	m_execTaskRequests;
	SyncTask					m_MasterSync;

	// Only single muster thread can join the execution on specific queue
	// this mutex will block others. The atomic prevents wait on
	// the master if another master is running
	tbb::atomic<bool>		m_bMasterRunning;

private:
	//Disallow copy constructor
	base_command_list(const base_command_list& l) : ITaskList() {}

	class base_list_root_task  : public tbb::task
	{
		tbb::task* execute()
		{
			wait_for_all();
			decrement_ref_count();
			return NULL;
		}
	};
};

static bool execute_command(ITaskBase* cmd)
{
	bool runNextCommand = true;

	if ( cmd->IsTaskSet() )
	{
		ITaskSet* pTask = static_cast<ITaskSet*>(cmd);
		size_t dim[MAX_WORK_DIM];
		unsigned int dimCount;
		int res = pTask->Init(dim, dimCount);
		__TBB_ASSERT(res==0, "Init Failed");
		if (res != 0)
		{
			pTask->Finish(FINISH_INIT_FAILED);
			return false;
		}
		if (1 == dimCount)
		{
			assert(dim[0] <= CL_MAX_INT32);
			tbb::parallel_for(tbb::blocked_range<int>(0, (int)dim[0]), TaskLoopBody1D(*pTask), tbb::auto_partitioner());
		} else if (2 == dimCount)
		{
			assert(dim[0] <= CL_MAX_INT32);
			assert(dim[1] <= CL_MAX_INT32);
			tbb::parallel_for(tbb::blocked_range2d<int>(0, (int)dim[1],
														0, (int)dim[0]),
				TaskLoopBody2D(*pTask), tbb::auto_partitioner());
		} else
		{
			assert(dim[0] <= CL_MAX_INT32);
			assert(dim[1] <= CL_MAX_INT32);
			assert(dim[2] <= CL_MAX_INT32);
			tbb::parallel_for(tbb::blocked_range3d<int>(0, (int)dim[2],
														0, (int)dim[1],
														0, (int)dim[0]),
				TaskLoopBody3D(*pTask), tbb::auto_partitioner());
		}
		runNextCommand = pTask->Finish(FINISH_COMPLETED);
	}
	else
	{
        ITask* pCmd = static_cast<ITask*>(cmd);
		runNextCommand = pCmd->Execute();
	}

	runNextCommand &= !cmd->CompleteAndCheckSyncPoint();
	return runNextCommand;
}

class in_order_executor_task : public tbb::task
{
public:
	in_order_executor_task(base_command_list<in_order_executor_task>* list) : m_list(list){}
	
	tbb::task* execute()
	{
		assert(m_list);
		ConcurrentTaskQueue* work = m_list->GetExecutingContainer();
		assert(work);
		TaskVector currentCommandBatch;
		ITaskBase* currentTask;
		bool mustExit = false;

		assert(m_list->m_refCount>1);

		while(true)
		{
			//First check if we need to stop interating, next get next available record
			while( !(mustExit && m_list->m_bMasterRunning) && work->TryPop(currentTask))
			{
				mustExit = !execute_command(currentTask); //stop requested

				currentCommandBatch.push_back(currentTask);

				if ( currentCommandBatch.size() >= MAX_BATCH_SIZE )
				{
					// When we excide maximum allowed command batch
					// We need to release, prevent memory overflow
					FreeCommandBatch(&currentCommandBatch);
				}
			}

			FreeCommandBatch(&currentCommandBatch);

			if ( mustExit )
			{
				m_list->m_execTaskRequests.fetch_and_store(0);
				break;
			}
			if ( 1 == m_list->m_execTaskRequests-- )
			{
				break;
			}
		}

		m_list->Release();

		return NULL;
	}

protected:
	base_command_list<in_order_executor_task>* m_list;

	void FreeCommandBatch(TaskVector* pCmdBatch)
	{
		//release all executed tasks
		for (TaskVector::const_iterator it = pCmdBatch->begin(); it != pCmdBatch->end(); ++it)
		{
			(*it)->Release();
		}
		pCmdBatch->clear();
	}
};

struct ExecuteContainerBody
{
	TaskVector*		m_work;
	volatile bool*	m_bMasterSync;

	ExecuteContainerBody(TaskVector* work, volatile bool* masterSync) :
			m_work(work), m_bMasterSync(masterSync) {}

	void operator()(const tbb::blocked_range<size_t>& range) const 
	{
		bool masterSync = false; // Monitor if we should stop master

		for (size_t it = range.begin(); it != range.end(); ++it)
		{
			ITaskBase* cmd = m_work->at(it);
			masterSync |= !execute_command(cmd);

			cmd->Release();
		}

		*m_bMasterSync = masterSync;
	}
};

class unorder_executor_task : public tbb::task
{
public:
	unorder_executor_task(base_command_list<unorder_executor_task>* list) : m_list(list) {}

	tbb::task* execute()
	{
		assert(m_list);
		TaskVector		work;
		volatile bool	mustExit = false;

		assert(m_list->m_refCount>1);
		while (true)
		{
			size_t size = FillWork(work);
			while ( size > 0 )
			{

				//Todo: handle small sizes without parallel for
				tbb::blocked_range<size_t> r(0, size);
				//for out of order, do parallel for
				parallel_for(r, ExecuteContainerBody(&work, &mustExit));

				work.clear();

				size = FillWork(work);
			}

			if ( mustExit )
			{
				m_list->m_execTaskRequests.fetch_and_store(0);
				break;
			}

			if ( 1 == m_list->m_execTaskRequests-- )
			{
				break;
			}
		}

		m_list->Release();

		return NULL;
	}
protected:
	// Move work from incoming queue to work queue
	// Todo: currently this is serial, see if we can parallelize it somehow
	size_t FillWork(TaskVector& workList)
	{
		unsigned int uiBatchSize = 0;
		ITaskBase* current;
		while ( (uiBatchSize<MAX_BATCH_SIZE) && m_list->GetExecutingContainer()->TryPop(current) )
		{
			workList.push_back(current);
			++uiBatchSize;
		}
		return workList.size();
	}

	base_command_list<unorder_executor_task>* m_list;
};

/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() :
	m_pGrpContext(NULL), m_lActivateCount(0), m_pExecutorList(NULL)
{
	INIT_LOGGER_CLIENT("TBBTaskExecutor", LL_DEBUG);

	m_threadPoolChangeObserver = new ThreadIDAssigner;
	if ( NULL == m_threadPoolChangeObserver )
	{
		LOG_ERROR(TEXT("%s"), "Failed to allocate ThreadIDAssigner");
	}
}

TBBTaskExecutor::~TBBTaskExecutor()
{
	if ( NULL != m_threadPoolChangeObserver)
	{
		delete m_threadPoolChangeObserver;
		m_threadPoolChangeObserver = NULL;
	}

	LOG_INFO(TEXT("%s"),"TBBTaskExecutor Destroyed");
	RELEASE_LOGGER_CLIENT;
}

int	TBBTaskExecutor::Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData)
{
	if ( 0 != gWorker_threads )
	{
		assert(0 && "TBBExecutor already initialized");
		return gWorker_threads;
	}

	if ( NULL == m_threadPoolChangeObserver )
	{
		// Observer must exits
		LOG_ERROR(TEXT("%s"), "Can't init TBBTaskExecutor, m_threadPoolChangeObserver == NULL"); 
		return 0;
	}

	m_pGPAData = pGPAData;
	
	// Explicitly load TBB library
	if ( !LoadTBBLibrary() )
	{
		LOG_ERROR(TEXT("%s"), "Failed to load TBB library");
		return 0;
	}

	LOG_INFO(TEXT("TBBTaskExecutor constructed to %d threads"), gWorker_threads);
	LOG_INFO(TEXT("TBBTaskExecutor initialized to %u threads"), uiNumThreads);
	if (uiNumThreads > 0)
	{
		gWorker_threads = uiNumThreads;
	}
	else
	{
		gWorker_threads = Intel::OpenCL::Utils::GetNumberOfProcessors();
	}

#ifdef _DEBUG
	DEBUG_Thread_ID_running.init(gWorker_threads, 1);
#endif

	LOG_INFO(TEXT("%s"),"Done");	
	return gWorker_threads;
}

bool TBBTaskExecutor::Activate()
{
	assert(gWorker_threads && "TBB executor should be initialized first");
	OclAutoMutex mu(&m_muActivate);

	if ( 0 != m_lActivateCount )
	{
		// We are already acticated
		++m_lActivateCount;
		return true;
	}

	ThreadIDAssigner::SetScheduler(NULL);

    InitSchedulerForMasterThread(-1, true);
	tbb::task_scheduler_init* pScheduler = ThreadIDAssigner::GetScheduler();
	if ( NULL == pScheduler )
	{
		LOG_ERROR(TEXT("%s"), "Failed to initilize task scheduler");
		return false;
	}

	++m_lActivateCount;
	return true;
}

bool TBBTaskExecutor::AllocateResources()
{
	m_pGrpContext = new tbb::task_group_context(tbb::task_group_context::bound, tbb::task_group_context::concurrent_wait);
	if ( NULL == m_pGrpContext )
	{
		LOG_ERROR(TEXT("%s"), "Failed to allocate m_pGrpContext");
		return false;
	}

	m_pExecutorList = new base_command_list<in_order_executor_task>(false, this); // TODO: Why not unordered
	if ( NULL == m_pExecutorList )
	{
		delete m_pGrpContext;
		m_pGrpContext = NULL;
		LOG_ERROR(TEXT("%s"), "Failed to allocate m_pExecutorList");
		return false;
	}

	ThreadIDAllocator* pNewAllocator = new ThreadIDAllocator(gWorker_threads);
	if ( NULL == pNewAllocator )
	{
		m_pExecutorList->Release();
		m_pExecutorList = NULL;
		delete m_pGrpContext;
		m_pGrpContext = NULL;
		return false;
	}
	m_threadPoolChangeObserver->SetThreadIdAllocator(pNewAllocator);

	return true;
}

void TBBTaskExecutor::ReleaseResources()
{
	assert(NULL!=m_pExecutorList);
	if ( NULL != m_pExecutorList )
	{
		m_pExecutorList->Release();
		m_pExecutorList = NULL;
	}

	assert(NULL!=m_pGrpContext);
	if ( NULL != m_pGrpContext )
	{
		// Commenting out the delete as a work-around
		// Currently deleting the context while worker threads are executing causes crashes
		//m_pGrpContext->cancel_group_execution();
		//delete m_pGrpContext;
		m_pGrpContext = NULL;
	}
}

void TBBTaskExecutor::Deactivate()
{
	OclAutoMutex mu(&m_muActivate);

	if ( 0 != --m_lActivateCount )
	{
		// Need to keep active instance
		return;
	}

	assert(NULL != m_threadPoolChangeObserver);

	LOG_INFO(TEXT("%s"),"Shutting down...");

	if ( !ThreadIDAssigner::IsWorkerScheduler() )
	{
		ReleaseSchedulerForMasterThread();
	}
	else
	{
		//gThreadAvailabilityMask
		assert(0 && "Don't know what to do when release from worker thread");
	}
}

unsigned int TBBTaskExecutor::GetNumWorkingThreads() const
{
	return gWorker_threads;
}

ITaskList* TBBTaskExecutor::CreateTaskList(CommandListCreationParam* param)
{
    bool       subdevice = param->isSubdevice;
    bool       OOO       = param->isOOO;

    if (!subdevice)
    {
	    InitSchedulerForMasterThread();
    }

	ITaskList *pList = NULL;
	if (OOO) 
	{
		pList = new base_command_list<unorder_executor_task>(subdevice, this);
	}
	else
	{
		pList = new base_command_list<in_order_executor_task>(subdevice, this);
	}

	return pList;
}

unsigned int TBBTaskExecutor::Execute(ITaskBase * pTask)
{
	m_pExecutorList->Enqueue(pTask);
	m_pExecutorList->Flush();		
	return 0;
}

te_wait_result TBBTaskExecutor::WaitForCompletion(ITaskBase * pTask)
{
	return m_pExecutorList->WaitForCompletion(pTask);
}

void TBBTaskExecutor::ReleasePerThreadData()
{
	// TBB was not initialized within calling thread or its TBB worker thread.
	if ( ThreadIDAssigner::IsWorkerScheduler())
	{
		return;
	}

	ReleaseSchedulerForMasterThread();
}

ocl_gpa_data* TBBTaskExecutor::GetGPAData() const
{
	return m_pGPAData;
}

bool TBBTaskExecutor::LoadTBBLibrary()
{
	bool bLoadRes = true;

#ifdef WIN32
	// The loading on tbb.dll was delayed,
	// Need to load manually before defualt dll is loaded
	char tbbPath[MAX_PATH];

	Intel::OpenCL::Utils::GetModuleDirectory(tbbPath, MAX_PATH);

#ifdef _DEBUG
		STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb_debug.dll");
#else
		STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb.dll");
#endif

	bLoadRes = m_dllTBBLib.Load(tbbPath);
	if ( !bLoadRes )
	{
		LOG_ERROR(TEXT("Failed to load TBB from %s"), tbbPath);
	}
#endif

	return bLoadRes;
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
