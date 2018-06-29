// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <algorithm>
#include "arena_handler.h"
#include "tbb_executor.h"
#include "base_command_list.h"
#include "cl_sys_info.h"
#include "cl_shutdown.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_defines.h"
#include "hw_utils.h"

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

//
// ArenaHandler
//

ArenaHandler::ArenaHandler() :
    tbb::task_scheduler_observer(m_arena),
    m_device(nullptr), 
    m_uiMaxNumThreads(0), m_uiLevel(0)
{
    INIT_LOGGER_CLIENT("ArenaHandler", LL_INFO);
    memset( m_uiPosition, 0, sizeof(m_uiPosition) );
}

ArenaHandler::~ArenaHandler()
{
    RELEASE_LOGGER_CLIENT;
    Terminate();
    StopMonitoring();
}

void ArenaHandler::Init(
            unsigned int                  uiMaxNumThreads, 
            unsigned int                  uiReservedPlacesForMasters,
            unsigned int uiLevel, const unsigned int p_uiPosition[],
            TEDevice*                     device)
{
    LOG_INFO(TEXT("ArenaHandler::Init(uiMaxNumThreads=%d, uiReservedPlacesForMasters=%d, uiLevel=%d)"), uiMaxNumThreads, uiReservedPlacesForMasters, uiLevel);
    m_uiMaxNumThreads = uiMaxNumThreads;
    m_uiLevel         = uiLevel;
    m_device          = device;

    MEMCPY_S( m_uiPosition, sizeof(m_uiPosition), p_uiPosition, sizeof(m_uiPosition) ); // assume same size
    
    assert ( (uiReservedPlacesForMasters <= uiMaxNumThreads) && "Reserved slot for masters should not exceed the total amount of slots" );
    assert (uiReservedPlacesForMasters <= 1 && "currently only single reservation is allowed for master"); // until TBB will add default constructor for Arenas

    if (m_uiLevel > 0) // position allocations is not used in top level arenas
    {
        m_freePositions.resize( uiMaxNumThreads );
        for (unsigned int i = 0; i < uiMaxNumThreads; ++i)
        {
            // TODO: What does it mean? Why assigning always to 0
            m_freePositions[0] = i;
        }
    }

    LOG_INFO(TEXT("Calling to task_arena::initialize(%d, %d)"), uiMaxNumThreads, uiReservedPlacesForMasters);
    m_arena.initialize( uiMaxNumThreads, uiReservedPlacesForMasters );
    StartMonitoring();
}

unsigned int ArenaHandler::AllocateThreadPosition()
{
    if (0 == m_uiLevel)
    {
        // on the top level if thread exits - it exits the whole device, so no need in retaining slot position
        // ALERT!!! DK!!!
        // TBB now always allocate slot for master. TaskExecutor now never allows master to join, but number of slots 
        // may be more that size of device
        unsigned int position = tbb::task_arena::current_thread_index();
        if (position >= m_uiMaxNumThreads)
        {
            // wrap around
            assert( (position == m_uiMaxNumThreads) && "Assumption that current_thread_index() may return numberer between 0..slots_count is violated" );
            position = 0;
        }
        return position;
    }

    // if thread enters low level arena from upper level - we need to retain reported position regardless to tbb slot
    // so allocate the position and assign to thread. Even if this thread is not from the upper level but just worker that
    // joined arena, we cannot use tbb slot as this number may be already saved by some another upper level thread.
    OclAutoMutex lock(&m_lock);

    unsigned int pos = m_freePositions.back();
    m_freePositions.pop_back();
    return pos;
}

void ArenaHandler::FreeThreadPosition( unsigned int pos )
{
    if (0 != m_uiLevel)
    {
        OclAutoMutex lock(&m_lock);
        m_freePositions.push_back(pos);
    }
}

//
// TEDevice
//

