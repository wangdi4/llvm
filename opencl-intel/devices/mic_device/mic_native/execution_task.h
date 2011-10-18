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

class ExecutionTask
{

public:

	/* Factory for ExecutionTask object (In order or Out of order).
	   DO NOT delete this object, 'finish' metod will delete this object. */
	static ExecutionTask* ExecutionTaskFactory(dispatcher_data* dispatcherData, misc_data* miscData);

	/* Initializing and performing the preExecution conditions */
	bool init(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

	/* Perform PostExecution conditions.
	   AND Delete this object as the last command.
	   In case of in order queue it will be call from 'runTask' method (as the last command),
	   In case of out of order queue, it will be call by the thread that complete the kernel execution. */
	void finish();

	/* Run NDRange task */
	virtual void runTask() = 0;

protected:

	ExecutionTask(dispatcher_data* dispatcherData, misc_data* miscData);

	virtual ~ExecutionTask();

	/* Save the input buffers.
	   In case of OutOfOrder CommandList AddRefCounter for each buffer. (In order to lock it on the device) */
	virtual bool lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength) = 0;

	/* If m_signalBarrierFlag == true signal the completionBarrier. */
	void signalUserBarrierForCompletion();

	void getExecutionRegion(uint64_t region[]);

	// The received dispatcher_data
	dispatcher_data* m_dispatcherData;
	// The received misc_data
	misc_data* m_miscData;

	// The input from the main function
	uint32_t m_lockBufferCount;
	void** m_lockBufferPointers;
	uint64_t* m_lockBufferLengths;

	ICLDevBackendKernel_* m_kernel;
	ICLDevBackendBinary_* m_pBinary;
	ProgramMemoryManager* m_progamExecutableMemoryManager;

	// Executable information
    size_t                      m_MemBuffCount;
    size_t*                     m_pMemBuffSizes;

	// The kernel arguments blob
	char* m_lockedParams;

	// The completionBarrier
	COIEVENT m_completionBarrier;
	bool m_signalBarrierFlag;
};


class BlockingTask : public ExecutionTask
{

public:

	BlockingTask(dispatcher_data* dispatcherData, misc_data* miscData);

	void runTask();

	bool lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

protected:

	virtual ~BlockingTask();
};


class NonBlockingTask : public ExecutionTask
{

public:

	NonBlockingTask(dispatcher_data* dispatcherData, misc_data* miscData);

	void runTask();

	bool lockInputBuffers(uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

protected:

	virtual ~NonBlockingTask();

private:

	void releaseResources();

};

}}}