// Copyright (c) 2009 Intel Corporation
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

///////////////////////////////////////////////////////////
//  dispatcher_commands.h
//  Declaration of internal task dispatcher commands
////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "cpu_dev_limits.h"
#include "memory_allocator.h"
#include "program_service.h"
#include "task_executor.h"
#include "wg_context.h"
#include "cl_synch_objects.h"
#include "kernel_command.h"
#include "ocl_itt.h"
#include "cl_thread.h"
#include "tbb/scalable_allocator.h"

#define COLOR_TABLE_SIZE 64

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::DeviceCommands;

namespace Intel { namespace OpenCL  { namespace BuiltInKernels {
    class IBuiltInKernel;
}}}

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher;

typedef cl_dev_err_code fnDispatcherCommandCreate_t(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

// Base class for handling dispatcher command execution
// All Commands will be implement this interface
class DispatcherCommand
{
public:
    DispatcherCommand(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
    virtual ~DispatcherCommand();

    /**
     * @return the DispatcherCommand's TaskDispatcher
     */
    TaskDispatcher* GetTaskDispatcher() { return m_pTaskDispatcher; }

protected:
    void NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr);    

    cl_dev_err_code ExtractNDRangeParams(void* pTargetTaskParam, const cl_kernel_argument*   pParams, 
                                         cl_dev_cmd_memobj_param_kernel* pMemObjParams, size_t stMemObjParams);

    TaskDispatcher*             m_pTaskDispatcher;
    MemoryAllocator*            m_pMemAlloc;
    IOCLDevLogDescriptor*       m_pLogDescriptor;
    cl_int                      m_iLogHandle;
    cl_dev_cmd_desc*            m_pCmd;
    ocl_gpa_data*               m_pGPAData;
#if defined(USE_ITT)
    __itt_id                    m_ittID;
#endif

    volatile bool                m_bCompleted;
};

template<class ITaskClass>
    class CommandBaseClass : public ITaskClass, public DispatcherCommand
{
public:
    CommandBaseClass(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
      DispatcherCommand(pTD, pCmd)
    {
        m_aIsSyncPoint = FALSE;
    }

    ~CommandBaseClass()
    {
    }

    // ITaskBase
    bool            SetAsSyncPoint();
    bool            CompleteAndCheckSyncPoint();
    bool            IsCompleted() const {return m_bCompleted;}
    long            Release() { return 0; }
    TASK_PRIORITY   GetPriority() const { return TASK_PRIORITY_MEDIUM;}

    void    Cancel() { NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, cl_int(CL_DEV_COMMAND_CANCELLED)); }
    Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL; }

protected:

    Intel::OpenCL::Utils::AtomicCounter    m_aIsSyncPoint;

};

// OCL Read/Write buffer execution
class ReadWriteMemObject : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // ITask interface
    bool    Execute();

protected:
    ReadWriteMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);
};

//OCL Copy Mem Obj Command
class CopyMemObject : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();

protected:
    CopyMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
};

// OCL Native function execution
class NativeFunction : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();

protected:
    NativeFunction(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);

    char*               m_pArgV;
};

// OCL Map function execution
class MapMemObject : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();

protected:
    MapMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
};

// OCL UnMap function execution
class UnmapMemObject : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();

protected:
    UnmapMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
};

// OCL Kernel execution
class NDRange : public CommandBaseClass<ITaskSet>, public KernelCommand
{
public:    

    PREPARE_SHARED_PTR(NDRange)

    static unsigned int RGBTable[COLOR_TABLE_SIZE];
    static AtomicCounter RGBTableCounter;

    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    /**
     * @return the NDRange that is currently running on the local thread
     */
    static NDRange* GetThreadLocalNDRange() { return sm_pCurrentWgContext->GetCurrentNDRange(); }

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITaskSet interface
    int     Init(size_t region[], unsigned int &regCount);
    void*   AttachToThread(void* pWgContext, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);
    void    DetachFromThread(void* pWgContext);
    bool    ExecuteIteration(size_t x, size_t y, size_t z, void* pWgContext);
    bool    Finish(FINISH_REASON reason);
    
    // Optimize By
    TASK_SET_OPTIMIZATION OptimizeBy()                        const { return TASK_SET_OPTIMIZE_DEFAULT; }
    unsigned int          PreferredSequentialItemsPerThread() const { return 1; }

    bool IsCompleted() const { return CommandBaseClass<ITaskSet>::IsCompleted(); }

    ITaskGroup* GetNDRangeChildrenTaskGroup() { return GetParentTaskGroup().GetPtr(); }
    char* GetParamsPtr() { return (char*)(((cl_dev_cmd_param_kernel*)m_pCmd->params)->arg_values); }

protected:
    NDRange(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, const SharedPtr<ITaskList>& pList, const SharedPtr<KernelCommand>& parent, const SharedPtr<ITaskGroup>& childrenTaskGroup);

    cl_dev_err_code CreateBinary(const ICLDevBackendKernel_* kernel, size_t szArgSize, const cl_work_description_type* workDesc, ICLDevBackendBinary_** pBinary);

    // inherited from DeviceCommand and KernelCommand

