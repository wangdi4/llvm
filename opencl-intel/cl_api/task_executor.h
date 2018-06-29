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

/*
*
* File task_executor.h
* Declares an interface for task execution model
* The task model should satisfy next guidelines:
*    a.  The task creation order is unknown. Task execution will be handled by list.
*        Lists are of two types, ordered - wherein the tasks shall be executed by the given order
*        un-ordered - wherein tasks are executed without decencies
*    b.  Async. execution. The execution function will be returned immediately,
*        the provided callback function will be called when a task is completed
*    c.  It'll be two types of tasks:
*        I.  Simple - single function task, single instance of function will be executed
*        II. Complex (Task Set) - the main function will be executed multiple times
*            This complex function will have initialization and finalization stages
*            single function that should be called before and after the main loop.
*
*/

#pragma once

#include <stddef.h>
#include <cl_sys_defines.h>
#include "cl_shared_ptr.h"
#include "cl_device_api.h"

#ifndef DEVICE_NATIVE
#include "ocl_itt.h"
#endif

#if defined (_WIN32)
#ifdef TASK_EXECUTOR_EXPORTS
#define TASK_EXECUTOR_API    __declspec(dllexport)
#else
#define TASK_EXECUTOR_API    __declspec(dllimport)
#endif
#else
#define TASK_EXECUTOR_API
#endif

// Forward declaration
struct ocl_gpa_data;

namespace Intel { namespace OpenCL { namespace Utils {
    class IAtExitCentralPoint;
    class FrameworkUserLogger;
}}}

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class ITaskExecutor;

// The following enum is used for defining task priority
typedef enum
{
    TASK_PRIORITY_LOW = 0,
    TASK_PRIORITY_MEDIUM_LOW,
    TASK_PRIORITY_MEDIUM,
    TASK_PRIORITY_MEDIUM_HIGH,
    TASK_PRIORITY_HIGH
} TASK_PRIORITY;

typedef enum
{
    FINISH_COMPLETED = 0,
    FINISH_INIT_FAILED,
    FINISH_EXECUTION_FAILED
} FINISH_REASON;

// The following result define WaitForCompletion states
typedef enum 
{
    TE_WAIT_COMPLETED                = 0,// All processing tasks were completed
    TE_WAIT_MASTER_THREAD_BLOCKING,      // WaitForCompletion was blocked by another master thread
    TE_WAIT_NOT_SUPPORTED                // Wait for completion doesn't supported
} te_wait_result;

// The following enum defines CommandList type
typedef enum 
{
    TE_CMD_LIST_IN_ORDER            = 0,// Process tasks in order of enqueing
    TE_CMD_LIST_OUT_OF_ORDER,           // Process tasks in any order
    TE_CMD_LIST_IMMEDIATE               // Process each task immediately using the caller thread also
} TE_CMD_LIST_TYPE;

// preferred CommandList scheduling type
typedef enum  
{
    TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC = 0,             // in TBB case - use auto_partitioner for TaskSets
    TE_CMD_LIST_PREFERRED_SCHEDULING_PRESERVE_TASK_AFFINITY,  // try to preserve task affinities to threads in TaskSet
    TE_CMD_LIST_PREFERED_SCHEDULING_UNEVEN_OPENCL,            // Use OpenCL specific partitioner
    
    TE_CMD_LIST_PREFERRED_SCHEDULING_LAST
} TE_CMD_LIST_PREFERRED_SCHEDULING;

// TaskSets optimizations
typedef enum 
{
    TASK_SET_OPTIMIZE_DEFAULT = 0,      // in TBB case - use TBB internal by-tile approach in 2D/3D cases
    TASK_SET_OPTIMIZE_BY_ROW,           // Optimize 2D/3D cases by row
    TASK_SET_OPTIMIZE_BY_COLUMN,        // Optimize 2D/3D cases by column
    TASK_SET_OPTIMIZE_BY_TILE,          // Optimize 2D/3D cases by tile

    TASK_SET_OPTIMIZE_BY_LAST
} TASK_SET_OPTIMIZATION;

// Boolean answers with default
typedef enum
{
    TE_NO           = 0,
    TE_YES          = 1,
    TE_USE_DEFAULT  = 2
} TE_BOOLEAN_ANSWER;

// Command List Creation params
struct CommandListCreationParam
{
    TE_CMD_LIST_TYPE                    cmdListType;
    TE_CMD_LIST_PREFERRED_SCHEDULING    preferredScheduling;

