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

/////////////////////////////////////////////////////////////
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once
#include "task_executor.h"
#include "device_queue.h"
#include "mic_device_interface.h"
#include "mic_tracer.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::MICDevice;

class FillMemObjTask : virtual public ITask, virtual public TaskHandler
{
public:
    PREPARE_SHARED_PTR(FillMemObjTask)
    
    static inline SharedPtr<FillMemObjTask> Allocate( const QueueOnDevice& queue ) { return new FillMemObjTask( queue ); }
    
    // TaskHandler methods
    
    // called immediately after creation and after filling the COI-passed data
    bool InitTask();

    // must be called at the very end of the ITaskBase finish stage and 
    // must call to QueueOnDevice->FinishTask() at the very end of itself
    void FinishTask();

    // ITask methods 
    
	// Task execution routine, will be called by task executor
	// return false when task execution fails
	bool	Execute();

    // Releases task object, shall be called instead of delete operator.
    long    Release() { delete this; return 0; }

    // overriden from ReferenceCountedObject
    std::string GetTypeName() const { return "FillMemObjTask"; }

protected:
    FillMemObjTask( const QueueOnDevice& queue );
    ~FillMemObjTask() {};

private:
	// Fill mem obj dispatcher data
	fill_mem_obj_dispatcher_data*	m_pFillMemObjDispatcherData;

	// The Buffer to fill pointer
	char*							m_fillBufPtr;
    
	// Struct which define chunk of memory (The ptr and its size)
	struct chunk_struct
	{
		char* fromPtr;
		uint64_t size;
	};

    void copyPatternOnContRegion( chunk_struct* chunk, chunk_struct* pattern );
    void executeInternal( char* buffPtr, fill_mem_obj_dispatcher_data* pMemFillInfo, 
                          chunk_struct* lastChunk, chunk_struct* pattern );

    
};

}}}
