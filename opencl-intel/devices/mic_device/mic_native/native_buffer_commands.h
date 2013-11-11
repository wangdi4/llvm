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

class FillMemObjTask : virtual public Intel::OpenCL::TaskExecutor::ITask, virtual public TaskHandler<FillMemObjTask, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data >
{
public:
    PREPARE_SHARED_PTR(FillMemObjTask)

    FillMemObjTask( uint32_t lockBufferCount, void** pLockBuffers, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data* pDispatcherData, size_t uiDispatchSize );
    
    // TaskHandler methods
    bool PrepareTask();

    // TaskHandler methods
    const FillMemObjTask& GetAsCommandTypeConst() const { return *this; }
    Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() { return static_cast<Intel::OpenCL::TaskExecutor::ITaskBase*>(this);}

    // ITask methods 
    // Returns true in case current task is a synchronization point
    // No more tasks will be executed in this case
    bool    CompleteAndCheckSyncPoint() { return false; }
    
    // Set current command as synchronization point
    // Returns true if command is already completed
    bool    SetAsSyncPoint() { return false; }

    // Returns true if command is already completed
    bool    IsCompleted() const { return false; }
    
    // Task execution routine, will be called by task executor
    // return false when task execution fails
    bool    Execute();

    // Task execution routine, will be called by task executor instead of Execute() if CommandList is canceled
    void    Cancel();    

    // Releases task object, shall be called instead of delete operator.
    long    Release() { delete this; return 0; }

    Intel::OpenCL::TaskExecutor::TASK_PRIORITY   GetPriority() const { return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;}


    Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL;}
    typedef Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data dispatcher_data_type;

protected:
    friend class TaskHandler<FillMemObjTask, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data >;
    // Copy constructor used for task duplication
    FillMemObjTask(const FillMemObjTask& o);

    // The Buffer to fill pointer
    char*                            m_fillBufPtr;
    
    // Struct which define chunk of memory (The ptr and its size)
    struct chunk_struct
    {
        char* fromPtr;
        uint64_t size;
    };

    void copyPatternOnContRegion( chunk_struct* chunk, chunk_struct* pattern );
    void executeInternal( char* buffPtr, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data* pMemFillInfo,
                          chunk_struct* lastChunk, chunk_struct* pattern );
};

}}}
