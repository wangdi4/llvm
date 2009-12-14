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
#include <Windows.h>

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")

using namespace Intel::OpenCL::Utils;

//#define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

//! global TBB task scheduler objects
unsigned int					gTBB_threads = 0;
// Make TBB init class and WG executor thread local
tbb::task_scheduler_init*		gTBB_init = NULL;
tbb::task_group*				gTBB_executor = NULL;
// Global counter for worker thread enumeration
volatile long	g_ulThreadEnum;

// Worker thread id
DWORD g_dwMainThreadId;

#ifdef __WIN_XP__
unsigned int g_uiWorkerIdInx = -1;
#else
__declspec(thread) unsigned int t_uiWorkerId = -1;
#endif

// Logger
DECLARE_LOGGER_CLIENT;

namespace Intel { namespace OpenCL { namespace TaskExecutor {

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
		bool is_divisible() const {return 1<size();}

		//! Split range.  
		/** The new Range *this has the second half, the old range r has the first half. 
		Unspecified if end()<begin() or !is_divisible(). */
		Range1D( Range1D& r, tbb::split ) : 
		my_end(r.my_end),
			my_begin(do_split(r))
		{}

	private:
		/** NOTE: my_end MUST be declared before my_begin, otherwise the forking constructor will break. */
		size_t my_end;
		size_t my_begin;

		//! Auxiliary function used by forking constructor.
		/** Using this function lets us not require that Value support assignment or default construction. */
		static unsigned int do_split( Range1D& r ) {
			__TBB_ASSERT( r.is_divisible(), "cannot split blocked_range that is not divisible" );
			unsigned int middle = r.my_begin + (r.my_end-r.my_begin)/2u;
			r.my_end = middle;
			return middle;
		}

		friend class Range3D;
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
			my_grain_size = (unsigned int)((n+gTBB_threads*16-1)/(gTBB_threads*16));
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
		}

		//! The pages of the iteration space 
		const Range1D& pages() const {return my_pages;}

		//! The rows of the iteration space 
		const Range1D& rows() const {return my_rows;}

		//! The columns of the iteration space 
		const Range1D& cols() const {return my_cols;}

	};

	struct TaskLoopBody {
		ITaskSet &task;
		TaskLoopBody(ITaskSet &t) : task(t) {}
		void operator()(const Range3D& r) const {
			unsigned int uiWorkerId;
#ifdef __WIN_XP__
			uiWorkerId = (unsigned int)TlsGetValue(g_uiWorkerIdInx);;
#else
			uiWorkerId = t_uiWorkerId;
#endif
			if ( task.AttachToThread(uiWorkerId) != 0 )
				return;
            for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
				for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
					for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
						task.ExecuteIteration(k, j, i, uiWorkerId);
		}
	};