    CommandListCreationParam( TE_CMD_LIST_TYPE type, 
                              TE_CMD_LIST_PREFERRED_SCHEDULING sched = TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC, bool bIsProfilingEnabled = false, bool bIsDefaultQueue = false) :
        cmdListType(type), preferredScheduling(sched), isProfilingEnabled(bIsProfilingEnabled), isQueueDefault(bIsDefaultQueue) {}
    bool  isProfilingEnabled;
    bool  isQueueDefault;
};

// Root device Creation params

// Init task executor to use uiNumThreads for execution
// if uiNumThreads == AUTO_THREADS, number of threads will be defined by implementation
static const unsigned int TE_AUTO_THREADS = (unsigned int)(-1);
static const unsigned int TE_MAX_LEVELS_COUNT = 2;
static const unsigned int TE_UNKNOWN      = (unsigned int)(-1);
enum TE_MASTERS_JOINING
{
    TE_ENABLE_MASTERS_JOIN,         // enable  master threads join execution in specific Root Device
    TE_DISABLE_MASTERS_JOIN         // disable master threads join execution in specific Root Device
};

struct RootDeviceCreationParam
{
    unsigned int        uiNumOfLevels;
    unsigned int        uiThreadsPerLevel[TE_MAX_LEVELS_COUNT];
    TE_MASTERS_JOINING  mastersJoining;
    unsigned int        uiNumOfExecPlacesForMasters; // if TE_ENABLE_MASTERS_JOIN, how many cores in device should be reserved for masters only.
                                                     // Only 0 or 1 is supported now and cannot exceed number of threads on the top level

    /**
     * Create Root Device in hierarchical mode - threads are splitted into levels. 
     * Each thread belongs to all levels where the top level is 0. Use this to express HW structure.
     * TE_AUTO_THREADS cannot be used if uiNumOfLevels > 1.
     * @param levels            - number of levels in Root Device. Must be >0 and <TE_MAX_LEVELS_COUNT
     * @param threadsPerLevel   - pointer to array with number of threads per each level. Must be > 0 and overall number is limited to GetMaxNumOfConcurrentThreads().
     * @param joining           - can user threads join the execution in this device?
     * @param reservedForMasters- if joining is enabled, how many places should be reserved in the top level device for masters. Extra joining masters will either replace
     *                            workers or will not allow joining
     */
    RootDeviceCreationParam( unsigned int levels, const unsigned int threadsPerLevel[], 
                             TE_MASTERS_JOINING joining = TE_ENABLE_MASTERS_JOIN, unsigned int reservedForMasters = 0 ) :
        uiNumOfLevels( levels ), mastersJoining( joining ), uiNumOfExecPlacesForMasters( reservedForMasters )
    {
        if (nullptr != threadsPerLevel)
        {
            MEMCPY_S( uiThreadsPerLevel, sizeof(uiThreadsPerLevel), threadsPerLevel, sizeof(unsigned int)*((levels < TE_MAX_LEVELS_COUNT) ? levels : TE_MAX_LEVELS_COUNT ));
        }
    }

    /**
     * Create Root Device in flat mode - threads are not splitted into levels. This means that all threads are at level 0 only.
     * @param overallThreads    - number of threads in the root device or TE_AUTO_THREADS.
     * @param joining           - can user threads join the execution in this device?
     * @param reservedForMasters- if joining is enabled, how many places should be reserved in the top level device for masters. Extra joining masters will either replace
     *                            workers or will not allow joining
     */
    RootDeviceCreationParam( unsigned int overallThreads = TE_AUTO_THREADS,  
                             TE_MASTERS_JOINING joining  = TE_ENABLE_MASTERS_JOIN, unsigned int reservedForMasters = 0 ) :
        uiNumOfLevels( 1 ), mastersJoining( joining ), uiNumOfExecPlacesForMasters( reservedForMasters )
    {
        uiThreadsPerLevel[0] = overallThreads;
    }

};

/**
 * This class represents a task group that is implemented using a native task group class of the threading library (in TBB, it is tbb::task_group).
 * It is required because in TBB our implementation of class ITaskGroup is required for some scenarios, but not appropriate for others.
 */
class IThreadLibTaskGroup : public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(IThreadLibTaskGroup)

    /**
     * Status of the IThreadLibTaskGroup
     */
    enum TaskGroupStatus
    {
        NOT_COMPLETE, // Not cancelled and not all tasks in group have completed. 
        COMPLETE,     // Not cancelled and all tasks in group have completed
        CANCELED      // Task group received cancellation request
    };

    /**
     * Wait until all functors have been run
     * @return the status of this IThreadLibTaskGroup after the wait has been completed
     */
    virtual TaskGroupStatus Wait() = 0;
};

/**
 * This class represents a group of tasks
 */
