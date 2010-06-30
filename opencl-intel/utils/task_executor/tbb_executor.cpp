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
#include "tbb_executor.h"
#include "tbb/tbb.h"
#include "tbb/scalable_allocator.h"
#include "logger.h"
#include <vector>
#include <cassert>
#include <tbb/blocked_range.h>
#include <tbb/atomic.h>
#include <Windows.h>

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")
using namespace Intel::OpenCL::Utils;

//#define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

#ifdef __WIN_XP__
unsigned int g_uiWorkerIdInx = -1;
unsigned int g_uiShedulerInx = -1;
#else
__declspec(thread) unsigned int t_uiWorkerId = -1;
__declspec(thread) tbb::task_scheduler_init* t_pScheduler = NULL;
#endif

//The variables below are used to ensure working threads have unique IDs in the range [0, numThreads - 1]
//The idea is that as threads enter the pool they get a unique identifier (NextAvailableThreadId) and use the thread ID from gAvailableThreadIds in that index
//When a thread leaves the pool, it decrements NextAvailableThreadId and writes his id to gAvailableThreadIds in the previous value
static volatile LONG gThreadAvailabilityMask = 0;

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
public:
	ThreadIDAssigner() : tbb::task_scheduler_observer() {}
	~ThreadIDAssigner() {}

	virtual void on_scheduler_entry( bool is_worker )
	{
		unsigned int uiWorkerId = 0;
		if ( is_worker )
		{
			long canExit = false;
			while (uiWorkerId < gWorker_threads)
			{
				// TODO: use 64 bit version when compiled under x64
				canExit = InterlockedBitTestAndReset(&gThreadAvailabilityMask, uiWorkerId);
				if (canExit) break;
				++uiWorkerId;
			}
			assert(uiWorkerId < gWorker_threads);
			++uiWorkerId;
			t_pScheduler = (tbb::task_scheduler_init*)-1;
		}
#ifdef _EXTENDED_LOG
		LOG_INFO("------->%s %d was joined as %d\n", is_worker ? "worker" : "master",
			GetThreadId(GetCurrentThread()), uiWorkerId);
#endif
#ifdef __WIN_XP__
		TlsSetValue(g_uiWorkerIdInx, (void*)uiWorkerId);
#else
		t_uiWorkerId = uiWorkerId;
#endif 

	}
	virtual void on_scheduler_exit( bool is_worker )
	{
#ifdef _EXTENDED_LOG
		LOG_INFO("------->%s %d was left as %d\n", is_worker ? "worker" : "master",
			GetThreadId(GetCurrentThread()), t_uiWorkerId);
#endif

		if ( (is_worker)  && (t_uiWorkerId != -1))
		{
			//assert(t_uiWorkerId != -1);
			long prevVal;
			--t_uiWorkerId;
			prevVal = InterlockedBitTestAndSet(&gThreadAvailabilityMask, t_uiWorkerId);
			//Just for extra safety, make sure we're not relinquishing an ID somebody already relinquished
			assert(prevVal==0);
			t_pScheduler = NULL;
#ifdef __WIN_XP__
			TlsSetValue(g_uiWorkerIdInx, (void*)-1);
#else
			t_uiWorkerId = -1;
#endif 
		}
	}
};

//A singleton copy of the observer class
static ThreadIDAssigner gThreadPoolChangeObserver;

namespace Intel { namespace OpenCL { namespace TaskExecutor {

	// Make TBB init class and WG executor thread local
	tbb::task_group* TBBTaskExecutor::sTBB_executor = NULL;

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

