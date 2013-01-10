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

#include "execution_task.h"
#include "mic_device_interface.h"
#include "mic_tracer.h"

using namespace Intel::OpenCL::MICDevice;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

/* FillMemObjTask inherit from TaskInterface, and implements most of its' functionality.
   It is an abstract class, It does not implement the execution methods which thier implementation is depend on specific threading model. */
class FillMemObjTask : public TaskInterface
{
public:

	/* Initialize the task and execute pre-exe operations.
	   Return CL_DEV_SUCCESS if succeeded. */
	virtual cl_dev_err_code init(TaskHandler* pTaskHandler);

	/* Finish the task and exexute post-exe operations.
	   In regular mode - it will call from "run()" method of this object only. 
	   Call it from other object only in case of error. (In order to execute the post-exe operations). */
	virtual void finish(TaskHandler* pTaskHandler);

	/* Do nothing. */
	virtual cl_dev_err_code attachToThread(TlsAccessor* tlsAccessor, size_t uiWorkerId) { return CL_DEV_SUCCESS; };

	/* Do nothing. */
	virtual cl_dev_err_code	detachFromThread(TlsAccessor* tlsAccessor, size_t uiWorkerId) { return CL_DEV_SUCCESS; };

	/* Do nothing. */
	virtual cl_dev_err_code executeIteration(TlsAccessor* tlsAccessor, HWExceptionsJitWrapper& hw_jit_wrapper, size_t x, size_t y, size_t z, size_t uiWorkerId = (size_t)-1) { return CL_DEV_SUCCESS; };

	/* Return CommandTracer */
	virtual CommandTracer* getCommandTracerPtr() {return m_pCommandTracer; };

protected:

	FillMemObjTask();

	virtual ~FillMemObjTask() {};

	// Fill mem obj dispatcher data
	fill_mem_obj_dispatcher_data*	m_pFillMemObjDispatcherData;

	// The Buffer to fill pointer
	char*							m_fillBufPtr;

	// CommandTracer object
	CommandTracer*					m_pCommandTracer;

};


/* TBBFillMemObjTask inherit from FillMemObjTask
   It impelement the missing functionality of FillMemObjTask that is depenedent on specific threading model. (Using TBB in this case). */
class TBBFillMemObjTask : public FillMemObjTask, public TBBTaskInterface
{
public:

	/* TBBFillMemObjExecutor class Inherit from tbb::task in order to be able to enqueue this object to TBB task queue. */
	class TBBFillMemObjExecutor : public tbb::task
	{
	public:

		virtual ~TBBFillMemObjExecutor() {};

		TBBFillMemObjExecutor(TBBFillMemObjTask* pTbbFillMemObjTask, TaskHandler* pTaskHandler);

		// This method is an abstract method of tbb:task, have to implement it in order to be tbb:task object.
		// This method execute the NDRange task and call finish at the end of it (In order to release resources and to signal the completion barrier if needed).
		tbb::task* execute();

	private:

		// Struct which define chunk of memory (The ptr and its size)
		struct chunk_struct
		{
			char* fromPtr;
			uint64_t size;
		};

		/* fill chunk->size bytes of buffer which pointed by chunk->ptr with pattern */
		void copyPatternOnContRegion(chunk_struct* chunk, chunk_struct* pattern);

		/* Fill recursivly the buffer with pattern according to the definitions in pMemFillInfo.
			buffPtr - The initial address of the buffer we like to fill.
			pMemFillInfo - The dispatcher data we got from host which contain the information how to fill the buffer.
			lastChunk - Is the last chunk of memory that we didn't fill yet. (And we have to fill)
			pattern - The pattern that contain the fill data. */
		void executeInternal(char* buffPtr, fill_mem_obj_dispatcher_data* pMemFillInfo, chunk_struct* lastChunk, chunk_struct* pattern);

		TBBFillMemObjTask* m_pTbbFillMemObjTask;

		TaskHandler* m_taskHandler;
	};



	TBBFillMemObjTask();

	virtual ~TBBFillMemObjTask() {};

	virtual cl_dev_err_code init(TaskHandler* pTaskHandler);

	// It is only delegation method to "execute()" of TBBFillMemObjExecutor.
	virtual void run();

	/* Return the instance of TBBNDRangeExecutor in order to enqueue it to TBB::task::queue. */
	virtual tbb::task* getTaskExecutorObj() {   assert(m_pTaskExecutor);
												return m_pTaskExecutor; };

private:

	TBBFillMemObjExecutor* m_pTaskExecutor;
};

// Define the class BlockingFillMemObjTask.
TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(BlockingFillMemObjTask, BlockingTaskHandler, TBBFillMemObjTask);
// Define the class NonBlockingFillMemObjTask.
TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(NonBlockingFillMemObjTask, TBBNonBlockingTaskHandler, TBBFillMemObjTask);

}}}