class ITaskGroup : public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(ITaskGroup)

    /**
     * Wait for the completion of all tasks in the group
     */
    virtual void WaitForAll() = 0;

};

/////////////////////////////////////////////////////////////////////////////
// ITaskBase interface - defines a basic set of functions
class ITaskBase : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(ITaskBase)

    // Returns whether the executed task is a task set.
    virtual bool    IsTaskSet() const = 0;

    // Return task priority, currently the implementation shall return TASK_PRIORITY_MEDIUM
    virtual TASK_PRIORITY    GetPriority() const = 0; 

    // Returns true in case current task is a syncronization point
    // No more tasks will be executed in this case
    virtual bool    CompleteAndCheckSyncPoint() = 0;
    
    // Set current command as syncronization point
    // Returns true if command is already completed
    virtual bool    SetAsSyncPoint() = 0;

    // Returns true if command is already completed
    virtual bool    IsCompleted() const = 0;

    // Task execution routine called instead of actual execution if CommandList is canceled
    virtual void    Cancel() = 0;

    // Releases task object, shall be called instead of delete operator.
    virtual long    Release() = 0;

    // For NDRange commands return the ITaskGroup used to group its children; for other commands return NULL
    virtual IThreadLibTaskGroup* GetNDRangeChildrenTaskGroup() = 0;
};

/////////////////////////////////////////////////////////////////////////////
// ITask interface - defines a basic set of functions
class ITask : public ITaskBase
{
public:

    PREPARE_SHARED_PTR(ITask)

    bool    IsTaskSet() const {return false;}
    // Task execution routine, will be called by task executor
    // return false when task execution fails
    virtual bool    Execute() = 0;

    // Task execution routine, will be called by task executor instead of Execute() if CommandList is canceled
    // virtual void    Cancel() = 0;

};

/////////////////////////////////////////////////////////////////////////////
// ITaskSet interface - defines a function set for execution of complex tasks
class ITaskSet: public ITaskBase
{
public:
    PREPARE_SHARED_PTR(ITaskSet)

    bool    IsTaskSet() const {return true;}
    // Initialization function. This functions is called before the "main loop"
    // Generally initializes internal data structures
    // Fills the buffer with 3D number of iterations to run
    // Fills regCount with actual number of regions
    // Returns 0 if initialization success, otherwise an error code
    virtual int     Init(size_t region[], unsigned int& regCount, size_t numberOfThreads = 1) = 0;

    virtual void*   GetConcreteClass() { return this; }

    // Is called when the task is going to be called for the first time within specific thread. 
    // @param currentThreadData - data returned by OnThreadEntry()
    // Returns data to be passed to ExecuteIteration methods, if attach process succeeded, otherwise NULL to abort
    virtual void*   AttachToThread(void* currentThreadData, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) = 0;

    // Is called when the task will not be executed by the specific thread    
    // Receives data returned by AttachToThread.
    virtual void    DetachFromThread(void* data_from_AttachToThread) = 0;

    // "Main loop"
    // The function is called with different 'inx' parameters for each iteration number
    // Return false to abort 
    virtual bool    ExecuteIteration(size_t x, size_t y, size_t z, void* data_from_AttachToThread) = 0;

    // Final stage, free execution resources
    // Return false when command execution fails
    virtual bool    Finish(FINISH_REASON reason) = 0;

    // Task execution routine, will be called by task executor instead of Init() if CommandList is canceled. If Init() was already called,
    // Cancel() is not called - normal processing is continued
    // virtual void    Cancel() = 0;

    // Optimize By
    virtual TASK_SET_OPTIMIZATION OptimizeBy()                        const = 0;
    virtual size_t PreferredSequentialItemsPerThread() const = 0;

};

/////////////////////////////////////////////////////////////////////////////
// ITaskList interface - defines a function set for task list handling

class ITEDevice;