    std::vector<SharedPtr<KernelCommand> >& GetWaitingChildrenForWG() { return sm_pCurrentWgContext->GetWaitingChildrenForWg(); }

    std::vector<SharedPtr<KernelCommand> >& GetWaitingChildrenForParentInWg() { return sm_pCurrentWgContext->GetWaitingChildrenForParent(); }

    cl_int                      m_lastError;
    ICLDevBackendBinary_*       m_pBinary;

    // Executable information
    size_t                      m_MemBuffCount;
    size_t*                     m_pMemBuffSizes;

    // Information about the hardware and a potential override for work group to thread mapping
    unsigned int                m_numThreads;
    bool                        m_bEnablePredictablePartitioning;

    // Used when running in "predictable partitioning" mode (i.e. 1:1 mapping between threads and WGs when using fission)
    // Ensures no work group is executed twice, regardless of task stealing
    Intel::OpenCL::Utils::AtomicBitField m_bWGExecuted;

    // Unique ID of the NDRange command
    static Intel::OpenCL::Utils::AtomicCounter    s_lGlbNDRangeId;
    long                                                            m_lNDRangeId;

    static THREAD_LOCAL WGContext* sm_pCurrentWgContext;

#ifdef _DEBUG
    // For debug
    AtomicCounter m_lExecuting;
    AtomicCounter m_lFinish;
    AtomicCounter m_lAttaching;
#endif
};

/**
 * This class represents an NDRange command enqueued by a kernel
 */
class DeviceNDRange : public NDRange
{
public:

    PREPARE_SHARED_PTR(DeviceNDRange)

    static SharedPtr<DeviceNDRange> Allocate(TaskDispatcher* pTD, const SharedPtr<ITaskList>& pList, const SharedPtr<KernelCommand>& parent, const SharedPtr<ITaskGroup>& childrenTaskGroup,
        const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel, const void* pContext, size_t szContextSize, const cl_work_description_type* pNdrange)
    {
        return new DeviceNDRange(pTD, pList, parent, childrenTaskGroup, pKernel, pContext, szContextSize, pNdrange);
    }

    DeviceNDRange(TaskDispatcher* pTD, const SharedPtr<ITaskList>& pList, const SharedPtr<KernelCommand>& parent, const SharedPtr<ITaskGroup>& childrenTaskGroup,
        const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel, const void* pContext, size_t szContextSize, const cl_work_description_type* pNdrange
#if 0
        , tbb::scalable_allocator<DeviceNDRange>& deviceNDRangeAllocator,
        tbb::scalable_allocator<char>& deviceNDRangeContextAllocator
#endif
        ) : NDRange(pTD, InitCmdDesc(m_paramKernel, m_cmdDesc, pKernel, pContext, szContextSize, pNdrange, pList), pList, parent, childrenTaskGroup)
#if 0
        , m_deviceNDRangeAllocator(deviceNDRangeAllocator),
        m_deviceNDRangeContextAllocator(deviceNDRangeContextAllocator)
#endif
        { } 

    /**
     * @return the next available command ID for DeviceNDRange commands
     */
    static long GetNextCmdId() { return sm_cmdIdCnt++; }    

    // inherited methods:    

    int    Init(size_t region[], unsigned int &regCount);

    bool Finish(FINISH_REASON reason);

    void Cleanup(bool bIsTerminate = false)
    { 
        const cl_dev_cmd_param_kernel& paramKerel = *((cl_dev_cmd_param_kernel*)m_cmdDesc.params);
#if 0
        m_deviceNDRangeContextAllocator.deallocate((char*)paramKerel.arg_values, paramKerel.arg_size);
        m_deviceNDRangeAllocator.deallocate(this, sizeof(DeviceNDRange));        
#else
        delete[] (const char*)(paramKerel.arg_values);
        delete this;
#endif
    }

private:

    // dummy classes until we have BE implementation:

    static cl_dev_cmd_desc* InitCmdDesc(cl_dev_cmd_param_kernel& paramKernel, cl_dev_cmd_desc& cmdDesc, const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
        const void* pContext, size_t szContextSize, const cl_work_description_type* pNdrange, const SharedPtr<ITaskList>& pList);    
    
    static AtomicCounter sm_cmdIdCnt;

    cl_dev_cmd_param_kernel m_paramKernel;
    cl_dev_cmd_desc m_cmdDesc;
#if 0
    tbb::scalable_allocator<DeviceNDRange>& m_deviceNDRangeAllocator;
    tbb::scalable_allocator<char>& m_deviceNDRangeContextAllocator;
#endif
    
};

// OCL fill buffer/image command
class FillMemObject : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // ITask interface
    bool    Execute();

protected:
    FillMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);
};

// OCL migrate buffer/image command
class MigrateMemObject  :public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList);

    // ITask interface
    bool    Execute();

protected:
    MigrateMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);
};

#ifdef __INCLUDE_MKL__
// OCL Native function execution
class NativeKernelTask : public CommandBaseClass<ITask>
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();

protected:
    NativeKernelTask(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd);

    Intel::OpenCL::BuiltInKernels::IBuiltInKernel* m_pBIKernel;
};
#endif

}}}
