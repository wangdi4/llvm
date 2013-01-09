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

#ifdef DEVICE_NATIVE
    // no logger on discrete device
    #define LOG_ERROR(...)
    #define LOG_INFO(...)
    
    #define DECLARE_LOGGER_CLIENT
    #define INIT_LOGGER_CLIENT(...)
    #define RELEASE_LOGGER_CLIENT
#else
    #include "Logger.h"
#endif // DEVICE_NATIVE

#include <vector>
#include <cassert>
#ifdef WIN32
#include <stdafx.h>
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
#include "tbb_execution_schedulers.h"

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

        // fork execution
        TBB_ExecutionSchedulers::parallel_execute( cmdList, dim, dimCount, *pTask );
        // join execution

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
            m_list->EnqueueOOOFunc<ExecuteContainerBody>(functor);
            pTask = GetTask();
        }
        if (NULL != pTask && NULL != dynamic_cast<SyncTask*>(pTask.GetPtr()))
        {
            // synchronization point
            m_list->WaitForAllCommands();
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

void immediate_executor_task::operator()()
{    
	assert(m_list);
    assert(m_pTask);

    execute_command(m_pTask, *m_list);
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

	if (uiNumThreads != AUTO_THREADS)
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
    m_pGlobalArenaHandler = new RootDevArenaHandler(gWorker_threads, *this);

	LOG_INFO(TEXT("TBBTaskExecutor constructed to %d threads"), gWorker_threads);
	LOG_INFO(TEXT("TBBTaskExecutor initialized to %u threads"), uiNumThreads);
	LOG_INFO(TEXT("%s"),"Done");	
	return gWorker_threads;
}

void TBBTaskExecutor::Finalize()
{
    assert( 0 == m_lActivateCount );
    if (0 != m_lActivateCount)
    {
        LOG_ERROR(TEXT("%s"), "TBBTaskExecutor::Finalize called on still active Task Executor");
        return;
    }
    
    if (NULL != m_pGlobalArenaHandler)
    {
        delete m_pGlobalArenaHandler;
        m_pGlobalArenaHandler = NULL;
    }

    if (NULL != m_pScheduler)
    {
        delete m_pScheduler;
        m_pScheduler = NULL;
    }

    gWorker_threads = 0;
    m_pWgContextPool = NULL;
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

	m_pExecutorList = in_order_command_list::Allocate(this, *m_pGlobalArenaHandler); // TODO: Why not unordered
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

SharedPtr<ITaskList> TBBTaskExecutor::CreateTaskList(const CommandListCreationParam& param, void* pSubdevTaskExecData)
{
    ArenaHandler* const pDevArena = NULL != pSubdevTaskExecData ? (ArenaHandler*)pSubdevTaskExecData : m_pGlobalArenaHandler;
	SharedPtr<ITaskList> pList = NULL;

    assert( TE_CMD_LIST_PREFERRED_SCHEDULING_LAST > param.preferredScheduling );

    if ( param.preferredScheduling >= TE_CMD_LIST_PREFERRED_SCHEDULING_LAST)
    {
        LOG_ERROR(TEXT("Trying to create TaskExecutor Command list with unknown scheduler: %d"), (int)(param.preferredScheduling));
        return pList;
    }

    switch ( param.cmdListType )
    {
        case TE_CMD_LIST_IN_ORDER:
            pList = in_order_command_list::Allocate(this, *pDevArena, &param);
            break;

        case TE_CMD_LIST_OUT_OF_ORDER:
            pList = out_of_order_command_list::Allocate(this, *pDevArena, &param);
            break;

        case TE_CMD_LIST_IMMEDIATE:
            pList = immediate_command_list::Allocate(this, *pDevArena, &param);
            break;

        default:
            LOG_ERROR(TEXT("Trying to create TaskExecutor Command list with unknown type: %d"), (int)(param.cmdListType));
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
    if (uiNumSubdevComputeUnits < gWorker_threads)
    {
        return new SubdevArenaHandler(uiNumSubdevComputeUnits, gWorker_threads, *this, pLegalCores, observer);
    }
    else
    {
        return m_pGlobalArenaHandler;   // since m_pGlobalArenaHandler lives as long as TBBTaskExecutor, no reference counting is needed.
    }
}

void TBBTaskExecutor::ReleaseSubdevice(void* pSubdevData)
{
	if (pSubdevData != m_pGlobalArenaHandler)
	{
		delete (ArenaHandler*)pSubdevData;
	}
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