void TEDevice::init_next_arena_level( unsigned int current_level, unsigned int position[] )
{
    unsigned int threads_per_level = m_deviceDescriptor.uiThreadsPerLevel[current_level];
    unsigned int arenas_per_level  = m_deviceDescriptor.uiThreadsPerLevel[current_level-1];

    ArenaHandler* next_level_array = new ArenaHandler[ arenas_per_level ];;
    m_lowLevelArenas[current_level-1] = next_level_array;

    for (unsigned int arena_idx = 0; arena_idx < arenas_per_level; ++arena_idx)
    {
        position[current_level-1] = arena_idx;   // this is position of current arena on the previous level
        next_level_array[arena_idx].Init( threads_per_level, 1, current_level, position, this );

        if (current_level < m_deviceDescriptor.uiNumOfLevels-1)
        {
            // recursion
            init_next_arena_level( current_level + 1, position );
        }
    }

    position[current_level-1] = 0;
}

TEDevice::TEDevice(  const RootDeviceCreationParam& device_desc, void* user_data, ITaskExecutorObserver* observer, 
                     TBBTaskExecutor& taskExecutor, const SharedPtr<TEDevice>& parent ) :
  m_state( INITIALIZING ),
  m_deviceDescriptor( device_desc ), m_taskExecutor( taskExecutor ), m_userData( user_data ), m_observer( observer ), 
  m_pParentDevice( parent ), m_maxNumOfActiveThreads(0)
{
    INIT_LOGGER_CLIENT("TEDevice", LL_INFO);
    LOG_INFO(TEXT("Initialized with levels=%d, %d threads on level 0"), m_deviceDescriptor.uiNumOfLevels, m_deviceDescriptor.uiThreadsPerLevel[0]);

    memset( m_lowLevelArenas, 0, sizeof(m_lowLevelArenas) );

    // calculate maximum number of threads
    m_maxNumOfActiveThreads = m_deviceDescriptor.uiThreadsPerLevel[0];
    for (unsigned int level = 1; level < m_deviceDescriptor.uiNumOfLevels; ++level)
    {
        m_maxNumOfActiveThreads *= m_deviceDescriptor.uiThreadsPerLevel[level];
    }

    // setup arenas

    // setup main arena
    unsigned int position[TE_MAX_LEVELS_COUNT];
    memset( position, 0, sizeof( position ) );

    if (m_deviceDescriptor.mastersJoining == TE_ENABLE_MASTERS_JOIN)
    {
        if (m_deviceDescriptor.uiNumOfExecPlacesForMasters > m_deviceDescriptor.uiThreadsPerLevel[0])
        {
            m_deviceDescriptor.uiNumOfExecPlacesForMasters = m_deviceDescriptor.uiThreadsPerLevel[0];
        }
    }
    else
    {
        m_deviceDescriptor.uiNumOfExecPlacesForMasters = 0;
    }

    LOG_INFO(TEXT("Initializing main arena with %d threads, reserved master slots %d, level = %d"),
        m_maxNumOfActiveThreads, m_deviceDescriptor.uiNumOfExecPlacesForMasters, 0);
    m_mainArena.Init( m_maxNumOfActiveThreads, m_deviceDescriptor.uiNumOfExecPlacesForMasters,
                      0, position, this );
    if ( m_deviceDescriptor.uiNumOfLevels > 1)
    {
        init_next_arena_level( 1, position );
    }

    m_state = WORKING;
}

TEDevice::~TEDevice()
{
    ShutDown();

    //  All threads that we allocated data for exited from arena, new threads will not call us anymore and
    //  noone is in the process of calling callbacks - delete
    for (unsigned int i = 0; i < m_deviceDescriptor.uiNumOfLevels-1; ++i)
    {
        delete [] m_lowLevelArenas[i];
        m_lowLevelArenas[i] = nullptr;
    }

    RELEASE_LOGGER_CLIENT;
}