	struct TaskLoopBody3D {
		ITaskSet &task;
		TaskLoopBody3D(ITaskSet &t) : task(t) {}
		void operator()(const Range3D& r) const {
			unsigned int uiWorkerId;
#ifdef __WIN_XP__
			uiWorkerId = (unsigned int)TlsGetValue(g_uiWorkerIdInx);;
#else
			uiWorkerId = t_uiWorkerId;
#endif
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				long lMasterCheck = g_alMasterIdCheck.compare_and_swap(true, false);
				assert(lMasterCheck == false);
			}
#endif
			if ( task.AttachToThread(uiWorkerId) != 0 )
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
		void operator()(const Range2D& r) const {
			unsigned int uiWorkerId;
#ifdef __WIN_XP__
			uiWorkerId = (unsigned int)TlsGetValue(g_uiWorkerIdInx);;
#else
			uiWorkerId = t_uiWorkerId;
#endif
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				long lMasterCheck = g_alMasterIdCheck.compare_and_swap(true, false);
				assert(lMasterCheck == false);
			}
#endif
			if ( task.AttachToThread(uiWorkerId) != 0 )
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
		void operator()(const Range1D& r) const {
			unsigned int uiWorkerId;
#ifdef __WIN_XP__
			uiWorkerId = (unsigned int)TlsGetValue(g_uiWorkerIdInx);;
#else
			uiWorkerId = t_uiWorkerId;
#endif
#ifdef _DEBUG
			if ( 0 == uiWorkerId )
			{
				long lMasterCheck = g_alMasterIdCheck.compare_and_swap(true, false);
				assert(lMasterCheck == false);
			}
#endif
			if ( task.AttachToThread(uiWorkerId) != 0 )
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
			unsigned int dim[3], dimCount;
			int res = task.Init(dim, dimCount);
			__TBB_ASSERT(res==0, "Init failed");
			if (res != 0)
			{
				task.Finish(FINISH_INIT_FAILED);
				return;
			}
			if (1 == dimCount)
			{
				tbb::parallel_for(Range1D(0, dim[0]), TaskLoopBody1D(task), tbb::auto_partitioner());
			} else if (2 == dimCount)
			{
				tbb::parallel_for(Range2D(dim), TaskLoopBody2D(task), tbb::auto_partitioner());
			} else
			{
				tbb::parallel_for(Range3D(dim), TaskLoopBody3D(task), tbb::auto_partitioner());
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
	base_command_list()
	{
		task_group = new tbb::task_group();
	}

	virtual ~base_command_list()
	{
		task_group->wait();
		delete task_group;
	}
	bool WaitForCompletion()
	{
		// Initialize TBB if not, required for master threads
		if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
		{
			GetTaskExecutor()->Init(0);
		}
		long lVal = g_alMasterRunning.compare_and_swap(true, false);
		if (lVal)
		{
			// When another master is running we can't block this thread
			return false;
		}
		tbb::task_group_status st = task_group->wait();
		g_alMasterRunning.fetch_and_store( false );
		return st == tbb::complete;
	}

protected:
	tbb::task_group*	task_group;
};

//! Out-of-order command list
class unordered_command_list : base_command_list
{
public:
	void *operator new(std::size_t bytes) { return scalable_malloc(bytes); }
	void operator delete(void *p) { scalable_free(p); }
	unsigned int Enqueue(ITaskBase* pTask)
	{
		// Initialize TBB if not, required for master threads
		if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
		{
			GetTaskExecutor()->Init(0);
		}
		if ( pTask->IsTaskSet() )
			task_group->run( TaskSetBody(*((ITaskSet*)pTask)) );
		else
			task_group->run( TaskBody(*((ITask*)pTask)) );
		return 0;
	}
	void Flush()
	{
		// Initialize TBB if not, required for master threads
		if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
		{
			GetTaskExecutor()->Init(0);
		}
	}
	void Release() { LOG_INFO("Delete 0x%X", this); delete this; }
};

//! In-order command list
class ordered_command_list;

//! functor for service task
struct queue_executor_proxy {
	ordered_command_list *list;
	queue_executor_proxy(ordered_command_list *l) : list( l ) {}
	inline void operator()();
};

// implementation of in-order command list dispatcher
class ordered_command_list : public base_command_list
{
	tbb::spin_mutex mutex;
	tbb::atomic<int> ref_count;
//		typedef std::vector<ITaskBase*, tbb::scalable_allocator<ITaskBase*> > queue_t;
	typedef std::vector<ITaskBase* > queue_t;
    tbb::atomic<bool> is_executed; //< shows whether the service task of the dispatcher runs
	queue_t input, queue, work; //< input queues of commands
public:
	void *operator new(std::size_t bytes) { return scalable_malloc(bytes); }
	void operator delete(void *p) { scalable_free(p); }
    ordered_command_list() {
		ref_count = 1;
		is_executed = false;
		LOG_INFO("Created at 0x%X", this);
        input.reserve(VECTOR_RESERVE); work.reserve(VECTOR_RESERVE); queue.reserve(VECTOR_RESERVE);
    }
	/*override*/
	unsigned int Enqueue(ITaskBase* pTask)
	{
		// Initialize TBB if not, required for master threads
		if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
		{
			GetTaskExecutor()->Init(0);
		}
		input.push_back(pTask);
		//if( !is_executed ) Flush(); // TODO: evaluate performance difference of enabled vs disabled
		return 0;
	}
	/*override*/ void Flush() {
#ifdef _EXTENDED_LOG
		LOG_INFO("Enter");
#endif
		// Initialize TBB if not, required for master threads
		if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
		{
			GetTaskExecutor()->Init(0);
		}
        tbb::spin_mutex::scoped_lock lock;
        bool is_locked = is_executed;
        if( is_locked ) lock.acquire(mutex); // no sync necessary otherwise
        if( !input.empty() ) {
		    if( queue.empty() )
			    queue.swap( input );
		    else { // move items
			    queue.insert( queue.end(), input.begin(), input.end() );
			    input.clear();
		    }
        }
        if( !is_executed && !queue.empty() ) {
            assert(ref_count > 0);
			ref_count++; // service task will hold the reference
            is_executed = true; // run the service task
            if( is_locked ) lock.release(); // without the lock
            task_group->run( queue_executor_proxy(this) );
        } // else the lock is released here
#ifdef _EXTENDED_LOG
		LOG_INFO("Exit");
#endif
	}
	/*override*/ void Release() {
        assert( input.empty() ); // Flush() must be called before Release()
		if( ! --ref_count ) {
			assert(!is_executed && queue.empty());
			LOG_INFO("Deleted at 0x%X", this);
			delete this;
		}
	}

	//! Service task of dispatcher. Executes the commands.
	int queue_executor() {
#ifdef _EXTENDED_LOG
		LOG_INFO("Enter");
#endif
		do {
			work.clear();
			{ // get new work
				tbb::spin_mutex::scoped_lock lock(mutex);
				queue.swap( work );
                assert( queue.empty() );
				if( work.empty() ) {
					is_executed = false; // no work available, stop the task
#ifdef _EXTENDED_LOG
					LOG_INFO("Exit - no work");
#endif
					return --ref_count;
				}
			} // release the lock
			for(queue_t::iterator cmd = work.begin(); cmd != work.end(); cmd++)
				if ( (*cmd)->IsTaskSet() )
				{
					ITaskSet* pTask = (ITaskSet*)(*cmd);
					unsigned int dim[MAX_WORK_DIM], dimCount;
					int res = pTask->Init(dim, dimCount);
					__TBB_ASSERT(res==0, "Init Failed");
					if (res != 0)
					{
						pTask->Finish(FINISH_INIT_FAILED);
						continue;
					}
					if (1 == dimCount)
					{
						tbb::parallel_for(Range1D(0, dim[0]), TaskLoopBody1D(*pTask), tbb::auto_partitioner());
					} else if (2 == dimCount)
					{
						tbb::parallel_for(Range2D(dim), TaskLoopBody2D(*pTask), tbb::auto_partitioner());
					} else
					{
						tbb::parallel_for(Range3D(dim), TaskLoopBody3D(*pTask), tbb::auto_partitioner());
					}
					pTask->Finish(FINISH_COMPLETED);
				}
				else
				{
					((ITask*)(*cmd))->Execute(); // execute one by one
				}
			for(queue_t::iterator cmd = work.begin(); cmd != work.end(); cmd++)
				(*cmd)->Release();
		} while(true);
		//return 0;
	}
};

void queue_executor_proxy::operator()() {
	if( list->queue_executor() == 0 )
		delete list; // delete the command list if ref_count == 0
}

/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() : m_lRefCount(0)
{
	g_alMasterRunning = false;
#ifdef _DEBUG
	g_alMasterIdCheck = false;
#endif
	gWorker_threads = tbb::task_scheduler_init::default_num_threads();
#ifdef __WIN_XP__
	g_uiWorkerIdInx = TlsAlloc();
	g_uiShedulerInx = TlsAlloc();
#endif

	//Enable observation of thread addition/removal from the working thread pool
	gThreadPoolChangeObserver.observe(); 
}

TBBTaskExecutor::~TBBTaskExecutor()
{
#ifdef __WIN_XP__
	TlsFree(g_uiWorkerIdInx);
	TlsFree(g_uiShedulerInx);
#endif
	gThreadPoolChangeObserver.observe(false); 
}

int	TBBTaskExecutor::Init(unsigned int uiNumThreads)
{
	unsigned long ulNewVal = InterlockedIncrement(&m_lRefCount);
	if ( ulNewVal == 1 )
	{
		INIT_LOGGER_CLIENT(L"TBBTaskExecutor", LL_DEBUG);
		LOG_INFO("TBBTaskExecutor constructed to %d threads", gWorker_threads);
		if( !uiNumThreads )
			uiNumThreads = gWorker_threads;
		else
		{
			gWorker_threads = uiNumThreads;
		}

		//Initialize the "available threads" mask
		gThreadAvailabilityMask = (1 << (gWorker_threads)) - 1;

		t_pScheduler = new tbb::task_scheduler_init(gWorker_threads + 1);
		sTBB_executor = new tbb::task_group();
	}
	else
	{
		t_pScheduler = new tbb::task_scheduler_init(gWorker_threads + 1);
		LOG_INFO("Local executor was already initialized to 0x%X", sTBB_executor);
	}

	gThreadPoolChangeObserver.observe();
	LOG_INFO("Done");
	return gWorker_threads+1;
}

unsigned int TBBTaskExecutor::GetNumWorkingThreads() const
{
	return gWorker_threads + 1;
}

ITaskList* TBBTaskExecutor::CreateTaskList(bool OOO)
{
	// Initialize TBB if not, required for master threads
	if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
	{
		GetTaskExecutor()->Init(0);
	}
	ITaskList *pList = OOO? (ITaskList*)(new unordered_command_list()) : (ITaskList*)(new ordered_command_list());

	return pList;
}

unsigned int TBBTaskExecutor::Execute(ITaskBase * pTask)
{
	// Initialize TBB if not, required for master threads
	if ( (NULL == t_pScheduler) && (-1 != (size_t)t_pScheduler) )
	{
		GetTaskExecutor()->Init(0);
	}

	if ( pTask->IsTaskSet() )
	{
#ifdef _EXTENDED_LOG
		LOG_INFO("TaskSet Enqueued %0X", pTask);
#endif
		sTBB_executor->run( TaskSetBody(*((ITaskSet*)pTask)) );
	}
	else
	{
#ifdef _EXTENDED_LOG
		LOG_INFO("Task Enqueued %0X", pTask);
#endif
		sTBB_executor->run( TaskBody(*((ITask*)pTask)) );
	}
	return 0;
}

bool TBBTaskExecutor::WaitForCompletion()
{
	long lVal = g_alMasterRunning.compare_and_swap(true, false);
	if (lVal)
	{
		// When another master is running we can't block this thread
		return false;
	}
	tbb::task_group_status st = sTBB_executor->wait();
	g_alMasterRunning.fetch_and_store( false );
	return st == tbb::complete;
}
// Cancels execution of uncompleted tasks and and then release task executor resources
void TBBTaskExecutor::Close(bool bCancel)
{
	// TBB was not initialized within calling thread or it's TBB worker thread.
	if ( (NULL == t_pScheduler) || (-1 == (size_t)t_pScheduler) )
	{
		return;
	}

	unsigned long ulNewVal = InterlockedDecrement(&m_lRefCount);

	if ( 0 != ulNewVal )
	{
		LOG_INFO("Still alive");
		delete t_pScheduler;
		t_pScheduler = NULL;
		return;
	}
	LOG_INFO("Shutting down...");
	if ( NULL != sTBB_executor )
	{
		if ( bCancel )
		{
			sTBB_executor->cancel(); // the work may be canceled if not completed
		}

		sTBB_executor->wait();
		delete sTBB_executor;
		sTBB_executor = NULL;
	}

	t_pScheduler->terminate();
	delete t_pScheduler;
	t_pScheduler = NULL;
	gThreadPoolChangeObserver.observe(false);
	LOG_INFO("Done");
	RELEASE_LOGGER_CLIENT;
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
