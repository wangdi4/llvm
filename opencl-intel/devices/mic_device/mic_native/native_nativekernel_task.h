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
//  native_nativekernel_task.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once
#include "task_handler.h"
#include "program_memory_manager.h"
#include "mic_tbb_tracer.h"
#include "mic_device_interface.h"

#include <builtin_kernels.h>
#include <task_executor.h>

#ifdef USE_ITT
#include <ocl_itt.h>
#endif

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

class NativeKernelTask : virtual public Intel::OpenCL::TaskExecutor::ITask,
                    virtual public TaskHandler<NativeKernelTask, Intel::OpenCL::MICDevice::ndrange_dispatcher_data >
{
public:
    PREPARE_SHARED_PTR(NativeKernelTask)

    NativeKernelTask( uint32_t lockBufferCount, void** pLockBuffers, Intel::OpenCL::MICDevice::ndrange_dispatcher_data* pDispatcherData, size_t uiDispatchSize, QueueOnDevice* pQueue );
    ~NativeKernelTask();

    static SharedPtr<NativeKernelTask> Allocate( uint32_t lockBufferCount, void** pLockBuffers, Intel::OpenCL::MICDevice::ndrange_dispatcher_data* pDispatcherData, size_t uiDispatchSize, QueueOnDevice* pQueue )
    {
        return new NativeKernelTask( lockBufferCount, pLockBuffers, pDispatcherData, uiDispatchSize, pQueue );
    }

    // TaskHandlerBase methods
    bool PrepareTask();

    // TaskHandler methods
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

    bool    Execute();

    // Releases task object, shall be called instead of delete operator.
    long    Release() { return releaseImp(); };

    // Task execution routine, will be called by task executor instead of Init() if CommandList is canceled. If Init() was already called,
    // Cancel() is not called - normal processing is continued
    void Cancel();

    // Optimize By
    Intel::OpenCL::TaskExecutor::TASK_PRIORITY         GetPriority() const { return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;}
    Intel::OpenCL::TaskExecutor::TASK_SET_OPTIMIZATION OptimizeBy() const  { return gMicExecEnvOptions.tbb_block_optimization; }
    unsigned int          PreferredSequentialItemsPerThread() const        { return gMicExecEnvOptions.use_TBB_grain_size; }

    Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup* GetNDRangeChildrenTaskGroup() { return nullptr;}

    typedef Intel::OpenCL::MICDevice::ndrange_dispatcher_data dispatcher_data_type;


protected:
    friend class TaskHandler<NativeKernelTask, Intel::OpenCL::MICDevice::ndrange_dispatcher_data >;

    const Intel::OpenCL::BuiltInKernels::IBuiltInKernel*  m_pKernel;
    char*                                                 m_pKernelArgs;

#ifdef ENABLE_MIC_TRACER
    friend class          NDRangePerfData;
    NDRangePerfData       m_tbb_perf_data;
#endif

#ifdef USE_ITT
    __itt_string_handle*        m_pIttKernelName;
    __itt_domain*               m_pIttKernelDomain;
#endif
};

}}}