void TEDevice::ShutDown()
{
    if (isTerminating())
    {
        return;
    }

    // must be in a block to release after block exit to avoid deadlock with threads counting down later in shut down
    {
        OclAutoWriter lock(&m_stateLock);

        if (isTerminating())
        {
            return;
        }
        
        m_state = TERMINATING;
    }

    // 1. Count down until all threads stopped
    if (!IsShutdownMode())
    {
        TBB_PerActiveThreadData* tls = m_taskExecutor.GetThreadManager().GetCurrentThreadDescriptor();
        int remainder = ((nullptr == tls) || (tls->device != this)) ? 0 : 1; // how many threads may remain inside arena
        
        while ((m_numOfActiveThreads > remainder) && (m_numOfActiveThreads <= (int)m_maxNumOfActiveThreads))
        {
            hw_pause();
        }

        // if current thread is inside TE Device - notify about exit manually
        if (0 != remainder)
        {
            assert( (nullptr != tls->attached_arenas[tls->attach_level]) && "NULL arena pointer at attach level" );
            on_scheduler_exit(!(tls->is_master), *(tls->attached_arenas[tls->attach_level]));
        }
    }

    // 2. Signal all now-entring threads that we are exiting
    m_state = DISABLE_NEW_THREADS;

    // 3. new threads may enter before disabling - wait all to exit
    if (!IsShutdownMode())
    {
        while ((m_numOfActiveThreads > 0) && (m_numOfActiveThreads <= (int)m_maxNumOfActiveThreads))
        {
            hw_pause();
        }
    }
    // now all threads that we allocated data for exited from TEDevice

    // 4. Stop all observers
    //    observer stopping blocks if any observer callback is in process
    if (!IsShutdownMode())
    {
        for (unsigned int i =  m_deviceDescriptor.uiNumOfLevels-1; i > 0  ; --i)
        {
            ArenaHandler* ar = m_lowLevelArenas[i-1];
            assert( (nullptr != ar) && "Low level arena array in NULL for hierarchical arenas?" );
            for (unsigned int j = 0; j < m_deviceDescriptor.uiThreadsPerLevel[ i ]; ++i)
            {
                ar[j].StopMonitoring();
            }
        }
        m_mainArena.StopMonitoring();
    }
    m_userData = nullptr; // to be on the safe side

    // 5. Signal all arenas to terminate
    for (unsigned int i =  m_deviceDescriptor.uiNumOfLevels-1; i > 0  ; --i)
    {
        ArenaHandler* ar = m_lowLevelArenas[i-1];
        assert( (nullptr != ar) && "Low level arena array in NULL for hierarchical arenas?" );
        for (unsigned int j = 0; j < m_deviceDescriptor.uiThreadsPerLevel[ i ]; ++i)
        {
            ar[j].Terminate();
        }
    }
    m_mainArena.Terminate();

    m_state = SHUTTED_DOWN;
    LOG_INFO(TEXT("%s"),"Shutdown completed");
}

bool TEDevice::AreEnqueuedTasks() const
{
    // Not used for now. When we use it, we'll implement it with a counter that is incremented whenever a task is enqueued.
    assert(false && "TEDevice::AreEnqueuedTasks() Not implemented");
    return false;
}

void TEDevice::free_thread_arenas_resources( TBB_PerActiveThreadData* tls, unsigned int starting_level )
{
    // free all allocations in the lower level arenas
    for (unsigned int i = starting_level; i < m_deviceDescriptor.uiNumOfLevels; ++i)
    {
        tls->attached_arenas[i]->FreeThreadPosition( tls->position[i] );
        tls->attached_arenas[i] = nullptr;
    }
}

void TEDevice::AttachMasterThread(void* user_tls)
{
    TBBTaskExecutor::ThreadManager& thread_manager = m_taskExecutor.GetThreadManager();
    TBB_PerActiveThreadData* tls = thread_manager.RegisterAndGetCurrentThreadDescriptor();

    assert( (nullptr != tls) && "TBB Thread Manager was not able to find free entry" );
    
    tls->device = this;
    tls->attach_level = 0;
    tls->is_master = true;
    
    // arena was not used yet - allocate position inside arena
    tls->position[0]         = m_mainArena.AllocateThreadPosition();
    assert(0==tls->position[0] && "Currently master should be allocated on slot 0");
    
    tls->attached_arenas[0]  = &m_mainArena;
    
    // report entry to user - need be done only at the lowest level
    tls->enter_tried_to_report = true;

    // per thread user data recides inside per-thread descriptor
    tls->user_tls = user_tls;
    tls->enter_reported = true;
}

