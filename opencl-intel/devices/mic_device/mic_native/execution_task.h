// Copyright (c) 2006-2008 Intel Corporation
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

/////////////////////////////////////////////////////////////
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once

#include "mic_device_interface.h"
#include "program_memory_manager.h"
#include "cl_sys_defines.h"

#include "cl_dev_backend_api.h"

#include <tbb/tbb.h>
#include <tbb/task.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {


/* Define a class "class_name" that inherit from "task_handler_class", "task_interface_class" and from "TaskContainerInterface" */
#define TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(class_name, task_handler_class, task_interface_class)  \
	class class_name : public task_handler_class, public task_interface_class, public TaskContainerInterface \
	{ \
	public: \
		TaskHandler* getMyTaskHandler() { return this; } \
		\
		TaskInterface* getMyTask() { return this; } \
	}

class TaskInterface;

/* TaskHandler is an abstract class that manage the execution of a task. */
class TaskHandler
{
public:

	friend class NDRangeTask;

	enum TASK_TYPES
	{
		NDRANGE_TASK_TYPE = 0
	};

	/* Factory for TaskHandler object (In order or Out of order).
	   DO NOT delete this object, 'finish' metod will delete this object. */
	static TaskHandler* TaskFactory(TASK_TYPES type, dispatcher_data* dispatcherData, misc_data* miscData);

	/* Initializing the task */
	virtual bool InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength) = 0;

	/* Run the task */
	virtual void RunTask() = 0;

protected:

	TaskHandler();

	virtual ~TaskHandler() {};

	/* It will be call from 'runTask' method (of m_task) as the last command,
	   It will release the resources and singal the user barrier if needed. 
	   It also delete this object as the last command. 
	   The FinishTask is not public because We don't want the user to release the resource. (It will release itself when completed)*/
	virtual void FinishTask(COIEVENT* completionBarrier = NULL) = 0;

	// The received dispatcher_data
	dispatcher_data* m_dispatcherData;
	// The received misc_data
	misc_data* m_miscData;

	// The input from the main function
	uint32_t m_lockBufferCount;
	void** m_lockBufferPointers;
	uint64_t* m_lockBufferLengths;

	// a pointer to TaskInterface
	TaskInterface* m_task;

private:

	// Setter for this object TaskInterface.
	void setTaskInterface(TaskInterface* task) { m_task = task; }
};


/* BlockingTaskHandler inherits from "TaskHandler" and implement the functionality for Blocking task managment. */
class BlockingTaskHandler : public TaskHandler
{

public:

	bool InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

	void RunTask();

protected:

	virtual ~BlockingTaskHandler() {};

	void FinishTask(COIEVENT* completionBarrier = NULL);
};


/* NonBlockingTaskHandler inherits from "TaskHandler" and implement the functionality for Non-Blocking task management.
   It is an abstract class becuase it doesn't implement the method "RunTask()" which is thread specific implementation. */
class NonBlockingTaskHandler : public TaskHandler
{

public:

	bool InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

protected:

	virtual ~NonBlockingTaskHandler() {};

	void FinishTask(COIEVENT* completionBarrier = NULL);
};


/* TBBNonBlockingTaskHandler inherits from NonBlockingTaskHandler and implement the asynch execution call by using TBB mechanism. */
class TBBNonBlockingTaskHandler : public NonBlockingTaskHandler
{

public:

	void RunTask();

protected:

	virtual ~TBBNonBlockingTaskHandler() {};
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Interface for Task Execution. */
class TaskInterface
{
public:

	/* Initialize the task and execute pre-exe operations. */
	virtual bool init(TaskHandler* pTaskHandler) = 0;

	/* Run the task */
	virtual void run() = 0;

	/* Finish the task and exexute post-exe operations.
	   In regular mode will call from this object "RunTask" method only. 
	   Call it from other object only in case of error. (In order to execute the post-exe operations) */
	virtual void finish(TaskHandler* pTaskHandler) = 0;

	// Is called when the task is going to be called for the first time
	// within specific thread. uiWorkerId specifies the worker thread id.
	// Returns CL_DEV_SUCCESS, if attach process succeeded.
	virtual cl_dev_err_code attachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) = 0;

	// Is called when the task will not be executed by the specific thread
	// uiWorkerId specifies the worker thread id.
	// Returns CL_DEV_SUCCESS, if detach process succeeded.
	virtual cl_dev_err_code	detachFromThread(unsigned int uiWorkerId) = 0;

	// The function is called with different 'inx' parameters for each iteration number
	virtual void executeIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId = (unsigned int)-1) = 0;

};


/* NDRangeTask inherit from TaskInterface, and implements most of its' functionality.
   It is an abstract class, It does not implement the execution methods which thier implementation is depend on specific threading model. */
class NDRangeTask : public TaskInterface
{
public:

	bool init(TaskHandler* pTaskHandler);

	void finish(TaskHandler* pTaskHandler);

	cl_dev_err_code attachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);

	cl_dev_err_code	detachFromThread(unsigned int uiWorkerId);

	void executeIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId = (unsigned int)-1);

protected:

	NDRangeTask();

	virtual ~NDRangeTask();


	// TaskHandler object (Need it as member only due to TBB execute() method signature)
	TaskHandler* m_taskHandler;

	ICLDevBackendKernel_* m_kernel;
	ICLDevBackendBinary_* m_pBinary;
	ProgramMemoryManager* m_progamExecutableMemoryManager;

	// Executable information
    size_t m_MemBuffCount;
    size_t* m_pMemBuffSizes;
	
	// working region
	uint64_t m_region[MAX_WORK_DIM];
	// dimensions
	unsigned int m_dim;

	// The kernel arguments blob
	char* m_lockedParams;
};



/* TBBNDRangeTask inherit from NDRangeTask and from tbb::task.
   It impelement the missing functionality of NDRangeTask that is depenedent on specific threading model. (Using TBB in this case).
   Inherit from tbb::task in order to be able to enqueue this object to TBB task queue. */
class TBBNDRangeTask : public NDRangeTask, public tbb::task
{
public:

	TBBNDRangeTask();

	virtual ~TBBNDRangeTask() {};

	// It is only delegation method to "execute()".
	void run() { execute(); };

	// This method is an abstract method of tbb:task, have to implement it in order to be tbb:task object.
	// This method execute the NDRange task and call finish at the end of it (In order to release resources and to signal the completion barrier if needed).
	tbb::task* execute();

};


/* Interface that define to methods that must be implement for each Task class. */
class TaskContainerInterface
{
public:

	/* Return this object as Taskhandler. */
	virtual TaskHandler* getMyTaskHandler() = 0;

	/* Return this object as TaskInterface. */
	virtual TaskInterface* getMyTask() = 0;
};


TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(BlockingNDRangeTask, BlockingTaskHandler, TBBNDRangeTask);
TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(NonBlockingNDRangeTask, TBBNonBlockingTaskHandler, TBBNDRangeTask);


}}}