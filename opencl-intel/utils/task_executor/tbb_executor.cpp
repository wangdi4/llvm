// Copyright (c) 2006-2009 Intel Corporation
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
#include <stdafx.h>
#include <vector>
#include <cassert>
#ifdef WIN32
#include <Windows.h>
#endif
#include <cl_sys_defines.h>
#include <tbb/blocked_range.h>
#include <tbb/atomic.h>
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#include <tbb/concurrent_queue.h>
#include <tbb/task.h>
#include <tbb/enumerable_thread_specific.h>
#include "tbb_executor.h"
#include "Logger.h"

#if defined(USE_GPA)   
// This code was removed for the initial porting of TAL
// to GPA 4.0 and might be used in later stages
//#include "tal\tal.h"
#endif

#define INVALID_WORKER_ID 0xFFFFFFFF
#define INVALID_SCHEDULER_ID 0xFFFFFFFF

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")

#if defined(USE_GPA)
// This code was removed for the initial porting of TAL
// to GPA 4.0 and might be used in later stages
//#ifdef _DEBUG
//#pragma comment(lib, "gpasdk_dd_2008.lib")
//#else
//#pragma comment(lib, "gpasdk_dr_2008.lib")
//#endif
#endif
using namespace Intel::OpenCL::Utils;

//#define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

//The variables below are used to ensure working threads have unique IDs in the range [0, numThreads - 1]
//The idea is that as threads enter the pool they get a unique identifier (NextAvailableThreadId) and use the thread ID from gAvailableThreadIds in that index
//When a thread leaves the pool, it decrements NextAvailableThreadId and writes his id to gAvailableThreadIds in the previous value
static AtomicBitField gThreadAvailabilityMask;

//! global TBB task scheduler objects
unsigned int gWorker_threads = 0;

// Currently only single muster thread can join the execution
// this mutex will block others. The atomic prevents wait on
// the master if another master is running
tbb::atomic<long>		g_alMasterRunning;

#ifdef _DEBUG
// Required for testing only single Master is using worker ID=0,
// As only single master should occupy ID=0, others will fail on assert 
tbb::atomic<long>		g_alMasterIdCheck;
#endif
// Logger
DECLARE_LOGGER_CLIENT;

//Implementation of the interface to be notified on thread addition/removal from the working thread pool
class ThreadIDAssigner : public tbb::task_scheduler_observer
{
private:
	static tbb::enumerable_thread_specific<unsigned int>              *t_uiWorkerId;
	static tbb::enumerable_thread_specific<tbb::task_scheduler_init*> *t_pScheduler;

public:
	bool m_bUseTaskalyzer;

	ThreadIDAssigner() : tbb::task_scheduler_observer()
	{
		assert(NULL == t_uiWorkerId);
		assert(NULL == t_pScheduler);

		if (NULL == t_uiWorkerId)
		{
			t_uiWorkerId = new tbb::enumerable_thread_specific<unsigned int>;
		}

		if (NULL == t_pScheduler)
		{
			t_pScheduler = new tbb::enumerable_thread_specific<tbb::task_scheduler_init*>;
		}
	}

	~ThreadIDAssigner()
	{
		if (NULL != t_uiWorkerId)
		{
			delete t_uiWorkerId;
			t_uiWorkerId = NULL;
		}

		if (NULL != t_pScheduler)
		{
			delete t_pScheduler;
			t_pScheduler = NULL;
		}
	}

	static unsigned int GetWorkerID()
	{
		bool alreadyHad = false;
		unsigned int ret = t_uiWorkerId->local(alreadyHad);
		return alreadyHad ? ret : INVALID_WORKER_ID;
	}
	static void SetWorkerID(unsigned int id)
	{
		t_uiWorkerId->local() = id;
	}

	static tbb::task_scheduler_init* GetScheduler()
	{
		bool alreadyHad = false;
		tbb::task_scheduler_init* ret = t_pScheduler->local(alreadyHad);
		return alreadyHad ? ret : NULL;
	}
	static void SetScheduler(tbb::task_scheduler_init* init)
	{						
		t_pScheduler->local() = init;		
	}
	static bool IsWorkerScheduler()
	{
		return (INVALID_SCHEDULER_ID == (cl_ulong)GetScheduler());
	}

	void SetUseTaskalyzer(bool bUseTaskalyzer)
	{
		m_bUseTaskalyzer = bUseTaskalyzer;
	}

