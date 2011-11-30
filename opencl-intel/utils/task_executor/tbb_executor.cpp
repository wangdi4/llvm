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
#include "tbb_executor.h"
#include "Logger.h"
#include <ocl_itt.h>

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

// Logger
DECLARE_LOGGER_CLIENT;
namespace Intel { namespace OpenCL { namespace TaskExecutor {

//Implementation of the interface to be notified on thread addition/removal from the working thread pool
ThreadIDAssigner::ThreadIDAssigner() : tbb::task_scheduler_observer()
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

ThreadIDAssigner::~ThreadIDAssigner()
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

unsigned int ThreadIDAssigner::GetWorkerID()
{
	bool alreadyHad = false;
	unsigned int ret = t_uiWorkerId->local(alreadyHad);
	return alreadyHad ? ret : INVALID_WORKER_ID;
}

void ThreadIDAssigner::SetWorkerID(unsigned int id)
{
	t_uiWorkerId->local() = id;
}

tbb::task_scheduler_init* ThreadIDAssigner::GetScheduler()
{
	bool alreadyHad = false;
	tbb::task_scheduler_init* ret = t_pScheduler->local(alreadyHad);
	return alreadyHad ? ret : NULL;
}

void ThreadIDAssigner::SetScheduler(tbb::task_scheduler_init* init)
{						
	t_pScheduler->local() = init;		
}

bool ThreadIDAssigner::IsWorkerScheduler()
{
	return (INVALID_SCHEDULER_ID == (cl_ulong)GetScheduler());
}

void ThreadIDAssigner::on_scheduler_entry( bool is_worker )
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

void ThreadIDAssigner::on_scheduler_exit( bool is_worker )
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
	}
}


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
		//t_pScheduler = new tbb::task_scheduler_init(gWorker_threads);
		ThreadIDAssigner::SetScheduler(new tbb::task_scheduler_init(gWorker_threads));
	}
}



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
			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
			for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
				for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
					task.ExecuteIteration(k, j, 0, uiWorkerId);
			task.DetachFromThread(uiWorkerId);
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
			if ( task.AttachToThread(uiWorkerId, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;

			for(size_t k = r.begin(), f = r.end(); k < f; k++ )
					task.ExecuteIteration(k, 0, 0, uiWorkerId);
			task.DetachFromThread(uiWorkerId);
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
	bool	IsFired() {return m_bFired;}

	// ITask interface
	virtual bool	Execute() { m_bFired = true; return false;}
	virtual void	Release() {}	//Persistent member, don't release

protected:
	volatile bool	m_bFired;
};

typedef OclNaiveConcurrentQueue<ITaskBase*> ConcurrentTaskQueue;
typedef std::vector<ITaskBase*>           TaskVector;

class base_command_list : public ITaskList
{
public:
	base_command_list(bool subdevice) :
		m_context(tbb::task_group_context::bound, tbb::task_group_context::concurrent_wait),
		m_subdevice(subdevice)
	{
		m_rootTask = new (tbb::task::allocate_root(m_context)) deleting_task(this);
		m_rootTask->set_ref_count(1);
		m_execTaskRequests = 0;
		m_bMasterRunning = false;
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

	te_wait_result WaitForCompletion()
	{
		InitSchedulerForMasterThread();

		bool bVal = m_bMasterRunning.compare_and_swap(true, false);
		if (bVal)
		{
			// When another master is running we can't block this thread
			return TE_WAIT_MASTER_THREAD_BLOCKING;
		}

		// Request processing task to stop
		unsigned int ret = RequestStopProcessing();
		while (ret > 0)
		{
			// This master started the task, so need wait for completion
			m_rootTask->wait_for_all();

			if ( m_MasterSync.IsFired() )
			{
				break;
			}
			ret = InternalFlush(true);
		}

		// If there's some incoming work during operation, try to flush it
		if ( HaveIncomingWork() )
		{
			Flush();
		}
		// Current master is not in charge for the work
		m_bMasterRunning = false;

		return TE_WAIT_COMPLETED;
	}

	bool Flush()
	{
		InternalFlush(false);
		return true;
	}

	void Release() 
	{ 
		m_rootTask->decrement_ref_count();
		//tbb::task::enqueue(*m_rootTask);
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

	// Ordered and OOO command lists have different task objects operating on them
	virtual unsigned int LaunchExecutorTask(bool blocking) = 0;

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

	inline unsigned int RequestStopProcessing()
	{
		m_MasterSync.Reset();

		Enqueue(&m_MasterSync);
		// Must flush here or risk deadlock - the enqueued NULL can result in an early exit at the expense of another flush
		// That will be lost as a result of storing 0 to taskExecuteRequests
		return InternalFlush(true);
	}

	tbb::task_group_context m_context;
    bool                    m_subdevice;
    tbb::task*              m_rootTask;

	ConcurrentTaskQueue		m_quIncomingWork;

	tbb::atomic<unsigned int>	m_execTaskRequests;
	SyncTask	m_MasterSync;

	// Only single muster thread can join the execution on specific queue
	// this mutex will block others. The atomic prevents wait on
	// the master if another master is running
	tbb::atomic<bool>		m_bMasterRunning;

private:
	//Disallow copy constructor
	base_command_list(const base_command_list& l) : ITaskList() {}

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

};

static bool execute_command(ITaskBase* cmd)
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
		pTask->Finish(FINISH_COMPLETED);
		return true;
	}
	else
	{
        ITask* pCmd = static_cast<ITask*>(cmd);
		return pCmd->Execute();
	}
}

class ordered_command_list : public base_command_list
{
public:
	ordered_command_list(bool subdevice) : base_command_list(subdevice)	{}

	virtual unsigned int LaunchExecutorTask(bool blocking);

protected:
};


class unordered_command_list : public base_command_list
{
public:
    unordered_command_list(bool subdevice) : base_command_list(subdevice) {}

	virtual unsigned int LaunchExecutorTask(bool blocking);

	// Move work from incoming queue to work queue
	// Todo: currently this is serial, see if we can parallelize it somehow
	size_t FillWork(TaskVector& workList)
	{
		ITaskBase* current;
		while (m_quIncomingWork.TryPop(current))
		{
			workList.push_back(current);
		}
		return workList.size();
	}

protected:
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

class in_order_executor_task : public tbb::task
{
public:
	in_order_executor_task(ordered_command_list* list) : m_list(list){}

	tbb::task* execute()
	{
		assert(m_list);
		ConcurrentTaskQueue* work = m_list->GetExecutingContainer();
		assert(work);
		TaskVector forDeletion;
		ITaskBase* currentTask;
		bool mustExit = false;

		while(true)
		{
			//First check if we need to stop interating, next get next available record
			while( !mustExit && work->TryPop(currentTask))
			{
				mustExit = !execute_command(currentTask); //stop requested

				forDeletion.push_back(currentTask);
			}

			//release all executed tasks
			for (TaskVector::const_iterator it = forDeletion.begin(); it != forDeletion.end(); ++it)
			{
				(*it)->Release();
			}
			forDeletion.clear();

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

		return NULL;
	}

protected:
	ordered_command_list* m_list;
};

class unorder_executor_task : public tbb::task
{
public:
	unorder_executor_task(unordered_command_list* list) : m_list(list) {}

	tbb::task* execute()
	{
		assert(m_list);
		TaskVector		work;
		volatile bool	mustExit = false;

		while (true)
		{
			size_t size = m_list->FillWork(work);
			if ( size > 0 )
			{

				//Todo: handle small sizes without parallel for
				tbb::blocked_range<size_t> r(0, size);
				//for out of order, do parallel for
				parallel_for(r, ExecuteContainerBody(&work, &mustExit));

				work.clear();

				if ( mustExit )
				{
					m_list->m_execTaskRequests.fetch_and_store(0);
					break;
				}
			}

			if ( 1 == m_list->m_execTaskRequests-- )
			{
				break;
			}
		}


		return NULL;
	}
protected:

	unordered_command_list* m_list;
};

unsigned int ordered_command_list::LaunchExecutorTask(bool blocking)
{
	if (!m_subdevice)
	{
		InitSchedulerForMasterThread();

		m_rootTask->increment_ref_count();
		tbb::task* executor = new (m_rootTask->allocate_child()) in_order_executor_task(this);
		
		assert(executor);
		if ( NULL == executor )
		{
			tbb::task::destroy(*executor);
			m_execTaskRequests = 0;
			return 0;
		}
		
		if (!blocking)
		{
			tbb::task::enqueue(*executor);
			return 1;
		}

		m_rootTask->spawn_and_wait_for_all(*executor);
	}
	else
	{
		in_order_executor_task queueTask(this);
		queueTask.execute();
	}

	return 0;
}

unsigned int unordered_command_list::LaunchExecutorTask(bool blocking)
{
	if (!m_subdevice)
	{
		InitSchedulerForMasterThread();

		m_rootTask->increment_ref_count();
		tbb::task* executor = new (m_rootTask->allocate_child()) unorder_executor_task(this);
		assert(executor);
		if ( NULL == executor)
		{
			tbb::task::destroy(*executor);
			m_execTaskRequests = 0;
			return 0;
		}
		if ( !blocking )
		{
			tbb::task::enqueue(*executor);
			return 1;
		}

		m_rootTask->spawn_and_wait_for_all(*executor);
	}
	else
	{
		// Always blocking
		unorder_executor_task queueTask(this);
		queueTask.execute();
	}

	return 0;
}

/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() : m_lRefCount(0)
{
#if defined(USE_GPA)   
    m_scheduler = NULL;
#endif
	m_threadPoolChangeObserver = new ThreadIDAssigner;

	gWorker_threads = tbb::task_scheduler_init::default_num_threads();

	ThreadIDAssigner::SetScheduler(NULL);
}

TBBTaskExecutor::~TBBTaskExecutor()
{
	if (m_threadPoolChangeObserver != NULL)
	{
		delete m_threadPoolChangeObserver;
		m_threadPoolChangeObserver = NULL;
	}		
}

int	TBBTaskExecutor::Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData)
{
	unsigned long ulNewVal = ++m_lRefCount;

	m_pGPAData = pGPAData;
	
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

    // if using GPA, initialize the "global" task scheduler init
    // Otherwise, initialize this master thread's observer
#if defined(USE_GPA)   
    if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
	    if (NULL == m_scheduler)
	    {
		    m_scheduler = new tbb::task_scheduler_init(gWorker_threads);		
	    }
    }
    else
    {
#endif
    InitSchedulerForMasterThread();
#if defined(USE_GPA)   
    }
#endif
	if (NULL == sTBB_executor)
	{
		//sTBB_executor = new unordered_command_list();
		sTBB_executor = new ordered_command_list(false);
	}
	else
	{
		LOG_DEBUG("sTBB_executor was initialized to %0x", sTBB_executor);
	}

	m_threadPoolChangeObserver->observe();
	
	LOG_INFO(TEXT("%s"),"Done");	
	return gWorker_threads;
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
		pList = new unordered_command_list(subdevice);
	}
	else
	{
		pList = new ordered_command_list(subdevice);
	}

	return pList;
}

unsigned int TBBTaskExecutor::Execute(ITaskBase * pTask)
{
	sTBB_executor->Enqueue(pTask);
	sTBB_executor->Flush();		
	return 0;
}

te_wait_result TBBTaskExecutor::WaitForCompletion()
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

ocl_gpa_data* TBBTaskExecutor::GetGPAData() const
{
	return m_pGPAData;
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
