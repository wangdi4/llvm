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

/*
*
* File task_executor.h
* Declares an interface for task execution model
* The task model should satisfy next guidelines:
*	a.	The task creation order is unknown. Task execution will be handled by list.
*		Lists are of two types, ordered - wherein the tasks shall be executed by the given order
*		un-ordered - wherein tasks are executed without decencies
*	b.	Async. execution. The execution function will be returned immediately,
*		the provided callback function will be called when a task is completed
*	c.	It’ll be two types of tasks:
*		I.	Simple – single function task, single instance of function will be executed
*		II.	Complex (Task Set) – the main function will be executed multiple times.
*			This complex function will have initialization and finalization stages –
*			single function that should be called before and after the main loop.
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
#define TASK_EXECUTOR_API	__declspec(dllexport)
#else
#define TASK_EXECUTOR_API	__declspec(dllimport)
#endif
#else
#define TASK_EXECUTOR_API
#endif

// Forward declaration
struct ocl_gpa_data;

namespace Intel { namespace OpenCL { namespace TaskExecutor {

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
	TE_WAIT_COMPLETED				= 0,// All processing tasks were completed
	TE_WAIT_MASTER_THREAD_BLOCKING,		// WaitForCompletion was blocked by another master thread
	TE_WAIT_NOT_SUPPORTED				// Wait for completion doesn't supported
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

// Command List Creation params
struct CommandListCreationParam
{
    TE_CMD_LIST_TYPE                    cmdListType;
    TE_CMD_LIST_PREFERRED_SCHEDULING    preferredScheduling;

    CommandListCreationParam( TE_CMD_LIST_TYPE type, 
                              TE_CMD_LIST_PREFERRED_SCHEDULING sched = TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC ) :
        cmdListType(type), preferredScheduling(sched) {}
};

/////////////////////////////////////////////////////////////////////////////
// ITaskBase interface - defines a basic set of functions
class ITaskBase : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(ITaskBase)

    virtual ~ITaskBase() { }

	// Returns whether the executed task is a task set.
    virtual bool	IsTaskSet() const = 0;
    // Return task priority, currently the implementation shall return TASK_PRIORITY_MEDIUM
    virtual TASK_PRIORITY	GetPriority() const { return TASK_PRIORITY_MEDIUM;}

	// Returns true in case current task is a syncronization point
	// No more tasks will be executed in this case
	virtual bool	CompleteAndCheckSyncPoint() { return false; }
	
	// Set current command as syncronization point
	// Returns true if command is already completed
	virtual bool	SetAsSyncPoint() { return false; }

	// Returns true if command is already completed
	virtual bool	IsCompleted() const { return false; }

    // Releases task object, shall be called instead of delete operator.
    virtual long	Release() = 0;

    // overriden from ReferenceCountedObject
    std::string GetTypeName() const { return "ITaskBase"; }
};

/////////////////////////////////////////////////////////////////////////////
// ITask interface - defines a basic set of functions
class ITask : public ITaskBase
{
public:

    PREPARE_SHARED_PTR(ITask)

	bool	IsTaskSet() const {return false;}
	// Task execution routine, will be called by task executor
	// return false when task execution fails
	virtual bool	Execute() = 0;
    // Affinitizes the calling thread to this task's affinity mask if applicable
    virtual void    AffinitizeToTask() {}
};

/////////////////////////////////////////////////////////////////////////////
// ITaskSet interface - defines a function set for execution of complex tasks
class ITaskSet: public ITaskBase
{
public:
    PREPARE_SHARED_PTR(ITaskSet)

	bool	IsTaskSet() const {return true;}
	// Initialization function. This functions is called before the "main loop"
	// Generally initializes internal data structures
	// Fills the buffer with 3D number of iterations to run
	// Fills regCount with actual number of regions
	// Returns 0 if initialization success, otherwise an error code
	virtual int		Init(size_t region[], unsigned int& regCount) = 0;

	// Is called when the task is going to be called for the first time
	// within specific thread. 
	// Returns 0, if attach process succeeded, otherwise -1
	virtual int	AttachToThread(WGContextBase* pWgContext, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) = 0;

	// Is called when the task will not be executed by the specific thread	
	// Returns 0, if detach process succeeded, otherwise -1
	virtual int	DetachFromThread(WGContextBase* pWgContext) = 0;

	// "Main loop"
	// The function is called with different 'inx' parameters for each iteration number
	virtual void	ExecuteIteration(size_t x, size_t y, size_t z, WGContextBase* pWgContext = NULL) = 0;

    virtual void	ExecuteAllIterations(size_t* dims, WGContextBase* pWgContext = NULL) = 0;

    // Final stage, free execution resources
	// Return false when command execution fails
	virtual bool	Finish(FINISH_REASON reason) = 0;

    // Optimize By
    virtual TASK_SET_OPTIMIZATION OptimizeBy()                        const { return TASK_SET_OPTIMIZE_DEFAULT; }
    virtual unsigned int          PreferredSequentialItemsPerThread() const { return 1; }
    
};

/////////////////////////////////////////////////////////////////////////////
// ITaskList interface - defines a function set for task list handling
class ITaskList : public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:

    PREPARE_SHARED_PTR(ITaskList)

    // Enqueue a given task for execution, the function is asynchronous and exits immediately in in-order and out-of-order lists.
    // In immediate list the task is enqueued, flushed and executes immediately. Functions returns only after the task completes.
    // Returns actual number of enqueued tasks
    // Task execution may be started immediately or postponed till Flush() command
	virtual unsigned int	Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask) = 0; // Dynamically detect Task or TaskSet

	// Ensures that all task were send to execution, non-blocking function
    virtual bool			Flush() = 0;

	// Add the calling thread to execution pool
	// Function blocks, until the pTask is completed or in case of NULL
	// all tasks belonging to the list are completed.
	// Not supported for immediate lists
    virtual te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait) = 0;    

};

// IAffinityChangeObserver - recieves notification on change of thread affinity
class IAffinityChangeObserver
{
public:
	virtual void NotifyAffinity(unsigned int tid, unsigned int core) = 0;
};

// Implementation specific class
class ITaskExecutor
{
public:
	// Init task executor to use uiNumThreads for execution
	// if uiNumThreads == AUTO_THREADS, number of threads will be defined by implementation
	// Returns 0, if succeeded, else -1
	static const unsigned int AUTO_THREADS = (unsigned int)(-1);
    virtual int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData) = 0;
    virtual void Finalize() = 0;

    /**
     * Set the IWGContextPool of the device agent
     * @param wgContextPool the IWGContextPool to be set
     */
    virtual void SetWGContextPool(IWGContextPool* pWgContextPool) = 0;

    /**
     * @param bBelongsToMasterThread whether the WG context belong to a master thread
     * @return a pointer to an allocated work group context for a worker thread or NULL if the IWGContextPool is not available any more
     */
    virtual WGContextBase* GetWGContext(bool bBelongsToMasterThread) = 0;

    /**
     * @param pWgContext a pointer to a work group context for a worker thread to be freed
     */
    virtual void ReleaseWorkerWGContext(WGContextBase* wgContext) = 0;

	// Activate thread pool. All worker threads are created
	virtual bool Activate() = 0;

	// Deactivate thread pool. All workrer threads are destroyed
	virtual void Deactivate() = 0;

	// Return number of initialized worker threads
	virtual unsigned int GetNumWorkingThreads() const = 0;

    // pSubdevTaskExecData is private data of the TaskExecutor for the specific sub-device or NULL for the root device
	virtual Intel::OpenCL::Utils::SharedPtr<ITaskList> CreateTaskList(const CommandListCreationParam& param, void* pSubdevTaskExecData = NULL) = 0;

	// Execute task immediately, independently to "listed" tasks. If pSubdevTaskExecData is not NULL, the task will be executed on this sub-device
    virtual unsigned int Execute(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask, void* pSubdevTaskExecData = NULL) = 0; // Dynamically detect Task or TaskSet

	// Add the calling thread to execution pool
	// Function blocks, until all independent tasks are completed.
	// Return false, if the calling thread was not joined the execution
	// If pSubdevTaskExecData is not NULL, wait on this sub-device
	virtual te_wait_result WaitForCompletion(ITaskBase * pTask, void* pSubdevTaskExecData = NULL) = 0;

	virtual ocl_gpa_data* GetGPAData() const = 0;    

    /**
     * @param uiNumSubdevComputeUnits   number of computing units in the sub-device
     * @return a pointer to an object representing the sub-device in the TaskExecutor module
     * @param pLegalCores       an array of the core IDs which it is legal for the worker threads to affinitize to (size of the array is uiNumSubdevComputeUnits)
     * @param observer          the IAffinityChangeObserver that observers changes in the affinity of worker threads																		
     */
    virtual void* CreateSubdevice(unsigned int uiNumSubdevComputeUnits, const unsigned int* pLegalCores, IAffinityChangeObserver& observer) = 0;

    /**
     * Release a sub-device
     * @param pSubdevData a pointer to the object representing the sub-device in the TaskExecutor module
     */
    virtual void ReleaseSubdevice(void* pSubdevData) = 0;

    /**
     * Wait until all work in a sub-device is complete
     * @param pSubdevData a pointer to the object representing the sub-device in the TaskExecutor module
     */
    virtual void WaitUntilEmpty(void* pSubdevData) = 0;

protected:

    ITaskExecutor() : m_pGPAData(NULL) { }

	ocl_gpa_data *m_pGPAData;    
    
};

// Function which retrieves TaskExecutor singleton object
TASK_EXECUTOR_API ITaskExecutor* GetTaskExecutor();

// IThreadPartitioner class - a class enabling controlling executing threads by affinity mask / count
class IThreadPoolPartitioner
{
public:
	virtual ~IThreadPoolPartitioner() {}
    virtual bool Activate()   = 0;
    virtual void Deactivate() = 0;
};

}}}
