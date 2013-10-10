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

#pragma once

#include <cassert>
#include <vector>
#include <set>
#include <tbb/task_scheduler_observer.h>
#include <tbb/task_arena.h>
#include <harness_trapper.h>

#include "cl_device_api.h"
#include "cl_synch_objects.h"
#include "cl_shutdown.h"
#include "task_executor.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class TBBTaskExecutor;
class ArenaHandler;
class TEDevice;
class in_order_executor_task;
class out_of_order_command_list;
class base_command_list;
template<typename T> class command_list;

using Intel::OpenCL::Utils::IsShutdownMode;

struct TBB_PerActiveThreadData
{
public:
    TEDevice* device;
    void*     user_tls;
    unsigned  int  position[ TE_MAX_LEVELS_COUNT ];

    // arenas where thread attached starting from attach_level
    ArenaHandler*  attached_arenas[ TE_MAX_LEVELS_COUNT ]; 
    unsigned  int  attach_level;  // at what level thread attached to the device

    bool      enter_tried_to_report;
    bool      enter_reported;
    bool      is_master;
    
    const static unsigned int UNKNOWN_LEVEL = (unsigned int)(-1);
    TBB_PerActiveThreadData() { reset(); }
    void reset();
    void thread_attach() { reset(); };
};

/**
 * This class is responsible for handling the explicit arena and the per arena slot resources 
 */
class ArenaHandler : public tbb::task_scheduler_observer
{
public:
    
    ArenaHandler();
    void Init(unsigned int                  uiMaxNumThreads, 
              unsigned int                  uiReservedPlacesForMasters,
              unsigned int uiLevel, const unsigned int p_uiPosition[],
              TEDevice*                     device);

    /**
     * Destructor
     */
    virtual ~ArenaHandler() { Terminate(); StopMonitoring(); }

    /**
     * Enqueue a functor on the arena.
     * @param F the type of the functor
     * @param f the functor object
     */
    template <class F>
    void Enqueue(F& f) { m_arena.enqueue(f); }

    /**
     * Execute a functor on the arena.
     * @param F the type of the functor
     * @param f the functor object
     */
    template<class F>
    void Execute(F& f) { m_arena.execute(f); }

    /**
     * @return the number of compute units of the sub-device of this ArenaHandler
     */
    unsigned int GetMaxNumThreads() const { return m_uiMaxNumThreads; }
    unsigned int GetArenaLevel() const { return m_uiLevel; }
    const unsigned int* GetArenaPosition() const { return m_uiPosition; }

    unsigned int AllocateThreadPosition();
    void         FreeThreadPosition( unsigned int pos );

    // overriden task_scheduler_observer methods
    virtual void on_scheduler_entry(bool bIsWorker);
    virtual void on_scheduler_exit(bool bIsWorker);
    virtual bool on_scheduler_leaving();

private:
    // start observations
    void StartMonitoring() { observe(true); }
    void StopMonitoring()  { observe(false); }

    void Terminate() 
    {
        m_arena.terminate();
    }

    /**
     * The explicit arena of this device
     */
    tbb::task_arena     m_arena; 
    TEDevice*           m_device;

    unsigned int        m_uiMaxNumThreads;
    unsigned int        m_uiLevel;
    unsigned int        m_uiPosition[TE_MAX_LEVELS_COUNT];

    Intel::OpenCL::Utils::OclSpinMutex m_lock;
    std::vector<unsigned int> m_freePositions;

    // do not implement:
    ArenaHandler(const ArenaHandler&);
    ArenaHandler& operator=(const ArenaHandler&);    

    friend class DevArenaObserver;
    friend class TEDevice;
};

/**
 * This class represents an ArenaHandler for root devices
 */
class TEDevice : public ITEDevice
{
public:
    PREPARE_SHARED_PTR(TEDevice)

    static Intel::OpenCL::Utils::SharedPtr<TEDevice> Allocate(
                                        const RootDeviceCreationParam& device_desc, void* user_data, ITaskExecutorObserver* observer, 
                                        TBBTaskExecutor& taskExecutor, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& parent = NULL )
    {
        return new TEDevice(device_desc, user_data, observer, taskExecutor, parent );
    }

