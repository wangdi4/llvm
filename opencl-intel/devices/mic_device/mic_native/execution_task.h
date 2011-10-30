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

#include "cl_dev_backend_api.h"

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {


/* Define a class "class_name" that inherite from "task_handler_class", "task_interface_class" and from "TaskContainerInterface" */
#define TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(class_name, task_handler_class, task_interface_class)  \
	class class_name : public task_handler_class, public task_interface_class, public TaskContainerInterface \
	{ \
	public: \
		TaskHandler* getMyTaskHandler() { return this; } \
		\
		TaskInterface* getMyTask() { return this; } \
	}

class TaskInterface;

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

	/* In case of in order queue it will be call from 'runTask' method (of m_task) as the last command,
	   In case of out of order queue, it will be call by the thread that complete the kernel execution.
	   It will release the resources and singal the user barrier if needed. 
	   It also delete this object as the last command. */
	virtual void FinishTask(COIEVENT* completionBarrier = NULL) = 0;

	// The received dispatcher_data
	dispatcher_data* m_dispatcherData;
	// The received misc_data
	misc_data* m_miscData;

	// The input from the main function
	uint32_t m_lockBufferCount;
	void** m_lockBufferPointers;
	uint64_t* m_lockBufferLengths;

	TaskInterface* m_task;

private:

	void setTaskInterface(TaskInterface* task) { m_task = task; }
};


class BlockingTaskHandler : public TaskHandler
{

public:

	/* Initializing the task */
	bool InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

	/* Run the task */
	void RunTask();

protected:

	virtual ~BlockingTaskHandler() {};

	/* It will be call from 'runTask' method (of m_task) as the last command. 
	   It also delete this object as the last command. */
	void FinishTask(COIEVENT* completionBarrier = NULL);
};


class NonBlockingTaskHandler : public TaskHandler
{

public:

	/* Initializing the task */
	bool InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

	/* Run the task */
	void RunTask();

protected:

	virtual ~NonBlockingTaskHandler() {};

	/* It will be call by the thread that complete the kernel execution. 
	   It will release the resources and singal the user barrier. 
	   It also delete this object as the last command. */
	void FinishTask(COIEVENT* completionBarrier = NULL);
};



class TaskInterface
{
public:

	/* Initialize the task and execute pre-exe operations. */
	virtual bool init(TaskHandler* pTaskHandler) = 0;

	/* Run the task */
	virtual void* execute() = 0;

	/* Finish the task and exexute post-exe operations.
	   In regular mode will call from this object "RunTask" method only. 
	   Call it from other object only in case of error. (In order to execute the post operations) */
	virtual void finish(TaskHandler* pTaskHandler) = 0;
};

class NDRangeTask : public TaskInterface
{
public:

	NDRangeTask();

	virtual ~NDRangeTask();

	bool init(TaskHandler* pTaskHandler);

	void* execute();

	void finish(TaskHandler* pTaskHandler);

private:

	// TaskHandler object (Need it as member only due to TBB execute() method signature
	TaskHandler* m_taskHandler;

	ICLDevBackendKernel_* m_kernel;
	ICLDevBackendBinary_* m_pBinary;
	ProgramMemoryManager* m_progamExecutableMemoryManager;

	// Executable information
    size_t m_MemBuffCount;
    size_t* m_pMemBuffSizes;

	// The kernel arguments blob
	char* m_lockedParams;
};


class TaskContainerInterface
{
public:

	virtual TaskHandler* getMyTaskHandler() = 0;

	virtual TaskInterface* getMyTask() = 0;
};


TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(BlockingNDRangeTask, BlockingTaskHandler, NDRangeTask);
TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(NonBlockingNDRangeTask, NonBlockingTaskHandler, NDRangeTask);


}}}