	// TODO: replace by lambda when available
	struct TaskSetBody {
		ITaskSet &task;
		TaskSetBody(ITaskSet &t) : task(t) {}
		void operator()() { 
			// TODO: How iterations in task sets can differ in workload? Define gs appropriately.
			unsigned int dim[3];
			int res = task.Init(dim);
			__TBB_ASSERT(res==0, "Init failed");
			if (res != 0)
			{
				task.Finish(FINISH_INIT_FAILED);
				return;
			}
			tbb::parallel_for(Range3D(dim), TaskLoopBody(task), tbb::auto_partitioner());
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

//! Out-of-order command list
class unordered_command_list : public ITaskList {
public:
	void *operator new(std::size_t bytes) { return scalable_malloc(bytes); }
	void operator delete(void *p) { scalable_free(p); }
	unsigned int Enqueue(ITaskBase* pTask)
	{
		if ( pTask->IsTaskSet() )
			gTBB_executor->run( TaskSetBody(*((ITaskSet*)pTask)) );
		else
			gTBB_executor->run( TaskBody(*((ITask*)pTask)) );
		return 0;
	}
	void Flush() {}
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
class ordered_command_list : public ITaskList {
	tbb::spin_mutex mutex;
	int ref_count;
//		typedef std::vector<ITaskBase*, tbb::scalable_allocator<ITaskBase*> > queue_t;
	typedef std::vector<ITaskBase* > queue_t;
    bool is_executed; //< shows whether the service task of the dispatcher runs
	queue_t input, queue, work; //< input queues of commands
public:
	void *operator new(std::size_t bytes) { return scalable_malloc(bytes); }
	void operator delete(void *p) { scalable_free(p); }
    ordered_command_list() : ref_count(1), is_executed(false) {
		LOG_INFO("Created at 0x%X", this);
        input.reserve(VECTOR_RESERVE); work.reserve(VECTOR_RESERVE); queue.reserve(VECTOR_RESERVE);
    }
	/*override*/ unsigned int Enqueue(ITaskBase* pTask) {
		input.push_back(pTask);
		//if( !is_executed ) Flush(); // TODO: evaluate performance difference of enabled vs disabled
		return 0;
	}
	/*override*/ void Flush() {
#ifdef _EXTENDED_LOG
		LOG_INFO("Enter");
#endif
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
            gTBB_executor->run( queue_executor_proxy(this) );
        } // else the lock is released here
#ifdef _EXTENDED_LOG
		LOG_INFO("Exit");
#endif
	}
	/*override*/ void Release() {
        assert( input.empty() ); // Flush() must be called before Release()
		tbb::spin_mutex::scoped_lock lock( mutex );
        assert( queue.empty() || is_executed );
		if( ! --ref_count ) {
			lock.release();
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
					unsigned int dim[3];
					int res = pTask->Init(dim);
					__TBB_ASSERT(res==0, "Init Failed");
					if (res != 0)
					{
						pTask->Finish(FINISH_INIT_FAILED);
						continue;
					}
					tbb::parallel_for(Range3D(dim), TaskLoopBody(*pTask), tbb::auto_partitioner());
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
	gTBB_threads = tbb::task_scheduler_init::default_num_threads() + 1;
#ifdef __WIN_XP__
	g_uiWorkerIdInx = TlsAlloc();
#endif

}

TBBTaskExecutor::~TBBTaskExecutor()
{
	if ( NULL != gTBB_init )
	{
		gTBB_init->terminate();
		delete gTBB_init;
		gTBB_init = NULL;
	}
#ifdef __WIN_XP__
	TlsFree(g_uiWorkerIdInx);
#endif
}

tbb::atomic<unsigned int> _barrier_cnt;
struct barrier {
	barrier(unsigned int n) { _barrier_cnt = n; }
	void operator()(unsigned int) const {
		if (g_dwMainThreadId == GetCurrentThreadId() )
		{
			// Main thread is not for execution
#ifdef __WIN_XP__
			TlsSetValue(g_uiWorkerIdInx, (void*)-1);
#else
			t_uiWorkerId = -1;
#endif
		} else
		{
			unsigned int uiWrkId = --_barrier_cnt;
#ifdef __WIN_XP__
			TlsSetValue(g_uiWorkerIdInx, (void*)uiWrkId);
#else
			t_uiWorkerId = uiWrkId;
#endif
		}
		while(_barrier_cnt);
	}
};

int	TBBTaskExecutor::Init(unsigned int uiNumThreads)
{
	unsigned long ulNewVal = InterlockedIncrement(&m_lRefCount);
	if ( ulNewVal == 1 )
	{
		INIT_LOGGER_CLIENT(L"TBBTaskExecutor", LL_DEBUG);
		LOG_INFO("TBBTaskExecutor constructed to %d threads", gTBB_threads);
		if( !uiNumThreads )
			uiNumThreads = gTBB_threads;
		else
		{
			gTBB_threads = uiNumThreads + 1;
		}
		
		gTBB_init = new tbb::task_scheduler_init( gTBB_threads );
		gTBB_executor = new tbb::task_group();
		// wait threads to warm up. Allocate thread id's
		// Store current thread id
		g_dwMainThreadId = GetCurrentThreadId();
		tbb::parallel_for(0U, gTBB_threads, 1U, barrier(gTBB_threads-1));
	}
	else
	{
		LOG_INFO("Local executor was already initialized to 0x%X", gTBB_executor);
	}
	LOG_INFO("Done");
	return uiNumThreads;
}

unsigned int TBBTaskExecutor::GetNumWorkingThreads() const
{
	return gTBB_threads-1;
}

ITaskList* TBBTaskExecutor::CreateTaskList(bool OOO)
{
	ITaskList *pList = OOO? (ITaskList*)(new unordered_command_list()) : (ITaskList*)(new ordered_command_list());

	return pList;
}

unsigned int TBBTaskExecutor::Execute(ITaskBase * pTask)
{
	if ( pTask->IsTaskSet() )
	{
#ifdef _EXTENDED_LOG
		LOG_INFO("TaskSet Enqueued %0X", pTask);
#endif
		gTBB_executor->run( TaskSetBody(*((ITaskSet*)pTask)) );
	}
	else
	{
#ifdef _EXTENDED_LOG
		LOG_INFO("Task Enqueued %0X", pTask);
#endif
		gTBB_executor->run( TaskBody(*((ITask*)pTask)) );
	}
	return 0;
}

// Cancels execution of uncompleted tasks and and then release task executor resources
void TBBTaskExecutor::Close(bool bCancel)
{
	unsigned long ulNewVal = InterlockedDecrement(&m_lRefCount);

	if ( 0 != ulNewVal )
	{
		LOG_INFO("Still alive");
		return;
	}
	LOG_INFO("Shutting down...");
	if ( NULL != gTBB_executor )
	{
		if ( bCancel )
		{
			gTBB_executor->cancel(); // the work may be canceled if not completed
		}

		gTBB_executor->wait();
		delete gTBB_executor;
		gTBB_executor = NULL;
	}

	gTBB_init->terminate();
	delete gTBB_init;
	gTBB_init = NULL;
	LOG_INFO("Done");
	RELEASE_LOGGER_CLIENT;
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