	virtual void on_scheduler_entry( bool is_worker )
	{
		unsigned int uiWorkerId = 0;
		if ( is_worker && (INVALID_WORKER_ID == GetWorkerID()))
		{
			long canExit = false;
			while (uiWorkerId < gWorker_threads)
			{
				// TODO: use 64 bit version when compiled under x64
				canExit = gThreadAvailabilityMask.bitTestAndReset(uiWorkerId);
				if (canExit) break;
				++uiWorkerId;
			}
			assert(uiWorkerId < gWorker_threads);
			++uiWorkerId;
			SetScheduler((tbb::task_scheduler_init*)INVALID_SCHEDULER_ID);
		}
#ifdef _EXTENDED_LOG
		LOG_INFO("------->%s %d was joined as %d\n", is_worker ? "worker" : "master",
			GetThreadId(GetCurrentThread()), uiWorkerId);
#endif
		SetWorkerID(uiWorkerId);

	}
	virtual void on_scheduler_exit( bool is_worker )
	{
#ifdef _EXTENDED_LOG
		LOG_INFO("------->%s %d was left as %d\n", is_worker ? "worker" : "master",
			GetThreadId(GetCurrentThread()), GetWorkerID());
#endif

		if ( (is_worker)  && (INVALID_WORKER_ID != GetWorkerID()))
		{
			long prevVal;

			prevVal = gThreadAvailabilityMask.bitTestAndSet(GetWorkerID() - 1);
			//Just for extra safety, make sure we're not relinquishing an ID somebody already relinquished
			assert(prevVal==0);
			SetScheduler(NULL);
			SetWorkerID(INVALID_WORKER_ID);
			
#if defined(USE_GPA)
			// This code was removed for the initial porting of TAL
			// to GPA 4.0 and might be used in later stages

			// Before the thread is closed, we need to flush the
			// trace data into file in order not to lose trace data
			//TAL_TRACE* trace;
			//if(m_bUseTaskalyzer)
			//{
			//	trace = TAL_GetThreadTrace();
			//	assert(NULL != trace);
			//
			//	TAL_Flush(trace);
			//}
#endif
		}
	}
};

//A singleton copy of the observer class
// The implementation base on the fact that 'gThreadPoolChangeObserver' is static and single.
tbb::enumerable_thread_specific<unsigned int>              *ThreadIDAssigner::t_uiWorkerId = NULL;
tbb::enumerable_thread_specific<tbb::task_scheduler_init*> *ThreadIDAssigner::t_pScheduler = NULL;
//static ThreadIDAssigner* gThreadPoolChangeObserver = NULL;

static void InitSchedulerForMasterThread()
{
	tbb::task_scheduler_init* pScheduler = ThreadIDAssigner::GetScheduler();
	if ( (NULL == pScheduler) && (!ThreadIDAssigner::IsWorkerScheduler()) )
	{
		//t_pScheduler = new tbb::task_scheduler_init(gWorker_threads + 1);
		ThreadIDAssigner::SetScheduler(new tbb::task_scheduler_init(gWorker_threads + 1));
	}
}


namespace Intel { namespace OpenCL { namespace TaskExecutor {

	// Make TBB init class and WG executor thread local
	ITaskList* TBBTaskExecutor::sTBB_executor = NULL;

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
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				long lMasterCheck = g_alMasterIdCheck.compare_and_swap(true, false);
				assert(lMasterCheck == false);
			}
#endif

			size_t firstWGID[3] = {r.pages().begin(), r.rows().begin(),r.cols().begin()}; 
			size_t lastWGID[3] = {r.pages().end(),r.rows().end(),r.cols().end()}; 

			uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
            for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
				for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
					for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
						task.ExecuteIteration(k, j, i, uiWorkerId);
			task.DetachFromThread(uiWorkerId);
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				g_alMasterIdCheck.fetch_and_store(false);
			}
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
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				long lMasterCheck = g_alMasterIdCheck.compare_and_swap(true, false);
				assert(lMasterCheck == false);
			}
#endif

			size_t firstWGID[2] = {r.rows().begin(),r.cols().begin()}; 
			size_t lastWGID[2] = {r.rows().end(),r.cols().end()}; 

			uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);
			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
			for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
				for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
					task.ExecuteIteration(k, j, 0, uiWorkerId);
			task.DetachFromThread(uiWorkerId);
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				g_alMasterIdCheck.fetch_and_store(false);
			}
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
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				long lMasterCheck = g_alMasterIdCheck.compare_and_swap(true, false);
				assert(lMasterCheck == false);
			}