class ITaskList : public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(ITaskList)

    // Enqueue a given task for execution, the function is asynchronous and exits immediately in in-order and out-of-order lists.
    // In immediate list the task is enqueued, flushed and executes immediately. Functions returns only after the task completes.
    // Returns actual number of enqueued tasks
    // Task execution may be started immediately or postponed till Flush() command
    virtual unsigned int    Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask) = 0; // Dynamically detect Task or TaskSet

    // Make all outstanding and future tasks cancel instead of execute.
    virtual void            Cancel() = 0;

    // Ensures that all task were send to execution, non-blocking function
    virtual bool            Flush() = 0;

    // Immediately spawn a task without enqueuing it first in order to save the lock on the queue.
    virtual void            Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask, IThreadLibTaskGroup& taskGroup) = 0;
    // whether this ITaskList supports device-side enqueuing of commands
    virtual bool            DoesSupportDeviceSideCommandEnqueue() const = 0;

    // Add the calling thread to execution pool
    // Function blocks, until the pTask is completed or in case of NULL
    // all tasks belonging to the list are completed.
    // Not supported for immediate lists
    virtual te_wait_result  WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait) = 0;


    // Returns true if master thread joined for execution on this queue
    virtual bool            IsMasterJoined() const = 0;

    // Returns true if master thread can join execution
    virtual bool            CanMasterJoin() const = 0;

    // Returns whether profiling is enabled for this ITaskList
    virtual bool            IsProfilingEnabled() const = 0;

    // Return maximum concurency from device assosiated with this list
    virtual int             GetDeviceConcurency() const = 0;

    // Returns the ITEDevice of this ITaskList
    virtual Intel::OpenCL::Utils::SharedPtr<ITEDevice>       GetDevice() = 0;
    virtual Intel::OpenCL::Utils::ConstSharedPtr<ITEDevice>  GetDevice() const = 0;

    // Returns an ITaskGroup for NDRange commands to use to wait for their children
    virtual Intel::OpenCL::Utils::SharedPtr<IThreadLibTaskGroup> GetNDRangeChildrenTaskGroup() = 0;

    // Returns an optional in-order ITaskList that can be used to enqueue child kernels in debug mode
    virtual ITaskList* GetDebugInOrderDeviceQueue() { return nullptr; }
};

class ITaskExecutorObserver;

/////////////////////////////////////////////////////////////////////////////
// ITEDevice interface - defines a function set for TaskExecutor Device and SubDevice paradigm
class ITEDevice : public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(ITEDevice)

    /**
     * @param  uiNumSubdevComputeUnits - number of computing units in the sub-device. In the hierarchical mode it must be a subset of the level 0 units.
     * @return an object representing the sub-device in the TaskExecutor module
     * @param  user_handle - handle to be returned to used during GetCurrentDevice() calls
     */
    virtual Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateSubDevice( unsigned int uiNumSubdevComputeUnits, void* user_handle = nullptr ) = 0;

    /**
     * Reset ITaskExecutorObserver passed during device creation. Note: sub-devices share the same observer, so it will be reset for sub-devices also.
     * Reaset means no observer calls will be done after from this device and its sub-devices.
     */
    virtual void ResetObserver() = 0;

    /**
     * Set new ITaskExecutorObserver observer for the TEDevice.
     */
    virtual void SetObserver(ITaskExecutorObserver* pObserver) = 0;

    /**
     * Create Task Execution List to the given sub-device
     * @return pointer to the new list or NULL on error
     */
    virtual Intel::OpenCL::Utils::SharedPtr<ITaskList> CreateTaskList(const CommandListCreationParam& param ) = 0;

    /**
     * Retrieves concurrency level for the device
     * @return pointer to the new list or NULL on error
     */
    virtual int GetConcurrency() const = 0;

#ifdef __HARD_TRAPPING__
    /**
     * Trap worker threads in the device
     */
    virtual bool AcquireWorkerThreads(int num_workers = -1, int timeout = -1) = 0;

    /**
     * Release trapped workers
     */
    virtual void RelinquishWorkerThreads() = 0;
#endif // __HARD_TRAPPING__

    /**
     * Wait until all work in a sub-device is complete and mark device as disabled. No more enqueues are allowed after the ShutDown
     */
    virtual void ShutDown() = 0;


    // Workaround for MIC slowness
    /**
      * Set-up/Register calling thread (master) to be used by the device
      */    
    virtual void AttachMasterThread(void* user_tls) = 0;
    /**
      * Unegister calling thread (master) to be used by the device
      */    
    virtual void DetachMasterThread() = 0;

};

// ITaskExecutorObserver - recieves notification on ITaskExecutor events
// Note: ITaskExecutorObserver methods are called inside observer lock, used also by ITEDevice::ResetObserver()
class ITaskExecutorObserver
{
public:
    /**
    *
    *  In each notification following ITaskExecutor can be used:
    *       GetCurrentDevice()      - to determine relevant TE Device
    *       IsMaster()              - to determine is current thread a master or worker
    *       GetPosition()           - to determine current thread position inside a TE Device
    */

    /**
     * Notify about caller thread entry into either ITaskExecutor Root Device or some SubDevice 
     * @return per-thread data to associate with caller thread. NULL - error
     */
    virtual void* OnThreadEntry() = 0;