    /**
     * @param  uiNumSubdevComputeUnits - number of computing units in the sub-device. In the hiearachical mode it must be a subset of the level 0 units.
     * @return an object representing the sub-device in the TaskExecutor module
     * @param  user_handle - handle to be returned to used during GetCurrentDevice() calls
     */
    virtual Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateSubDevice( unsigned int uiNumSubdevComputeUnits, void* user_handle = NULL );

    /**
     * Reset ITaskExecutorObserver passed during device creation. Note: sub-devices share the same observer, so it will be reset for sub-devices also.
     * Reaset means no observer calls will be done after from this device and its sub-devices.
     */
    virtual void ResetObserver();


    // Set observer for the TEDevice
    virtual void SetObserver(ITaskExecutorObserver* pObserver);

    /**
     * Retrives concurrency level for the device
     * @return pointer to the new list or NULL on error
     */
    virtual int GetConcurrency() const;

#ifdef __HARD_TRAPPING__
    virtual bool AcquireWorkerThreads(int num_workers, int timeout);
    virtual void RelinquishWorkerThreads();
#endif // __HARD_TRAPPING__

    virtual void AttachMasterThread(void* user_tls);
    virtual void DetachMasterThread();

    /**
     * Create Task Execution List to the given sub-device
     * @return pointer to the new list or NULL on error
     */
    virtual Intel::OpenCL::Utils::SharedPtr<ITaskList> CreateTaskList(const CommandListCreationParam& param );

    /**
     * Wait until all work in a sub-device is complete and mark device as disabled. No more enqueues are allowed after the ShutDown
     */
    virtual void ShutDown();

    //
    //   Extra methods
    //

    /**
     * Enqueue a functor on the arena.
     * @param F the type of the functor
     * @param f the functor object
     */
    template <class F>
    void Enqueue(F& f);

    /**
     * Execute a functor on the arena.
     * @param F the type of the functor
     * @param f the functor object
     */
    template<class F>
    void Execute(F& f);

    /**
     * Lock/Unlock current state (working/shutting down)
     */
    void LockState() { m_stateLock.EnterRead(); }
    void UnLockState() { m_stateLock.LeaveRead(); }

    /**
     * @return whether there are enqueued tasks in the sub-device's command lists
     */
    bool AreEnqueuedTasks() const;

    /**
     * Whether this ArenaHandler is inside its destructor
     */
    bool isTerminating() const { return (m_state >= TERMINATING); }

    /**
     * Is master thread joining supported?
     */
    bool ShouldMasterJoinWork() const { return (TE_ENABLE_MASTERS_JOIN == m_deviceDescriptor.mastersJoining); }

    /**
     * Get number of threads in top level arena
     */
    unsigned int GetNumOfTopLevelThreads() const { return m_deviceDescriptor.uiThreadsPerLevel[0]  ; }

    /**
     * Get number of threads in top level arena
     */
    unsigned int GetNumOfLevels() const { return m_deviceDescriptor.uiNumOfLevels; }

    /**
     * Get user data associated with this device
     */
    void* GetUserData() const { return m_userData; }

    /**
     *  Owning task executor
     */
    TBBTaskExecutor&  GetTaskExecutor() { return m_taskExecutor; }

    // observer methods
    void on_scheduler_entry(bool bIsWorker, ArenaHandler& arena );
    void on_scheduler_exit(bool bIsWorker, ArenaHandler& arena );
    bool on_scheduler_leaving( ArenaHandler& arena );

    bool isSubDevice() const { return (NULL != m_pParentDevice.GetPtr()); }	

private:

    enum State 
    {
        INITIALIZING = 0,
        WORKING,
        TERMINATING,
        DISABLE_NEW_THREADS,
        SHUTTED_DOWN
    };

    Intel::OpenCL::Utils::OclReaderWriterLock   m_stateLock;
    volatile State                              m_state;

