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
#include "ocl_itt.h"

#if defined (_WIN32)
#ifdef TASK_EXECUTOR_EXPORTS
#define TASK_EXECUTOR_API	__declspec(dllexport)
#else
#define TASK_EXECUTOR_API	__declspec(dllimport)
#endif
#else
#define TASK_EXECUTOR_API
#endif
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

struct CommandListCreationParam
{
    bool  isOOO;
    bool  isSubdevice;
};

/////////////////////////////////////////////////////////////////////////////
// ITaskBase interface - defines a basic set of functions
class ITaskBase
{
public:
	// Returns whether the executed task is a task set.
    virtual bool	IsTaskSet() = 0;
    // Return task priority, currently the implementation shall return TASK_PRIORITY_MEDIUM
    virtual TASK_PRIORITY	GetPriority() const { return TASK_PRIORITY_MEDIUM;}
    // Releases task object, shall be called instead of delete operator.
    virtual void	Release() = 0;
};

/////////////////////////////////////////////////////////////////////////////
// ITask interface - defines a basic set of functions
class ITask : public ITaskBase
{
public:
	bool	IsTaskSet() {return false;}
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
	bool	IsTaskSet() {return true;}
	// Initialization function. This functions is called before the "main loop"
	// Generally initializes internal data structures
	// Fills the buffer with 3D number of iterations to run
	// Fills regCount with actual number of regions
	// Returns 0 if initialization success, otherwise an error code
	virtual int		Init(size_t region[], unsigned int& regCount) = 0;

	// Is called when the task is going to be called for the first time
	// within specific thread. uiWorkerId specifies the worker thread id.
	// If uiWorkerId = -1, this info is not available.
	// Returns 0, if attach process succeeded, otherwise -1
	virtual int	AttachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) = 0;

	// Is called when the task will not be executed by the specific thread
	// uiWorkerId specifies the worker thread id.
	// If uiWorkerId = -1, this info is not available.
	// Returns 0, if detach process succeeded, otherwise -1
	virtual int	DetachFromThread(unsigned int uiWorkerId) = 0;

	// "Main loop"
	// The function is called with different 'inx' parameters for each iteration number
	virtual void	ExecuteIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId = -1) = 0;

    virtual void	ExecuteAllIterations(size_t* dims, unsigned int uiWorkerId = -1) = 0;

    // Final stage, free execution resources
	virtual void	Finish(FINISH_REASON reason) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// ITaskList interface - defines a function set for task list handling
class ITaskList
{
public:
    // Enqueue a given task for execution, the function is asynchronous and exits immediately.
    // Returns actual number of enqueued tasks
    // Task execution may be started immediately or cashed till Flush() command
	virtual unsigned int	Enqueue(ITaskBase * pTask) = 0; // Dynamically detect Task or TaskSet

	// Ensures that all task were send to execution, non-blocking function
    virtual bool			Flush() = 0;

	// Add the calling thread to execution pool
	// Function blocks, until all tasks belonging to the list are completed.
	virtual te_wait_result WaitForCompletion() = 0;

    // Releases task object, shall be called instead of delete operator.
    virtual void			Release() = 0;
};

// Implementation specific class
class ITaskExecutor
{
public:
	// Init task executor to use uiNumThreads for execution
	// if uiNumThreads == 0, number of threads will be defined by implementation
	// Returns 0, if succeeded, else -1
	virtual int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData) = 0;

	// Return number of initialized worker threads
	virtual unsigned int GetNumWorkingThreads() const = 0;

	virtual ITaskList* CreateTaskList(CommandListCreationParam* param) = 0;

	// Execute task immediately, independently to "listed" tasks
	virtual unsigned int Execute(ITaskBase * pTask) = 0; // Dynamically detect Task or TaskSet

	// Add the calling thread to execution pool
	// Function blocks, until all independent tasks are completed.
	// Return false, if the calling thread was not joined the execution
	virtual te_wait_result WaitForCompletion() = 0;

	// Cancels execution of uncompleted tasks and and then release task executor resources
	virtual void Close(bool bCancel) = 0;

	virtual void ReleasePerThreadData() = 0;

	virtual ocl_gpa_data* GetGPAData() const = 0;

protected:
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

// IAffinityChangeObserver - recieves notification on change of thread affinity
class IAffinityChangeObserver
{
public:
	virtual void NotifyAffinity(unsigned int tid, unsigned int core) = 0;
};

// Factory function to instantiate the appropriate partitioner object
TASK_EXECUTOR_API IThreadPoolPartitioner* CreateThreadPartitioner(IAffinityChangeObserver* pObserver, unsigned int numThreads, unsigned int* legalCoreIDs = 0);

}}}