    /**
     * Notify about caller thread exit from either ITaskExecutor Root Device or some SubDevice 
     * @param  currentThreadData - per-thread data currently associated with caller thread
     */
    virtual void  OnThreadExit( void* currentThreadData ) = 0;

    /**
     * Can thread leave the device? The question is asked only when there is no hard demand. If there is high
     *      demand thread leavs the device unconditionally.
     * @param  currentThreadData - per-thread data currently associated with caller thread
     * @return TE_YES - may leave, TE_NO - prefer not to leave, TE_USE_DEFAULT - decide yourself
     */
    virtual TE_BOOLEAN_ANSWER MayThreadLeaveDevice( void* currentThreadData ) = 0;
};

// Implementation specific class
class ITaskExecutor  
{
public:
    /**
     * Init TE for specified number of threads. The first call specifies the number of threads to be used, 
     * all subsequent calls are ignored, just return number of threads from the first call.
     * @param  AUTO_THREADS cannot be used if uiNumOfLevels > 1.
     * @param ulAdditionalRequiredStackSize can be used to increase amount of stack in threads
     * @return the number of threads initialized, if succeeded, else -1
     */
    virtual int    Init(Intel::OpenCL::Utils::FrameworkUserLogger* pUserLogger, unsigned int uiNumOfThreads = TE_AUTO_THREADS, ocl_gpa_data * pGPAData = nullptr, size_t ulAdditionalRequiredStackSize = 0) = 0;

    virtual void Finalize() = 0;

    /**
     * Return number of threads that may participate in execution at the same time. Initialized at the first Init(). 
     * It is the same value Init() returns on success.
     * @return real number of threads that may concurrently participate in execution overall
     */
    virtual unsigned int GetMaxNumOfConcurrentThreads() const = 0;

    /**
     * Create Root Device 
     * @param device_desc       - device description. Specifies device structure and behavior
     * @param user_data         - some user data to associate with RootDevice - will be returned back by GetCurrentDevice()
     * @return Root Device, if succeeded, else NULL
     */
    virtual Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateRootDevice( 
                                    const RootDeviceCreationParam& device_desc,  
                                    void* user_data = nullptr, ITaskExecutorObserver* my_observer = nullptr ) = 0;

    virtual ocl_gpa_data* GetGPAData() const = 0;    

    // Methods to discover current thread state

    /**
     * Get current sub-device or root device that is executing as part of.
     * @return array of 2 handles - root or sub-device pointer + user handle to root or sub-device
     *                              If thread is outside of any TE Device or sub-device teDevice is set to NULL
     */
    struct DeviceHandleStruct 
    {
        ITEDevice* teDevice;       // returned by CreateRootDevice() or CreateSubdevice()
        void*      user_handle;    // received by CreateRootDevice() or CreateSubdevice()

        DeviceHandleStruct(ITEDevice* dev = nullptr, void* user = nullptr) : teDevice(dev), user_handle(user) {};
    };
    virtual DeviceHandleStruct GetCurrentDevice() const = 0;

    /**
     * Is current thread a user created thread (master) or internall created thread (worker)
     * @return true if master or outside of any TE Device or sub-device
     */
    virtual bool IsMaster() const = 0;

    /**
     * Get current thread position inside a sub-device
     * @param  level - request position at the given hierachy level. 0 - top level. Must be 0 in the flat mode.
     * @return 0-based position inside sub-device at given level or TE_UNKNOWN if level is above maximum or 
     *                 thread is outside of any TE Device or sub-device
     */
    virtual unsigned int GetPosition( unsigned int level = 0 ) const = 0;

    /**
     * @param device a ITEDevice
     * @return a new IThreadLibTaskGroup in device
     */
    virtual Intel::OpenCL::Utils::SharedPtr<IThreadLibTaskGroup> CreateTaskGroup(const Intel::OpenCL::Utils::SharedPtr<ITEDevice>& device) = 0;

    /**
     * Create an in-order debug device queue
     * @param rootDevice the root device on which the queue will reside
     */
    virtual void CreateDebugDeviceQueue(const Intel::OpenCL::Utils::SharedPtr<ITEDevice>& rootDevice) = 0;

    /**
     * Destroy the in-order debug device queue
     */
    virtual void DestroyDebugDeviceQueue() = 0;
    
protected:

    ITaskExecutor() : m_pGPAData(nullptr) { }

    ocl_gpa_data *m_pGPAData;    
    
};

// Function which retrieves TaskExecutor singleton object
TASK_EXECUTOR_API ITaskExecutor* GetTaskExecutor();

TASK_EXECUTOR_API void TE_RegisterGlobalAtExitNotification( Intel::OpenCL::Utils::IAtExitCentralPoint* fn );

}}}
