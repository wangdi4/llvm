// Copyright (c) 2006-2013 Intel Corporation
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

#include <vector>
#include <cassert>
#ifdef WIN32
#include <stdafx.h>
#include <Windows.h>
#endif

#include <cl_sys_defines.h>
#include <cl_sys_info.h>
#include <cl_shutdown.h>
#include <tbb/blocked_range.h>
#include <tbb/atomic.h>
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#include <tbb/concurrent_queue.h>
#include <tbb/task.h>
#include <tbb/enumerable_thread_specific.h>
#include "cl_shared_ptr.hpp"
#include "task_group.hpp"
#include "tbb_execution_schedulers.h"
#include "cl_user_logger.h"
#include "base_command_list.hpp"

// no local atexit handler - only global
USE_SHUTDOWN_HANDLER(NULL);

using namespace Intel::OpenCL::Utils;

//#define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

#define MAX_BATCH_SIZE            128
#define SPARE_STATIC_DATA       8

namespace Intel { namespace OpenCL { namespace Utils {

FrameworkUserLogger* g_pUserLogger = NULL;

}}}

namespace Intel { namespace OpenCL { namespace TaskExecutor {

void RegisterReleaseSchedulerForMasterThread();

// TBB thread manager
INSTANTIATE_THREAD_MANAGER( TBB_PerActiveThreadData );

//! global TBB task scheduler objects
unsigned int gWorker_threads = 0;
AtomicCounter    glTaskSchedCounter;

void TE_RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn )
{
    Intel::OpenCL::Utils::RegisterGlobalAtExitNotification(fn);
}

bool execute_command(const SharedPtr<ITaskBase>& pCmd, base_command_list& cmdList)
{
    bool runNextCommand = true;
    bool cancel = cmdList.Is_canceled();

    if (cancel)
    {
        pCmd->Cancel();
    }
    else if ( pCmd->IsTaskSet() )
    {
        const SharedPtr<ITaskSet>& pTaskSet = pCmd.StaticCast<ITaskSet>();
        runNextCommand = TBB_ExecutionSchedulers::parallel_execute( cmdList, pTaskSet );
    }
    else
    {
        const SharedPtr<ITask>& pTask = pCmd.StaticCast<ITask>();
        runNextCommand = pTask->Execute();
    }

    runNextCommand &= !pCmd->CompleteAndCheckSyncPoint();
    return (cancel || runNextCommand);
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
        //First check if we need to stop iterating, next get next available record
        while( !(mustExit && m_list->m_bMasterRunning) && work->TryPop(currentTask))
        {
            if (NULL == currentTask->GetNDRangeChildrenTaskGroup())
            {
                // task enqueued by the host
                mustExit = !execute_command(currentTask, *m_list); //stop requested
            }
            else
            {
                // child task (GetNDRangeChildrenTaskGroup() returns the parent's TaskGroup)
                ExecuteContainerBody functor(currentTask, *m_list);
                TbbTaskGroup& tbbTskGrp = *static_cast<TbbTaskGroup*>(currentTask->GetNDRangeChildrenTaskGroup());
                tbbTskGrp.Run(functor);
            }
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

void out_of_order_executor_task::operator()()
{
    while (true)
    {
        /* We still have a proble in case the user calls clFinish from one thread and then enqueues a command from another thread - the second command will wait until the SyncTask of the 
            clFinish is completed, although it could be executed without waiting. However, this is a rare case and we won't handle it for now. */
        SharedPtr<ITaskBase> pTask = GetTask();
        while (0 != pTask && NULL == dynamic_cast<SyncTask*>(pTask.GetPtr()))
        {
            ExecuteContainerBody functor(pTask, *m_list);
            if (pTask->GetNDRangeChildrenTaskGroup() != NULL)
            {
                // this is a child task - GetNDRangeChildrenTaskGroup() returns its parent TaskGroup
                TbbTaskGroup& tbbTskGrp = *static_cast<TbbTaskGroup*>(pTask->GetNDRangeChildrenTaskGroup());
                tbbTskGrp.Run(functor);
            }
            else
            {
                // this is a parent task (enqueued by the host)
                m_list->ExecOOOFunc<ExecuteContainerBody>(functor);
            }
            pTask = GetTask();
        }
        if (0 != pTask && NULL != dynamic_cast<SyncTask*>(pTask.GetPtr()))
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

/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() :
    m_pScheduler(NULL),
    m_pLoggerClient(NULL)
{
    // we deliberately don't delete m_pScheduler (see comment above its definition)
}

TBBTaskExecutor::~TBBTaskExecutor()
{
    /* We don't delete m_pGlobalArenaHandler because of all kind of TBB issues in the shutdown sequence, but since this destructor is called when the whole library goes down and m_pGlobalArenaHandler
       is a singleton, it doesn't really matter. */
    // TBB seem to have a bug in ~task_scheduler_init(), so we work around it by not deleting m_pScheduler (TBB bug #1955)
}

int TBBTaskExecutor::Init(FrameworkUserLogger* pUserLogger, unsigned int uiNumOfThreads, ocl_gpa_data * pGPAData)
{    
    g_pUserLogger = pUserLogger;    
    INIT_LOGGER_CLIENT("TBBTaskExecutor", LL_INFO);
    LOG_INFO(TEXT("Initialization request with %d threads"), uiNumOfThreads);
    if ( 0 != gWorker_threads )
    {
        assert(0 && "TBBExecutor already initialized");
        LOG_ERROR(TEXT("Already initialized with %d threads"), gWorker_threads);;
        return gWorker_threads;
    }

    m_pGPAData = pGPAData;

    // Explicitly load TBB library
    if ( !LoadTBBLibrary() )
    {
        LOG_ERROR(TEXT("%s"), "Failed to load TBB library");
        return 0;
    }

    // Check TBB library version

    if(TBB_INTERFACE_VERSION > tbb::TBB_runtime_interface_version())
    {
        std::stringstream stream;
        stream << "TBB version doens't match. Required " << __TBB_STRING(TBB_INTERFACE_VERSION) << ", loaded " << tbb::TBB_runtime_interface_version() << "." << std::ends;
        LOG_ERROR(TEXT(stream.str().c_str()), "");
        if (NULL != pUserLogger && pUserLogger->IsErrorLoggingEnabled())
        {
            pUserLogger->PrintError(stream.str().c_str());
        }
        return 0;
    }

#ifdef __MIC__
    // On MIC enable 2MB pages for tbb allocator, if exists
    if(tbb::tbb_allocator<int>::allocator_type() == tbb::tbb_allocator<int>::scalable)
    {
        scalable_allocation_mode(USE_HUGE_PAGES, 1);
    }
#endif

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
    RELEASE_LOGGER_CLIENT;
}

SharedPtr<ITEDevice>
TBBTaskExecutor::CreateRootDevice( const RootDeviceCreationParam& device_desc, void* user_data,  ITaskExecutorObserver* my_observer )
{
    LOG_INFO(TEXT("Creating RootDevice with %d threads"), device_desc.uiThreadsPerLevel[0]);
    assert( (gWorker_threads > 0) && "TBB executor should be initialized first");

    RootDeviceCreationParam device( device_desc );

    if ((TE_AUTO_THREADS == device.uiThreadsPerLevel[0]) && (1 == device.uiNumOfLevels))
    {
        device.uiThreadsPerLevel[0] = gWorker_threads;
    }

    // check params
    if ((0 == device.uiNumOfLevels) || (device.uiNumOfLevels > TE_MAX_LEVELS_COUNT))
    {
        LOG_ERROR(TEXT("Wrong uiNumOfLevels parameter = %d, must be between 1 and %d"), device.uiNumOfLevels, TE_MAX_LEVELS_COUNT);
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
            return NULL;
        }
        overall_threads *= device.uiThreadsPerLevel[i];
    }

    if ((overall_threads == 0) || (overall_threads > gWorker_threads))
    {
        assert( false && "Too many threads requested - above maximum configured" );
        LOG_ERROR(TEXT("Wrong number of threads specified per level. Amount of threads on each level should be above 0, overall number not exceed %d"), gWorker_threads);
        return NULL;
    }

    // Create root device
    SharedPtr<TEDevice> root = TEDevice::Allocate( device, user_data, my_observer, *this );

    if (0 == root)
    {
        LOG_ERROR(TEXT("%s"), "Root device allocation failed"); // make gcc happy
    }
    else
    {
        LOG_INFO(TEXT("Root device created with %d threads"), overall_threads);
    }
    const CommandListCreationParam param(TE_CMD_LIST_IN_ORDER);
    m_pDebugInOrderDeviceQueue = in_order_command_list::Allocate(*this, root, param);
    return root;
}

void TBBTaskExecutor::CreateDebugDeviceQueue(const SharedPtr<ITEDevice>& rootDevice)
{
    const CommandListCreationParam param(TE_CMD_LIST_IN_ORDER);
    m_pDebugInOrderDeviceQueue = in_order_command_list::Allocate(*this, rootDevice.StaticCast<TEDevice>(), param, true);
}

void TBBTaskExecutor::DestroyDebugDeviceQueue()
{
    m_pDebugInOrderDeviceQueue = NULL;
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
#ifdef BUILD_EXPERIMENTAL_21
        STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb_debug_2_1.dll");
#else // BUILD_EXPERIMENTAL_21
        STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb_debug.dll");
#endif // BUILD_EXPERIMENTAL_21
#else
#ifdef BUILD_EXPERIMENTAL_21
        STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb_2_1.dll");
#else // BUILD_EXPERIMENTAL_21
        STRCAT_S(tbbPath, MAX_PATH, "tbb\\tbb.dll");
#endif // BUILD_EXPERIMENTAL_21
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

SharedPtr<IThreadLibTaskGroup> TBBTaskExecutor::CreateTaskGroup(const SharedPtr<ITEDevice>& device)
{ 
    return TbbTaskGroup::Allocate();
}

}}}//namespace Intel, namespace OpenCL, namespace TaskExecutor