#endif

			size_t firstWGID[1] = {r.begin()}; 
			size_t lastWGID[1] = {r.end()}; 

			uiNumberOfWorkGroups = r.size();
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);
			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
			for(size_t k = r.begin(), f = r.end(); k < f; k++ )
					task.ExecuteIteration(k, 0, 0, uiWorkerId);
			task.DetachFromThread(uiWorkerId);
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				g_alMasterIdCheck.fetch_and_store(false);
			}
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

class base_command_list : public ITaskList
{
public:
	base_command_list();

	virtual ~base_command_list()
	{
	}

	virtual unsigned int Enqueue(ITaskBase* pTask) = 0;

	virtual bool WaitForCompletion()
	{
		InitSchedulerForMasterThread();

		long lVal = g_alMasterRunning.compare_and_swap(true, false);
		if (lVal)
		{
			// When another master is running we can't block this thread
			return false;
		}
		else
		{
			// First of all, prevent task exiting due to 0 ref-count
			int prev = m_taskExecuteRequests.fetch_and_increment();
			//Corner case: if prev is 0 then task isn't currently running
			if (prev > 0)
			{
				// Request task abort
				RequestStopProcessing();
				m_rootTask->wait_for_all();
			}
			// Allow others to flush if they need to
			m_taskExecuteRequests.fetch_and_store(0);
			g_alMasterRunning = false;

			// If there's some incoming work, try to flush it
			if (HaveIncomingWork())
			{
				Flush();
			}
			return true;
		}
	}

	void Flush()
	{
		int taskExecuteRequest = m_taskExecuteRequests.fetch_and_increment();
		if (0 == taskExecuteRequest)
		{
			//We need to launch the task to handle the existing input
			LaunchExecutorTask();
		}
		//Otherwise, the task is already running and will see our input in the next epoch
	}

	void Release() 
	{ 
		m_rootTask->decrement_ref_count();
		//tbb::task::enqueue(*m_rootTask);
	}

	tbb::atomic<int> m_taskExecuteRequests;

protected:
	// Helper functions to allow application thread participation in job execution
	// Aborts the processing task after the current batch
	virtual void RequestStopProcessing() = 0;
	// Lets us know whether there's (still) incoming work
	virtual bool HaveIncomingWork()      = 0;          

	// Ordered and OOO command lists have different task objects operating on them
	virtual void LaunchExecutorTask() = 0;

	tbb::task*              m_rootTask;
	tbb::task_group_context m_context;

private:
	//Disallow copy constructor
	base_command_list(const base_command_list& l) : ITaskList() {}
};

class deleting_task : public tbb::task
{
public:
	deleting_task(base_command_list* list) : m_list(list) {}
	tbb::task* execute()
	{
		assert(m_list);
		delete m_list;
		return NULL;
	}
protected:
	base_command_list* m_list;
};

base_command_list::base_command_list() : m_context(tbb::task_group_context::bound, tbb::task_group_context::concurrent_wait)
{
	m_rootTask = new (tbb::task::allocate_root(m_context)) deleting_task(this);
	m_rootTask->set_ref_count(1);
	m_taskExecuteRequests     = 0;
}



static void execute_command(ITaskBase* cmd)
{
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
			return;
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
		pTask->Finish(FINISH_COMPLETED);
	}
	else
	{
		(static_cast<ITask*>(cmd))->Execute(); // execute one by one
	}
}

typedef tbb::concurrent_queue<ITaskBase*> ConcurrentTaskQueue;
typedef std::vector<ITaskBase*>           TaskVector;


class ordered_command_list : public base_command_list
{
public:
	virtual unsigned int Enqueue(ITaskBase* pTask)
	{
		InitSchedulerForMasterThread();
		m_work.push(pTask);
		return 0;
	}

	virtual void LaunchExecutorTask();
	virtual void RequestStopProcessing()
	{
		Enqueue(NULL);
		// Must flush here or risk deadlock - the enqueued NULL can result in an early exit at the expense of another flush
		// That will be lost as a result of storing 0 to taskExecuteRequests
		Flush();
	}

	virtual ConcurrentTaskQueue* GetExecutingContainer()
	{
		return &m_work;
	}

	virtual bool HaveIncomingWork()
	{
		return !m_work.empty();
	}

protected:
	ConcurrentTaskQueue m_work;

};


class unordered_command_list : public base_command_list
{
public:
	unordered_command_list() : m_stopRequested(false) {}

	virtual unsigned int Enqueue(ITaskBase* pTask)
	{
		InitSchedulerForMasterThread();
		m_incoming.push(pTask);
		return 0;
	}
	virtual void LaunchExecutorTask();

	virtual void RequestStopProcessing()
	{
		m_stopRequested = true;
	}

