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

#ifdef TASK_EXECUTOR_EXPORTS
#define TASK_EXECUTOR_API	__declspec(dllexport)
#else
#define TASK_EXECUTOR_API	__declspec(dllimport)
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
	FINISH_EXECUTION_FAILED,
} FINISH_REASON;

/////////////////////////////////////////////////////////////////////////////
// ITaskBase interface - defines a basic set of functions
class ITaskBase
{
public:
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
	virtual void	Execute() = 0;
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
	// Returns 0 if initialization success, otherwise an error code
	virtual int		Init(unsigned int region[]) = 0;

	// Is called when the task is going to be called for the first time
	// within specific thread. uiWorkerId specifies the worker thread id.
	// If uiWorkerId = -1, this info is not available.
	// Returns 0, if attach process succeeded, otherwise -1
	virtual int	AttachToThread(unsigned int uiWorkerId) = 0;

	// "Main loop"
	// The function is called with different 'inx' parameters for each iteration number
	virtual void	ExecuteIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId = -1) = 0;

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
//    virtual unsigned int	Enqueue(ITask * pTask) = 0; // reference may be better to protect from NULL
//    virtual unsigned int	Enqueue(ITaskSet * pTask) = 0; // overloaded to distinguish types at compile-time
    // Insures that all task were send to execution, non-blocking function
    virtual void			Flush() = 0;
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
	virtual int	Init(unsigned int uiNumThreads) = 0;

	// Return number of initialized worker threads
	virtual unsigned int GetNumWorkingThreads() const = 0;

	virtual ITaskList* CreateTaskList(bool OOO = false) = 0;

	// Execute task immediately
	virtual unsigned int	Execute(ITaskBase * pTask) = 0; // Dynamically detect Task or TaskSet

	// Cancels execution of uncompleted tasks and and then release task executor resources
	virtual void Close(bool bCancel) = 0;
};

// Function which retrieves TaskExecutor singleton object 
TASK_EXECUTOR_API ITaskExecutor* GetTaskExecutor();

}}}