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
#include "task_handler.h"
#include "program_memory_manager.h"
#include "native_printf.h"
#include "mic_tbb_tracer.h"
#include "mic_device_interface.h"

#include <task_executor.h>

#ifdef USE_ITT
#include <ocl_itt.h>
#endif

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

class NDRangeTask : virtual public Intel::OpenCL::TaskExecutor::ITaskSet, virtual public TaskHandler<NDRangeTask, Intel::OpenCL::MICDevice::ndrange_dispatcher_data >
{
public:
    PREPARE_SHARED_PTR(NDRangeTask)

    NDRangeTask( uint32_t lockBufferCount, void** pLockBuffers, Intel::OpenCL::MICDevice::ndrange_dispatcher_data* pDispatcherData, size_t uiDispatchSize );
    ~NDRangeTask();

    // TaskHandlerBase methods
    bool PrepareTask();

    // TaskHandler methods
    const NDRangeTask& GetAsCommandTypeConst() const { return *this; }
    Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() { return static_cast<Intel::OpenCL::TaskExecutor::ITaskBase*>(this);}

    // ITaskSet methods 
    // Returns true in case current task is a synchronization point
    // No more tasks will be executed in this case
    bool    CompleteAndCheckSyncPoint() { return false; }
    
    // Set current command as synchronization point
    // Returns true if command is already completed
    bool    SetAsSyncPoint() { return false; }

    // Returns true if command is already completed
    bool    IsCompleted() const { return false; }

    // Initialization function. This functions is called before the "main loop"
    // Generally initializes internal data structures
    // Fills the buffer with 3D number of iterations to run
    // Fills regCount with actual number of regions
    // Returns 0 if initialization success, otherwise an error code
    int     Init(size_t region[], unsigned int& regCount);

    // Is called when the task is going to be called for the first time
    // within specific thread. 
    // Returns void* to be passed to other methods, if attach process succeeded, otherwise NULL
    void*   AttachToThread(void* pWgContextBase, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);

    // Is called when the task will not be executed by the specific thread    
    void    DetachFromThread(void* pWgContext);

    // "Main loop"
    // The function is called with different 'inx' parameters for each iteration number
    // Return false to break iterations
    bool    ExecuteIteration(size_t x, size_t y, size_t z, void* pWgContext = NULL);

   // Final stage, free execution resources
    // Return false when command execution fails
    bool    Finish(Intel::OpenCL::TaskExecutor::FINISH_REASON reason);

    // Task execution routine, will be called by task executor instead of Init() if CommandList is canceled. If Init() was already called,
    // Cancel() is not called - normal processing is continued
    void Cancel();

    // Releases task object, shall be called instead of delete operator.
    long    Release() { if (m_bDuplicated) delete this; return 0; }

    // Optimize By
    Intel::OpenCL::TaskExecutor::TASK_PRIORITY         GetPriority() const { return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;}
    Intel::OpenCL::TaskExecutor::TASK_SET_OPTIMIZATION OptimizeBy() const  { return gMicExecEnvOptions.tbb_block_optimization; }
    unsigned int          PreferredSequentialItemsPerThread() const        { return gMicExecEnvOptions.use_TBB_grain_size; }

    Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL;}

    typedef Intel::OpenCL::MICDevice::ndrange_dispatcher_data dispatcher_data_type;
protected:
    friend class TaskHandler<NDRangeTask, Intel::OpenCL::MICDevice::ndrange_dispatcher_data >;
    // Copy constructor used for task duplication
    NDRangeTask(const NDRangeTask& o);

    const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_*   m_kernel;
    Intel::OpenCL::DeviceBackend::ICLDevBackendBinary_*         m_pBinary;

#ifndef __NEW_BE_API__
    // !!!!!!
    // TODO: Remove with new API
    // Executable information
    size_t                m_MemBuffCount;
    size_t*               m_pMemBuffSizes;
    //////////////////////////////////////////
#endif

    // Print handle for this command
    PrintfHandle          m_printHandle;
    bool                  m_bSecureExecution;

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