	virtual TaskVector* GetExecutingContainer()
	{
		return &m_work;
	}

	virtual bool HaveIncomingWork()
	{
		return !m_incoming.empty();
	}
	bool NeedStop()
	{
		return m_stopRequested;
	}

	void ResetStop()
	{
		m_stopRequested = false;
	}

	// Move work from incoming queue to work queue
	// Todo: currently this is serial, see if we can parallelize it somehow
	void FillWork()
	{
		ITaskBase* current;
		while (m_incoming.try_pop(current))
		{
			m_work.push_back(current);
		}
	}

protected:
	volatile bool       m_stopRequested;
	ConcurrentTaskQueue m_incoming;
	TaskVector          m_work;
};

struct ExecuteContainerBody
{
	TaskVector* m_work;

	ExecuteContainerBody(TaskVector* work) : m_work(work) {}

	void operator()(const tbb::blocked_range<size_t>& range) const 
	{
		for (size_t it = range.begin(); it != range.end(); ++it)
		{
			execute_command((*m_work)[it]);
		}
	}
};

class in_order_executor_task : public tbb::task
{
public:
	in_order_executor_task(ordered_command_list* list) : m_list(list) {}
	tbb::task* execute()
	{
		ConcurrentTaskQueue* work = m_list->GetExecutingContainer();
		assert(m_list);
		assert(work);
		TaskVector forDeletion;
		ITaskBase* currentTask;
		bool mustExit = false;
		do 
		{
			//iterate one by one
			while(work->try_pop(currentTask))
			{
				if (NULL == currentTask) //stop requested
				{
					mustExit = true;
					break;
				}
				execute_command(currentTask);
				forDeletion.push_back(currentTask);
			}

			//release all executed tasks
			for (TaskVector::const_iterator it = forDeletion.begin(); it != forDeletion.end(); ++it)
			{
				(*it)->Release();
			}
			forDeletion.clear();
			//stop iterating if it was requested or if no new work entered the queue
			if (mustExit)
			{
				return NULL;
			}
			if (1 == m_list->m_taskExecuteRequests.fetch_and_decrement())
			{
				return NULL;
			}
		} while(true);
	}
protected:

	ordered_command_list* m_list;
};



class out_of_order_executor_task : public tbb::task
{
public:
	out_of_order_executor_task(unordered_command_list* list) : m_list(list) {}

	tbb::task* execute()
	{
		TaskVector* work = m_list->GetExecutingContainer();
		assert(m_list);
		assert(work);
		do 
		{
			//sanity - OOO task shouldn't leave unfinished work
			assert(work->empty());

			m_list->FillWork();

			//process the work vector in parallel
			process_container(work);

			work->clear();

			//stop iterating if it was requested 
			if (m_list->NeedStop())
			{
				m_list->ResetStop();
				return NULL;
			}

			//stop iterating if no new work entered the queue
			if (1 == m_list->m_taskExecuteRequests.fetch_and_decrement())
			{
				return NULL;
			}
		} while(true);
	}
protected:
	void process_container(TaskVector* work)
	{
		size_t numWork = work->size();
		if (0 == numWork)
		{
			return;
		}
		//Todo: handle small sizes without parallel for
		tbb::blocked_range<size_t> r(0, numWork);
		//for out of order, do parallel for
		parallel_for(r, ExecuteContainerBody(work));
		//release serially
		for(TaskVector::iterator cmd = work->begin(); cmd != work->end(); ++cmd)
		{
			(*cmd)->Release();
		}
	}

	unordered_command_list* m_list;
};

void ordered_command_list::LaunchExecutorTask()
{
	InitSchedulerForMasterThread();
	tbb::task* executor = new (m_rootTask->allocate_child()) in_order_executor_task(this);
	assert(executor);
	m_rootTask->increment_ref_count();
	tbb::task::enqueue(*executor);
}

void unordered_command_list::LaunchExecutorTask()
{
	InitSchedulerForMasterThread();
	tbb::task* executor = new (m_rootTask->allocate_child()) out_of_order_executor_task(this);
	assert(executor);
	m_rootTask->increment_ref_count();
	tbb::task::enqueue(*executor);
}



/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() : m_lRefCount(0), m_scheduler(NULL)
{
	m_threadPoolChangeObserver = new ThreadIDAssigner;

	g_alMasterRunning = false;
#ifdef _DEBUG
	g_alMasterIdCheck = false;
#endif
	gWorker_threads = tbb::task_scheduler_init::default_num_threads();
#ifdef __WIN_XP__
	g_uiWorkerIdInx = TlsAlloc();
	g_uiShedulerInx = TlsAlloc();
#endif

	ThreadIDAssigner::SetScheduler(NULL);
}