void TEDevice::DetachMasterThread()
{
    TBBTaskExecutor::ThreadManager& thread_manager = m_taskExecutor.GetThreadManager();
    TBB_PerActiveThreadData* tls = thread_manager.RegisterAndGetCurrentThreadDescriptor();

    assert(tls && "You should attach thread at first");
    if(tls)
        tls->reset();

    thread_manager.UnregisterCurrentThread();
}

void TEDevice::on_scheduler_entry( bool bIsWorker, ArenaHandler& arena )
{
    TBBTaskExecutor::ThreadManager& thread_manager = m_taskExecutor.GetThreadManager();
    TBB_PerActiveThreadData* tls = thread_manager.RegisterAndGetCurrentThreadDescriptor();

    assert( (nullptr != tls) && "TBB Thread Manager was not able to find free entry" );

    // case when for some reason we were unable to monitor exit from some device
    if ((nullptr != tls->device) && (this != tls->device))
    {
        // we discovered such things in some tests (task_executor_test).
        // assume device is not-existent already, otherwise we would catch its exit
        tls->reset();
    }

    unsigned int curr_arena_level = arena.GetArenaLevel();

    if (nullptr == tls->device)
    {
        // Yyyyes - thread is attaching to device
        if (new_threads_disabled())
        {
            // we are inside destructor after threads count reached 0 - disable new threads
            thread_manager.UnregisterCurrentThread();
            return;
        }
        
        ++m_numOfActiveThreads;
        tls->device = this;
        tls->attach_level = curr_arena_level;

        // master thread is only thread that is master on the top level
        if (curr_arena_level > 0)
        {
            errno_t error;
            error = MEMCPY_S( tls->position, sizeof(tls->position), arena.GetArenaPosition(), sizeof(unsigned  int)*m_deviceDescriptor.uiNumOfLevels );
            assert( (0 == error) && "Too many levels to copy to position?" );
            tls->is_master = false;
        }
        else
        {
            tls->is_master = (!bIsWorker);
        }
    }

    // now set current thread position inside current level
    ArenaHandler*& prev_arena = tls->attached_arenas[curr_arena_level];

    // in general current arena should be either not used at all or already used and have allocated position
    if ((nullptr != prev_arena) && (&arena != prev_arena))
    {
        // Arena was used but not this... Intermediate thread moved to another sub-arena?
        // This may be the case if task tries to join the top level arena during execution in the low level arena
        // free all allocations in the lower level arenas
        assert( false && "Looks like task tries to join top level arena while executing inside low level arena" );
        free_thread_arenas_resources( tls, curr_arena_level );
    }

    if (nullptr == prev_arena)
    {
        // arena was not used yet - allocate position inside arena
        tls->position[curr_arena_level]         = arena.AllocateThreadPosition();
        tls->attached_arenas[curr_arena_level]  = &arena;
    }

    // report entry to user - need be done only at the lowest level
    if ((!tls->enter_tried_to_report) && (curr_arena_level == (m_deviceDescriptor.uiNumOfLevels-1)))
    {
        tls->enter_tried_to_report = true;

        /* If the user enqueues a command and then exits the program without having waited for the command to finish, we might get here while things are being folded beneath our feet. We have no way to
           protect ourselves, except by raising a flag in a function registered by atexit. This isn't a perfect protection, since exit can be called while the worker thread is in this method after having
           already checked the flag, but it significantly reduces the probability for it. This also applies to the same flag checking in other methods. */
        {
            if ((nullptr != m_observer) && (!IsShutdownMode()))
            {
                // per thread user data recides inside per-thread descriptor
                tls->user_tls = m_observer->OnThreadEntry();
                tls->enter_reported = true;
            }
        }
    }
}

