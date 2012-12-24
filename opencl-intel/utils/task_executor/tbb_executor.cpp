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
#include "cl_shared_ptr.hpp"
#include "base_command_list.hpp"

using namespace Intel::OpenCL::Utils;

//#define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

namespace Intel { namespace OpenCL { namespace TaskExecutor {

void RegisterReleaseSchedulerForMasterThread();
// Logger
DECLARE_LOGGER_CLIENT;

//! global TBB task scheduler objects
unsigned int gWorker_threads = 0;
AtomicCounter	glTaskSchedCounter;

volatile bool gIsExiting = false;

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

    struct TaskLoopBody
    {
    protected:

        ArenaHandler& m_devArenaHandler;

        TaskLoopBody(ArenaHandler& devArenaHandler) : m_devArenaHandler(devArenaHandler) { }
    };

	struct TaskLoopBody3D : public TaskLoopBody {
		ITaskSet &task;
        TaskLoopBody3D(ArenaHandler& devArenaHandler, ITaskSet &t) : TaskLoopBody(devArenaHandler), task(t) {}
		void operator()(const tbb::blocked_range3d<size_t>& r) const {
			size_t uiNumberOfWorkGroups;

            size_t firstWGID[3] = {r.pages().begin(), r.rows().begin(),r.cols().begin()}; 
			size_t lastWGID[3] = {r.pages().end(),r.rows().end(),r.cols().end()}; 

			uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

            WGContextBase* const pWGContext = m_devArenaHandler.GetWGContext();
            if ( task.AttachToThread(pWGContext, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
            for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
				for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
					for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
                        task.ExecuteIteration(k, j, i, pWGContext);
            task.DetachFromThread(pWGContext);
		}
	};

	struct TaskLoopBody2D : public TaskLoopBody {
		ITaskSet &task;
        TaskLoopBody2D(ArenaHandler& devArenaHandler, ITaskSet &t) : TaskLoopBody(devArenaHandler), task(t) {}
		void operator()(const tbb::blocked_range2d<size_t>& r) const {
			size_t uiNumberOfWorkGroups;
            size_t firstWGID[2] = {r.rows().begin(),r.cols().begin()}; 
			size_t lastWGID[2] = {r.rows().end(),r.cols().end()}; 

			uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

            WGContextBase* const pWGContext = m_devArenaHandler.GetWGContext();
            if ( task.AttachToThread(pWGContext, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;
			for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
				for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
					task.ExecuteIteration(k, j, 0, pWGContext);
            task.DetachFromThread(pWGContext);
		}
	};    

	struct TaskLoopBody1D : public TaskLoopBody {
		ITaskSet &task;
        TaskLoopBody1D(ArenaHandler& devArenaHandler, ITaskSet &t) : TaskLoopBody(devArenaHandler), task(t) {}
		void operator()(const tbb::blocked_range<size_t>& r) const {
			size_t uiNumberOfWorkGroups;
			size_t firstWGID[1] = {r.begin()}; 
			size_t lastWGID[1] = {r.end()}; 

			uiNumberOfWorkGroups = r.size();
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);
            
            WGContextBase* const pWGContext = m_devArenaHandler.GetWGContext();
            if ( task.AttachToThread(pWGContext, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
				return;

			for(size_t k = r.begin(), f = r.end(); k < f; k++ )
					task.ExecuteIteration(k, 0, 0, pWGContext);
            task.DetachFromThread(pWGContext);
		}
	};

// The following functor classes can be replaced by lambda expressions once C++11 is supported by all compilers we use:

class ParallelForFunctor
{
protected:

    const size_t* const m_dim;
    ITaskSet& m_task;
    ArenaHandler& m_devArenaHandler;

    ParallelForFunctor(const size_t* dim, ITaskSet& task, ArenaHandler& devArenaHandler) : m_dim(dim), m_task(task), m_devArenaHandler(devArenaHandler) { }

};

class Dim1ParallelForFunctor : public ParallelForFunctor
{
public:
    Dim1ParallelForFunctor(const size_t* dim, ITaskSet& task, ArenaHandler& devArenaHandler) : ParallelForFunctor(dim, task, devArenaHandler) { }
    void operator()()
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, m_dim[0]), TaskLoopBody1D(m_devArenaHandler, m_task), tbb::auto_partitioner());
    }
};
class Dim2ParallelForFunctor : public ParallelForFunctor
{
public:
    Dim2ParallelForFunctor(const size_t* dim, ITaskSet& task, ArenaHandler& devArenaHandler) : ParallelForFunctor(dim, task, devArenaHandler) { }
    void operator()()
    {
        tbb::parallel_for(tbb::blocked_range2d<size_t>(0, m_dim[1],	0, m_dim[0]), TaskLoopBody2D(m_devArenaHandler, m_task), tbb::auto_partitioner());
    }
};
class Dim3ParallelForFunctor : public ParallelForFunctor
{
public:
    Dim3ParallelForFunctor(const size_t* dim, ITaskSet& task, ArenaHandler& devArenaHandler) : ParallelForFunctor(dim, task, devArenaHandler) { }
    void operator()()
    {
        tbb::parallel_for(tbb::blocked_range3d<size_t>(0, m_dim[2],	0, m_dim[1], 0, m_dim[0]), TaskLoopBody3D(m_devArenaHandler, m_task), tbb::auto_partitioner());
    }
};
static bool execute_command(const SharedPtr<ITaskBase>& pCmd, base_command_list& cmdList)
{
	bool runNextCommand = true;
    ITaskBase* cmd = pCmd.GetPtr();

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
			Dim1ParallelForFunctor functor(dim, *pTask, cmdList.GetDevArenaHandler());
            cmdList.ExecuteFunction(functor);
		} else if (2 == dimCount)
		{
			assert(dim[0] <= CL_MAX_INT32);
			assert(dim[1] <= CL_MAX_INT32);
			Dim2ParallelForFunctor functor(dim, *pTask, cmdList.GetDevArenaHandler());
            cmdList.ExecuteFunction(functor);
		} else
		{
			assert(dim[0] <= CL_MAX_INT32);
			assert(dim[1] <= CL_MAX_INT32);
			assert(dim[2] <= CL_MAX_INT32);
			Dim3ParallelForFunctor functor(dim, *pTask, cmdList.GetDevArenaHandler());
            cmdList.ExecuteFunction(functor);
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

void in_order_executor_task::operator()()
{    
	assert(m_list);
	ConcurrentTaskQueue* work = m_list->GetExecutingContainer();
	assert(work);
	TaskVector currentCommandBatch;
	SharedPtr<ITaskBase> currentTask;
	bool mustExit = false;

	while(true)
	{
		//First check if we need to stop interating, next get next available record
		while( !(mustExit && m_list->m_bMasterRunning) && work->TryPop(currentTask))
		{
			mustExit = !execute_command(currentTask, *m_list); //stop requested            

			currentCommandBatch.push_back(currentTask);

			if ( currentCommandBatch.size() >= MAX_BATCH_SIZE )
			{
				// When we excide maximum allowed command batch
				// We need to release, prevent memory overflow
                currentCommandBatch.clear();
			}
		}

        currentCommandBatch.clear();

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
}

struct ExecuteContainerBody
{
    const SharedPtr<ITaskBase> m_pTask;
    out_of_order_command_list& m_list;

    ExecuteContainerBody(const SharedPtr<ITaskBase>& pTask, out_of_order_command_list& list) :
			m_pTask(pTask), m_list(list) {}

	void operator()()
	{
        execute_command(m_pTask, m_list);
	}
};

void out_of_order_executor_task::operator()()
{
    while (true)
    {
        /* We still have a proble in case the user calls clFinish from one thread and then enqueues a command from another thread - the second command will wait until the SyncTask of the 
            clFinish is completed, although it could be executed without waiting. However, this is a rare case and we won't handle it for now. */
        SharedPtr<ITaskBase> pTask = GetTask();
        while (NULL != pTask && NULL == dynamic_cast<SyncTask*>(pTask.GetPtr()))
        {
			ExecuteContainerBody functor(pTask, *m_list);
            m_list->EnqueueFunction<ExecuteContainerBody>(functor);
            pTask = GetTask();
        }
        if (NULL != pTask && NULL != dynamic_cast<SyncTask*>(pTask.GetPtr()))
        {
            // synchronization point
            m_list->WaitOnOOOTaskGroup();
            static_cast<SyncTask*>(pTask.GetPtr())->Execute();
            m_list->m_execTaskRequests.fetch_and_store(0);
            break;
        }
        if (1 == m_list->m_execTaskRequests--)
        {                
            break;
        }
    }
}

SharedPtr<ITaskBase> out_of_order_executor_task::GetTask()
{
    SharedPtr<ITaskBase> pTask;
    if (m_list->GetExecutingContainer()->TryPop(pTask))
    {
        return pTask;
    }
    else
    {
        return NULL;
    }        
}

static void Terminate()
{
    gIsExiting = true;
}

/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() :
    m_lActivateCount(0), m_pExecutorList(NULL), m_pGlobalArenaHandler(NULL), m_pWgContextPool(NULL)
{
	INIT_LOGGER_CLIENT("TBBTaskExecutor", LL_DEBUG);
#if _DEBUG
    const int ret =
#endif
        atexit(Terminate);
#if _DEBUG
    assert(0 == ret);
#endif
    // we delibrately don't delete m_pScheduler (see comment above its definition)
}

TBBTaskExecutor::~TBBTaskExecutor()
{
    m_pExecutorList = NULL;
    /* We don't delete m_pGlobalArenaHandler because of all kind of TBB issues in the shutdown sequence, but since this destructor is called when the whole library goes down and m_pGlobalArenaHandler
       is a singleton, it doesn't really matter. */
    // TBB seem to have a bug in ~task_scheduler_init(), so we work around it by not deleting m_pScheduler (TBB bug #1955)
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

	m_pGPAData = pGPAData;
	
	// Explicitly load TBB library
	if ( !LoadTBBLibrary() )
	{
		LOG_ERROR(TEXT("%s"), "Failed to load TBB library");
		return 0;
	}

	if (uiNumThreads > 0)
	{
		gWorker_threads = uiNumThreads;
	}
	else
	{
		gWorker_threads = Intel::OpenCL::Utils::GetNumberOfProcessors();
	}
    m_pScheduler = new tbb::task_scheduler_init(tbb::task_scheduler_init::deferred);
    if (NULL == m_pScheduler)
    {
        LOG_ERROR(TEXT("%s"), "Failed to allocate task_scheduler_init");
        return 0;
    }
    m_pScheduler->initialize(gWorker_threads + 1);   // see comment above the definition of m_pScheduler
    m_pGlobalArenaHandler = new RootDevArenaHandler(gWorker_threads, gWorker_threads, *this);

	LOG_INFO(TEXT("TBBTaskExecutor constructed to %d threads"), gWorker_threads);
	LOG_INFO(TEXT("TBBTaskExecutor initialized to %u threads"), uiNumThreads);
	LOG_INFO(TEXT("%s"),"Done");	
	return gWorker_threads;
}

bool TBBTaskExecutor::Activate()
{
	LOG_INFO(TEXT("Enter count = %ld, SchedCount=%ld"), m_lActivateCount, (long)glTaskSchedCounter);
	assert(gWorker_threads && "TBB executor should be initialized first");
	OclAutoMutex mu(&m_mutex);

	if ( 0 != m_lActivateCount )
	{
		// We are already acticated
		long newCount = ++m_lActivateCount;
		long newSchedCount = ++glTaskSchedCounter;
		LOG_INFO(TEXT("Leave count = %ld, SchedCount=%ld"), newCount, newSchedCount);
		return true;
	}
	m_lActivateCount++;
	glTaskSchedCounter++;
	LOG_INFO(TEXT("Leave count = %ld, SchedCount=%ld"), m_lActivateCount, (long)glTaskSchedCounter);
    AllocateResources();
	return true;
}

bool TBBTaskExecutor::AllocateResources()
{
	LOG_INFO(TEXT("%s"), "Enter");

	m_pExecutorList = in_order_command_list::Allocate(false, this, *m_pGlobalArenaHandler); // TODO: Why not unordered
	if ( NULL == m_pExecutorList )
	{
		LOG_ERROR(TEXT("%s"), "Failed to allocate m_pExecutorList");
		return false;
	}

	LOG_INFO(TEXT("%s"), "Leave");
	return true;
}

void TBBTaskExecutor::ReleaseResources()
{
	LOG_INFO(TEXT("%s"), "Enter");
	if ( NULL != m_pExecutorList )
	{
		m_pExecutorList = NULL;
	}	
	LOG_INFO(TEXT("%s"), "Leave");
}

void TBBTaskExecutor::Deactivate()
{
	OclAutoMutex mu(&m_mutex);
	LOG_INFO(TEXT("Enter count = %ld, SchedCount=%ld"), m_lActivateCount, (long)glTaskSchedCounter);
        
	long count = --m_lActivateCount;
	long sched_count = --glTaskSchedCounter;
	if ( 0 != count )
	{
		// Need to keep active instance
		LOG_INFO(TEXT("Leave count = %ld, ShedCounter=%ld"), count, sched_count);
		return;
	}
    ReleaseResources();    
	LOG_INFO(TEXT("%s"),"Shutting down...");
	LOG_INFO(TEXT("Leave count = %ld, SchedCount=%ld"), m_lActivateCount, (long)glTaskSchedCounter);
}

unsigned int TBBTaskExecutor::GetNumWorkingThreads() const
{
	return gWorker_threads;
}

SharedPtr<ITaskList> TBBTaskExecutor::CreateTaskList(CommandListCreationParam* param, void* pSubdevTaskExecData)
{
    bool       subdevice = param->isSubdevice;
    bool       OOO       = param->isOOO;

    ArenaHandler* const pDevArena = NULL != pSubdevTaskExecData ? (ArenaHandler*)pSubdevTaskExecData : m_pGlobalArenaHandler;
	SharedPtr<ITaskList> pList = NULL;
	if (OOO) 
	{
        pList = out_of_order_command_list::Allocate(subdevice, this, *pDevArena);
	}
	else
	{
        pList = in_order_command_list::Allocate(subdevice, this, *pDevArena);
	}

	return pList;
}

unsigned int TBBTaskExecutor::Execute(const SharedPtr<ITaskBase>& pTask, void* pSubdevTaskExecData)
{
    if (NULL == pSubdevTaskExecData)
    {
        m_pExecutorList->Enqueue(pTask);
	    m_pExecutorList->Flush();		
    }
    else
    {
        SubdevArenaHandler& arenaHandler = *(SubdevArenaHandler*)pSubdevTaskExecData;
        arenaHandler.GetInternalCommandList().Enqueue(pTask);
        arenaHandler.GetInternalCommandList().Flush();
    }
	return 0;
}

te_wait_result TBBTaskExecutor::WaitForCompletion(ITaskBase * pTask, void* pSubdevTaskExecData)
{
    if (NULL == pSubdevTaskExecData)
    {
        return m_pExecutorList->WaitForCompletion(pTask);
    }
    else
    {
        return ((SubdevArenaHandler*)pSubdevTaskExecData)->GetInternalCommandList().WaitForCompletion(pTask);
    }	
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
		STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb_preview_debug.dll");
#else
		STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb_preview.dll");
#endif

	bLoadRes = m_dllTBBLib.Load(tbbPath);
	if ( !bLoadRes )
	{
		LOG_ERROR(TEXT("Failed to load TBB from %s"), tbbPath);
	}
#endif

	return bLoadRes;
}

void* TBBTaskExecutor::CreateSubdevice(unsigned int uiNumSubdevComputeUnits, const unsigned int* pLegalCores, IAffinityChangeObserver& observer)
{
        return new SubdevArenaHandler(uiNumSubdevComputeUnits, gWorker_threads, *this, pLegalCores, observer);
}

void TBBTaskExecutor::ReleaseSubdevice(void* pSubdevData)
{
    delete (ArenaHandler*)pSubdevData;
}

void TBBTaskExecutor::WaitUntilEmpty(void* pSubdevData)
{
    if (NULL != pSubdevData)
    {
        ((ArenaHandler*)pSubdevData)->WaitUntilEmpty();
    }
    else
    {
        m_pGlobalArenaHandler->WaitUntilEmpty();
    }
}

void TBBTaskExecutor::SetWGContextPool(IWGContextPool* pWgContextPool)
{
    OclAutoMutex mu(&m_mutex);
    m_pWgContextPool = pWgContextPool;
}

WGContextBase* TBBTaskExecutor::GetWGContext(bool bBelongsToMasterThread)
{
    OclAutoMutex mu(&m_mutex);
    if (NULL != m_pWgContextPool)
    {
        return m_pWgContextPool->GetWGContext(bBelongsToMasterThread);
    }
    else
    {
        return NULL;
    }            
}

void TBBTaskExecutor::ReleaseWorkerWGContext(WGContextBase* wgContext)
{
    OclAutoMutex mu(&m_mutex);
    if (NULL != m_pWgContextPool)
    {
        m_pWgContextPool->ReleaseWorkerWGContext(wgContext);
    }            
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