    RootDeviceCreationParam                     m_deviceDescriptor;
    TBBTaskExecutor&                            m_taskExecutor;
    void*                                       m_userData;

    Intel::OpenCL::Utils::OclReaderWriterLock   m_observerLock;
    ITaskExecutorObserver*                      m_observer;
    Intel::OpenCL::Utils::SharedPtr<TEDevice>   m_pParentDevice;

    ArenaHandler                                m_mainArena;
    ArenaHandler*                               m_lowLevelArenas[TE_MAX_LEVELS_COUNT-1]; // arrray or arrys of all levels except of 0

    Intel::OpenCL::Utils::AtomicCounter         m_numOfActiveThreads; 
    unsigned int                                m_maxNumOfActiveThreads;

    bool new_threads_disabled() const { return (m_state >= DISABLE_NEW_THREADS); }

    TEDevice( const RootDeviceCreationParam& device_desc, void* user_data, ITaskExecutorObserver* observer, 
              TBBTaskExecutor& taskExecutor, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& parent );

    ~TEDevice();

    // do not implement:
    TEDevice(const TEDevice&);
    TEDevice& operator=(const TEDevice&);       

    void init_next_arena_level( unsigned int current_level, unsigned int position[] );
    void free_thread_arenas_resources( TBB_PerActiveThreadData* tls, unsigned int starting_level );
    TEDevice* get_root() 
    {
        TEDevice* device = this;
        while (NULL != device->m_pParentDevice.GetPtr())
        {
            device = device->m_pParentDevice.GetPtr();
        }
        return device;
    }

#ifdef __HARD_TRAPPING__
    /**
     * Required for thread trapping inside the arena
     */
    Intel::OpenCL::Utils::AtomicPointer<tbb::Harness::TbbWorkersTrapper> m_worker_trapper;
#endif // __HARD_TRAPPING__
};

class TEDeviceStateAutoLock
{
public:
    TEDeviceStateAutoLock( TEDevice& device ) : m_device(device)
    {
        m_device.LockState();
    }
    ~TEDeviceStateAutoLock()
    {
        m_device.UnLockState();
    }

private:
    TEDevice& m_device;
};

// inlines
inline
void ArenaHandler::on_scheduler_entry(bool bIsWorker) 
{ 
    if (!IsShutdownMode())
    {
        m_device->on_scheduler_entry( bIsWorker, *this ); 
    }
}

inline
void ArenaHandler::on_scheduler_exit(bool bIsWorker) 
{ 
    if (!IsShutdownMode())
    {
        m_device->on_scheduler_exit( bIsWorker, *this ); 
    }
}

inline
bool ArenaHandler::on_scheduler_leaving() 
{ 
    return (IsShutdownMode()) ? true : m_device->on_scheduler_leaving( *this ); 
}

template <class F>
inline
void TEDevice::Enqueue(F& f)
{
    // ALERET!!! DK!!! Should we put here tasks also?
    if (1 == m_deviceDescriptor.uiNumOfLevels)
    {
        m_mainArena.Enqueue( f );
    }
    else
    {
        // ALERT!!! DK!!!
        // create hierarchical functor with f
        // m_mainArena.Enqueue( hierarchical_functor );
    }
}

template <class F>
inline
void TEDevice::Execute(F& f)
{
    // ALERET!!! DK!!! Should we put here tasks also?
    if (1 == m_deviceDescriptor.uiNumOfLevels)
    {
        m_mainArena.Execute( f );
    }
    else
    {
        // ALERT!!! DK!!!
        // create hierarchical functor with f
        // m_mainArena.Execute( hierarchical_functor );
    }
}

inline
void TBB_PerActiveThreadData::reset()
{
    device          = NULL;
    user_tls        = NULL;
    attach_level    = UNKNOWN_LEVEL;
    enter_tried_to_report = false;
    enter_reported  = false;
    is_master       = false;

    memset(attached_arenas, 0, sizeof(attached_arenas) );
}
}}}