void TEDevice::on_scheduler_exit( bool bIsWorker, ArenaHandler& arena )
{
    TBBTaskExecutor::ThreadManager& thread_manager = m_taskExecutor.GetThreadManager();
    TBB_PerActiveThreadData* tls = thread_manager.GetCurrentThreadDescriptor();

    if ((nullptr == tls) && new_threads_disabled())
    {
        // thread entered after disabling
        return;
    }

    assert( (nullptr != tls) && "TBB Thread Manager lost entry?" );
    assert( (nullptr != tls->device) && "Something wrong with observers - thread exits without enter" );
    assert( (this == tls->device) && "Something wrong with observers - thread exits the wrong device" );

    unsigned int curr_arena_level = arena.GetArenaLevel();

    if (curr_arena_level > tls->attach_level )
    {
        // we are leaving low level arena but still inside device - preserve all resources and do not report
        return;
    }

    if (curr_arena_level < tls->attach_level)
    {
        // oops - we are leaving arena above attach level?
        assert( false && "oops - we are leaving arena above attach level?" );
        free_thread_arenas_resources( tls, tls->attach_level );
        tls->reset();
        thread_manager.UnregisterCurrentThread();
        if (--m_numOfActiveThreads < 0)
        {
            assert( false && "Thread exits device while device already does not contain running threads while leaving above attach level" );
            ++m_numOfActiveThreads;
        }
        return;
    }

    // now the only case - we are leaving our attach_level - out from device
    if (tls->enter_reported)
    {
        if ((nullptr != m_observer) && (!IsShutdownMode()))
        {
            // per thread user data recides inside per-thread descriptor
            m_observer->OnThreadExit( tls->user_tls );
        }
    }

    free_thread_arenas_resources( tls, tls->attach_level );
    tls->reset();
    thread_manager.UnregisterCurrentThread();
    if (--m_numOfActiveThreads < 0)
    {
        assert( false && "Thread exits device while device already does not contain running threads while leaving normally" );
        ++m_numOfActiveThreads;
    }
}

bool TEDevice::on_scheduler_leaving( ArenaHandler& arena )
{
    if (isTerminating())
    {
        return true; // always allow leaving during shutdown
    }

    TE_BOOLEAN_ANSWER user_answer = TE_USE_DEFAULT;

    TBBTaskExecutor::ThreadManager& thread_manager = m_taskExecutor.GetThreadManager();
    TBB_PerActiveThreadData* tls = thread_manager.GetCurrentThreadDescriptor();

    if ((nullptr != tls) && (nullptr != tls->device) && (tls->enter_reported))
    {
        assert( (this == tls->device) && "Something wrong with observers - thread tries to leave the wrong device" );

        if ((nullptr != m_observer) && (!IsShutdownMode()))
        {
            user_answer = m_observer->MayThreadLeaveDevice( &(tls->user_tls) );
        }
    }

    bool may_leave = true;
    if (TE_USE_DEFAULT == user_answer)
    {
        // We return true, because of TBB bug #1967 and because just returning true instead of going over all the command lists actually gives 3% speedup!
        may_leave = true;
    }
    else
    {
        may_leave = (TE_YES == user_answer);
    }

    return may_leave;
}
void TEDevice::ResetObserver()
{ 
    m_observer = nullptr;
}
void TEDevice::SetObserver(ITaskExecutorObserver* pObserver)
{
        if ( (nullptr==pObserver) && (nullptr != m_observer) )
        {
            m_mainArena.observe(false);
            m_numOfActiveThreads = 0;
            m_observer = nullptr;
            return;
        }

           m_observer = pObserver;

    // Looks a raise in this place, but since it's relevant only for MIC we don't have an issue
    m_mainArena.observe(true);
}


SharedPtr<ITEDevice> TEDevice::CreateSubDevice( unsigned int uiNumSubdevComputeUnits, void* user_data ) 
{
    LOG_INFO(TEXT("Creating sub-device with %d compute units"), uiNumSubdevComputeUnits);

    RootDeviceCreationParam device_desc( m_deviceDescriptor );

    if ((uiNumSubdevComputeUnits < device_desc.uiThreadsPerLevel[0]) && (uiNumSubdevComputeUnits > 0))
    {
        device_desc.uiThreadsPerLevel[0] = uiNumSubdevComputeUnits;
    }

    device_desc.mastersJoining              = TE_DISABLE_MASTERS_JOIN;
    device_desc.uiNumOfExecPlacesForMasters = 0;

    return Allocate( device_desc, user_data, m_observer, m_taskExecutor, this );
}