TBBTaskExecutor::~TBBTaskExecutor()
{
#ifdef __WIN_XP__
	TlsFree(g_uiWorkerIdInx);
	TlsFree(g_uiShedulerInx);
#endif
	if (m_threadPoolChangeObserver != NULL)
	{
		delete m_threadPoolChangeObserver;
		m_threadPoolChangeObserver = NULL;
	}		
}

int	TBBTaskExecutor::Init(unsigned int uiNumThreads, bool bUseTaskalyzer)
{
	unsigned long ulNewVal = ++m_lRefCount;
	if ( ulNewVal == 1 )
	{
		INIT_LOGGER_CLIENT(L"TBBTaskExecutor", LL_DEBUG);
		LOG_INFO("TBBTaskExecutor constructed to %d threads", gWorker_threads);
		LOG_INFO("TBBTaskExecutor initialized to %u threads", uiNumThreads);
		if (uiNumThreads > 0)
		{
			gWorker_threads = uiNumThreads;
		}
	}
	else if (uiNumThreads > 0)
	{
		if (uiNumThreads != gWorker_threads)
		{
			LOG_ERROR("Error: an attempt to re-init TBB Executor with a different number of threads. Trying to init %u threads, %d threads already used", uiNumThreads, gWorker_threads);
			assert(0);
		}
	}

	//Initialize the "available threads" mask
	gThreadAvailabilityMask.init(gWorker_threads, 1);

	if (NULL == m_scheduler)
	{
		m_scheduler = new tbb::task_scheduler_init(gWorker_threads + 1);		
	}

	if (NULL == sTBB_executor)
	{
		//sTBB_executor = new unordered_command_list();
		sTBB_executor = new ordered_command_list();
	}
	else
	{
		LOG_DEBUG("sTBB_executor was initialized to %0x", sTBB_executor);
	}

	m_threadPoolChangeObserver->observe();
	// Set TAL usage flag 
	m_threadPoolChangeObserver->SetUseTaskalyzer(bUseTaskalyzer);
	m_bUseTaskalyzer = bUseTaskalyzer;
	
	LOG_INFO(TEXT("%s"),"Done");	
	return gWorker_threads + 1;
}

unsigned int TBBTaskExecutor::GetNumWorkingThreads() const
{
	return gWorker_threads + 1;
}

ITaskList* TBBTaskExecutor::CreateTaskList(bool OOO)
{
	InitSchedulerForMasterThread();

	ITaskList *pList = NULL;
	if (OOO) 
	{
		pList = new unordered_command_list();
	}
	else
	{
		pList = new ordered_command_list();
	}

	return pList;
}

unsigned int TBBTaskExecutor::Execute(ITaskBase * pTask)
{
	sTBB_executor->Enqueue(pTask);
	sTBB_executor->Flush();		
	return 0;
}

bool TBBTaskExecutor::WaitForCompletion()
{
	return sTBB_executor->WaitForCompletion();
}

// Cancels execution of uncompleted tasks and and then release task executor resources
void TBBTaskExecutor::Close(bool bCancel)
{
	// Worker threads should never get here
	assert(!ThreadIDAssigner::IsWorkerScheduler());

	unsigned long ulNewVal = --m_lRefCount;

	if ( 0 != ulNewVal )
	{
		LOG_INFO(TEXT("%s"),"Still alive");
		return;
	}

	LOG_INFO(TEXT("%s"),"Shutting down...");

	if (m_scheduler != NULL) 
	{
		//m_scheduler->terminate();
		delete m_scheduler;
		m_scheduler = NULL;	
	}

	if ( NULL != sTBB_executor )
	{				
		delete sTBB_executor;
		sTBB_executor = NULL;
	}	
	
	m_threadPoolChangeObserver->observe(false);
	LOG_INFO(TEXT("%s"),"Done");
	RELEASE_LOGGER_CLIENT;
}

void TBBTaskExecutor::ReleasePerThreadData()
{
	// TBB was not initialized within calling thread or its TBB worker thread.
	tbb::task_scheduler_init* pScheduler = ThreadIDAssigner::GetScheduler();
	if ( ThreadIDAssigner::IsWorkerScheduler())
	{
		return;
	}
	if ((NULL != pScheduler) && (INVALID_SCHEDULER_ID != (uintptr_t)pScheduler))
	{				
		
		//pScheduler->terminate();
		delete pScheduler;		
		ThreadIDAssigner::SetScheduler(NULL);
		
		
	}
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
