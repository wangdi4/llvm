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

#define MAX_BATCH_SIZE			128
#define SPARE_STATIC_DATA       8

namespace Intel { namespace OpenCL { namespace TaskExecutor {

void RegisterReleaseSchedulerForMasterThread();
// Logger
DECLARE_LOGGER_CLIENT;

// TBB thread manager
INSTANTIATE_THREAD_MANAGER( TBB_PerActiveThreadData );

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
	bool mustExit = false;

	while(true)
	{
    	SharedPtr<ITaskBase> currentTask;
		//First check if we need to stop interating, next get next available record
		while( !(mustExit && m_list->m_bMasterRunning) && work->TryPop(currentTask))
		{
			mustExit = !execute_command(currentTask, *m_list); //stop requested    
            currentTask = NULL;
		}

		if ( mustExit )
		{
			if (m_list->m_execTaskRequests.fetch_and_store(0) > 1)
			{
				m_list->InternalFlush(false);
			}
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
            m_list->ExecOOOFunc<ExecuteContainerBody>(functor);
            pTask = GetTask();
        }
        if (NULL != pTask && NULL != dynamic_cast<SyncTask*>(pTask.GetPtr()))
        {
            // synchronization point
            m_list->WaitForAllCommands();
            static_cast<SyncTask*>(pTask.GetPtr())->Execute();
			if (m_list->m_execTaskRequests.fetch_and_store(0) > 1)
			{
				m_list->InternalFlush(false);
			}
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
TBBTaskExecutor::TBBTaskExecutor()
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
    /* We don't delete m_pGlobalArenaHandler because of all kind of TBB issues in the shutdown sequence, but since this destructor is called when the whole library goes down and m_pGlobalArenaHandler
       is a singleton, it doesn't really matter. */
    // TBB seem to have a bug in ~task_scheduler_init(), so we work around it by not deleting m_pScheduler (TBB bug #1955)
	LOG_INFO(TEXT("%s"),"TBBTaskExecutor Destroyed");
	RELEASE_LOGGER_CLIENT;
}

int	TBBTaskExecutor::Init(unsigned int uiNumOfThreads, ocl_gpa_data * pGPAData)
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

	gWorker_threads = uiNumOfThreads;
	if (gWorker_threads == TE_AUTO_THREADS)
	{
		gWorker_threads = Intel::OpenCL::Utils::GetNumberOfProcessors();
	}

    m_pScheduler = new tbb::task_scheduler_init(tbb::task_scheduler_init::deferred);
    if (NULL == m_pScheduler)
    {
        LOG_ERROR(TEXT("%s"), "Failed to allocate task_scheduler_init");
        return 0;
    }
    m_pScheduler->initialize(gWorker_threads);
    m_threadManager.Init(gWorker_threads + SPARE_STATIC_DATA); // + SPARE to allow temporary oversubscription in flat mode and additional root devices

	LOG_INFO(TEXT("TBBTaskExecutor constructed to %d threads"), gWorker_threads);
	LOG_INFO(TEXT("TBBTaskExecutor initialized to %d threads"), uiNumOfThreads);
	LOG_INFO(TEXT("%s"),"Done");	
	return gWorker_threads;
}

void TBBTaskExecutor::Finalize()
{
    if (NULL != m_pScheduler)
    {
        delete m_pScheduler;
        m_pScheduler = NULL;
    }

    gWorker_threads = 0;
}

SharedPtr<ITEDevice>
TBBTaskExecutor::CreateRootDevice( const RootDeviceCreationParam& device_desc, void* user_data,  ITaskExecutorObserver* my_observer )
{
	LOG_INFO(TEXT("Enter%s"),"");
	assert(gWorker_threads && "TBB executor should be initialized first");

    RootDeviceCreationParam device( device_desc );

    if ((TE_AUTO_THREADS == device.uiThreadsPerLevel[0]) && (1 == device.uiNumOfLevels))
    {
        device.uiThreadsPerLevel[0] = gWorker_threads;
    }

    // check params
    if ((0 == device.uiNumOfLevels) || (device.uiNumOfLevels > TE_MAX_LEVELS_COUNT))
    {
    	LOG_ERROR(TEXT("Wrong uiNumOfLevels parameter = %d, must be between 1 and %d"), device.uiNumOfLevels, TE_MAX_LEVELS_COUNT);
    	LOG_INFO(TEXT("Leave%s"), "");
        return NULL;
    }

    // check for overall number of threads
    unsigned int overall_threads = 1;
    for (unsigned int i = 0; i < device.uiNumOfLevels; ++i)
    {
        unsigned int uiThreadsPerLevel =  device.uiThreadsPerLevel[i];
        if ((0 == uiThreadsPerLevel) || (TE_AUTO_THREADS == uiThreadsPerLevel))
        {
            assert( false && "Cannot specify 0 or TE_AUTO_THREADS threads per level" );
    	    LOG_ERROR(TEXT("Wrong number of threads per level: %u, must not be 0 or %u"), uiThreadsPerLevel, TE_AUTO_THREADS);
    	    LOG_INFO(TEXT("Leave%s"), "");
            return NULL;
        }
        overall_threads *= device.uiThreadsPerLevel[i];
    }

    if ((overall_threads == 0) || (overall_threads > gWorker_threads))
    {
        assert( false && "Too many threads requested - above maximum configured" );
    	LOG_ERROR(TEXT("Wrong number of threads specified per level. Amount of threads on each level should be above 0, overall number not exceed %d"), gWorker_threads);
    	LOG_INFO(TEXT("Leave%s"), "");
        return NULL;
    }

    // Create root device
    SharedPtr<TEDevice> root = TEDevice::Allocate( device, user_data, my_observer, *this );

    if (NULL == root)
    {
        LOG_ERROR(TEXT("Cannot allocate root device - exiting%s"), ""); // make gcc happy
    }

   	LOG_INFO(TEXT("Leave%s"), "");
	return root;
}

unsigned int TBBTaskExecutor::GetMaxNumOfConcurrentThreads() const
{
	return gWorker_threads;
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

ITaskExecutor::DeviceHandleStruct TBBTaskExecutor::GetCurrentDevice() const
{
    TBB_PerActiveThreadData* tls = m_threadManager.GetCurrentThreadDescriptor();
    
    if (NULL == tls)
    {
        return DeviceHandleStruct();
    }
    else
    {
        return DeviceHandleStruct( tls->device, (NULL != tls->device) ? tls->device->GetUserData() : NULL );
    }
}

bool TBBTaskExecutor::IsMaster() const
{
    TBB_PerActiveThreadData* tls = m_threadManager.GetCurrentThreadDescriptor();
    return ( (NULL != tls) ? tls->is_master : true);
}

unsigned int TBBTaskExecutor::GetPosition( unsigned int level ) const
{
    if (level > TE_MAX_LEVELS_COUNT)
    {
        assert( false && "Cannot return thread position for the level more then supported" );
        return TE_UNKNOWN;
    }

    TBB_PerActiveThreadData* tls = m_threadManager.GetCurrentThreadDescriptor();

    return (((NULL != tls) && (NULL != tls->device) && (level < tls->device->GetNumOfLevels())) ? tls->position[level] : TE_UNKNOWN);
}


}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