SharedPtr<ITaskList> TEDevice::CreateTaskList(const CommandListCreationParam& param)
{
    SharedPtr<ITaskList> pList = nullptr;

    assert( (TE_CMD_LIST_PREFERRED_SCHEDULING_LAST > param.preferredScheduling) && "Trying to create TaskExecutor Command list with unknown scheduler" );

    if ( param.preferredScheduling >= TE_CMD_LIST_PREFERRED_SCHEDULING_LAST)
    {
        return pList;
    }

    switch ( param.cmdListType )
    {
        case TE_CMD_LIST_IN_ORDER:
            pList = in_order_command_list::Allocate(m_taskExecutor, this, param);
            break;

        case TE_CMD_LIST_OUT_OF_ORDER:
            pList = out_of_order_command_list::Allocate(m_taskExecutor, this, param);
            break;

        case TE_CMD_LIST_IMMEDIATE:
            pList = immediate_command_list::Allocate(m_taskExecutor, this, param);
            break;

        default:
            assert( false && "Trying to create TaskExecutor Command list with unknown type");
    }

    return pList;
}

/**
    * Retrives concurrency level for the device
    * @return pointer to the new list or NULL on error
    */
int TEDevice::GetConcurrency() const
{
    assert ( (1 == m_deviceDescriptor.uiNumOfLevels)  && "Currently only single level devices are supported");

    return m_deviceDescriptor.uiThreadsPerLevel[0];
}

bool TEDevice::IsCurrentThreadInArena() const
{
    TBB_PerActiveThreadData* tls = GetTaskExecutor().GetThreadManager().GetCurrentThreadDescriptor();
    return !(nullptr == tls || nullptr == tls->device || this != tls->device);
}

#ifdef __HARD_TRAPPING__

struct TrapperRunner
{
protected:
    tbb::Harness::TbbWorkersTrapper& my_trapper;
public:
    TrapperRunner(tbb::Harness::TbbWorkersTrapper& trapper) : my_trapper(trapper) {};

    void operator()(void)
    {
        my_trapper();
    }
};

bool TEDevice::AcquireWorkerThreads(int num_workers, int timeout)
{
    LOG_INFO(TEXT("Acquiring %d threads, timeout %d"), num_workers, timeout);

    if ( -1 == num_workers )
    {
        num_workers = GetConcurrency();
    } else if ( 0 == num_workers )
    {
      num_workers = m_numOfActiveThreads;
    }

    tbb::Harness::TbbWorkersTrapper* new_trapper = new tbb::Harness::TbbWorkersTrapper(num_workers, true);
    tbb::Harness::TbbWorkersTrapper* old_trapper = m_worker_trapper.test_and_set(nullptr, new_trapper);
    assert( nullptr == old_trapper && "Another trapper already exists");
    if ( nullptr != old_trapper )
    {
        // Another trapper is already running, delete current and continue execution
        delete new_trapper;
        return false;
    }

    // Execute trapper
    TrapperRunner runner(*new_trapper);
    m_mainArena.Enqueue(runner);
    new_trapper->Wait();

    LOG_INFO(TEXT("Acquired %d threads"), new_trapper->GetTrappedThreadCount());
    return true;
}

void TEDevice::RelinquishWorkerThreads()
{
    tbb::Harness::TbbWorkersTrapper* trapper = m_worker_trapper.exchange(nullptr);
    assert (nullptr!=trapper && "Trying to relinquish from NULL trapper");
    if ( nullptr == trapper )
      return;

    delete trapper;
    LOG_INFO(TEXT("%s"), "Threads Relinquished");
}
#endif // __HARD_TRAPPING__

