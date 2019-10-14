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

///////////////////////////////////////////////////////////
//  task_dispatcher.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "cpu_logger.h"
#include "cpu_config.h"
#include "task_executor.h"
#include "task_dispatcher.h"
#include "dispatcher_commands.h"
#include "cl_shared_ptr.hpp"
#include <cl_synch_objects.h>
#include <cl_sys_info.h>
#include <cl_utils.h>
#if defined(USE_ITT)
#include <ocl_itt.h>
#endif

#ifdef __INCLUDE_MKL__
#include <mkl_builtins.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::Utils::SharedPtr;

class InPlaceTaskList : public ITaskList
{
public:

    PREPARE_SHARED_PTR(InPlaceTaskList)

    static SharedPtr<InPlaceTaskList> Allocate(const SharedPtr<IThreadLibTaskGroup>& ndrangeChildrenTaskGroup, bool bImmediate = true)
    {
        return SharedPtr<InPlaceTaskList>(new InPlaceTaskList(ndrangeChildrenTaskGroup, bImmediate));
    }

    virtual ~InPlaceTaskList();

    virtual unsigned int    Enqueue(const SharedPtr<ITaskBase>& pTaskBase);
    virtual bool            Flush();
    //Todo: WaitForCompletion only immediately returns if bImmediate is true
    virtual te_wait_result  WaitForCompletion(const SharedPtr<ITaskBase>& pTask) { return TE_WAIT_COMPLETED; };
    virtual void            Retain();
    virtual void            Release();
    virtual void            Cancel() {};

    virtual bool            IsMasterJoined() const { return true;}
    virtual bool            CanMasterJoin() const { return true;}
    virtual int             GetDeviceConcurency() const { return 1;}

    virtual SharedPtr<ITEDevice> GetDevice() { return nullptr; }
    virtual ConstSharedPtr<ITEDevice> GetDevice() const { return nullptr; }

    virtual SharedPtr<IThreadLibTaskGroup> GetNDRangeChildrenTaskGroup() { return m_ndrangeChildrenTaskGroup; }

    virtual void Launch(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask) { }

    bool DoesSupportDeviceSideCommandEnqueue() const { return false; }

    virtual bool IsProfilingEnabled() const { return false; }

    void Spawn(const SharedPtr<ITaskBase> &,Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup &)
    {
        assert(false && "Spawn shouldn't be called for InPlaceTaskList");
    }

protected:
    bool m_immediate;
    SharedPtr<IThreadLibTaskGroup> m_ndrangeChildrenTaskGroup;

    InPlaceTaskList(const SharedPtr<IThreadLibTaskGroup>& ndrangeChildrenTaskGroup, bool bImmediate = true);

    virtual void ExecuteInPlace(const SharedPtr<ITaskBase>& pTaskBase);
};

InPlaceTaskList::InPlaceTaskList(const SharedPtr<IThreadLibTaskGroup>& ndrangeChildrenTaskGroup, bool bImmediate) :
    m_immediate(bImmediate), m_ndrangeChildrenTaskGroup(ndrangeChildrenTaskGroup)
{
    //No support for just synchronous at the moment
    assert(m_immediate);
}

InPlaceTaskList::~InPlaceTaskList()
{
}

unsigned int InPlaceTaskList::Enqueue(const SharedPtr<ITaskBase>& pTaskBase)
{
    if (m_immediate)
    {
        ExecuteInPlace(pTaskBase);
    }
    else
    {
        //Todo: handle "execute on flush" lists
    }
    return CL_DEV_SUCCESS;
}

bool InPlaceTaskList::Flush()
{
    return true;
}

void InPlaceTaskList::Retain()
{

}

void InPlaceTaskList::Release()
{

}

void InPlaceTaskList::ExecuteInPlace(const SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>& pTaskBase)
{
    static size_t firstWGID[] = {0, 0, 0};
    assert(pTaskBase);
    if (pTaskBase->IsTaskSet())
    {
        ITaskSet* pTaskSet = static_cast<ITaskSet*>(pTaskBase.GetPtr());
        size_t dim[MAX_WORK_DIM];
        unsigned int dimCount;
        int res = pTaskSet->Init(dim, dimCount);
        if (res != 0)
        {
            pTaskSet->Finish(FINISH_INIT_FAILED);
        }
        void* local = pTaskSet->AttachToThread(this, dim[0] * dim[1] * dim[2], firstWGID, dim);
        if (nullptr == local)
        {
            pTaskSet->Finish(FINISH_INIT_FAILED);
        }
        for (size_t page = 0; page < dim[2]; ++page)
        {
            for (size_t col = 0; col < dim[1]; ++col)
            {
                for (size_t row = 0; row < dim[0]; ++row)
                {
                    pTaskSet->ExecuteIteration(row, col, page, local);
                }
            }
        }
        pTaskSet->DetachFromThread(local);
        pTaskSet->Finish(FINISH_COMPLETED);
        pTaskSet->Release();
    }
    else
    {
        ITask* pTask = static_cast<ITask*>(pTaskBase.GetPtr());
        pTask->Execute();
        pTask->Release();
    }
}

fnDispatcherCommandCreate_t* TaskDispatcher::m_vCommands[] =
{
    nullptr,
    &ReadWriteMemObject::Create,    //     CL_DEV_CMD_READ = 1,
    &ReadWriteMemObject::Create,    //    CL_DEV_CMD_WRITE,
    &CopyMemObject::Create,         //    CL_DEV_CMD_COPY,
    &MapMemObject::Create,          //    CL_DEV_CMD_MAP,
    &UnmapMemObject::Create,        //    CL_DEV_CMD_UNMAP,
    &NDRange::Create,                //    CL_DEV_CMD_EXEC_KERNEL,
    &NDRange::Create,                //    CL_DEV_CMD_EXEC_TASK,
    &NativeFunction::Create,        //    CL_DEV_CMD_EXEC_NATIVE,
    &FillMemObject::Create,         //    CL_DEV_CMD_FILL_BUFFER
    &FillMemObject::Create,         //    CL_DEV_CMD_FILL_IMAGE
    &MigrateMemObject::Create,      //    CL_DEV_CMD_MIGRATE
    &MigrateMemObject::Create       //    CL_DEV_CMD_SVM_MIGRATE
};

// Constructor/Dispatcher
TaskDispatcher::TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *devCallbacks, ProgramService    *programService,
                     MemoryAllocator *memAlloc, IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig, IAffinityChangeObserver* pObserver) :
        m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_pFrameworkCallBacks(devCallbacks),
        m_pProgramService(programService), m_pMemoryAllocator(memAlloc),
        m_pCPUDeviceConfig(cpuDeviceConfig), m_uiNumThreads(0),
        m_bTEActivated(false), m_pObserver(pObserver)
#if defined(__INCLUDE_MKL__) && !defined(__OMP2TBB__)
        ,m_pOMPExecutionThread(nullptr)
#endif
{
    // Set Callbacks into the framework: Logger + Info
    if ( nullptr != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, "CPU Device: TaskDispatcher", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            //TBD
            m_iLogHandle = 0;
        }
    }

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("TaskDispatcher Created"));

    m_pTaskExecutor = m_pFrameworkCallBacks->clDevGetTaskExecutor();
    m_pGPAData = m_pTaskExecutor->GetGPAData();

    assert(devCallbacks);    // We assume that pointer to callback functions always must be provided
}

TaskDispatcher::~TaskDispatcher()
{
#if defined(__INCLUDE_MKL__) && !defined(__OMP2TBB__)
    m_pOMPExecutionThread->Join();
    delete m_pOMPExecutionThread;
#endif

    if (m_bTEActivated)
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "m_pTaskExecutor->Deactivate();");
        if ((0 != m_pTaskExecutor) && (0 != m_pRootDevice ))
        {
            m_pRootDevice->ShutDown();
            m_pRootDevice->ResetObserver();
            m_pRootDevice = nullptr;
            m_pTaskExecutor->DestroyDebugDeviceQueue();
        }
    }
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("TaskDispatcher Released"));
    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
}

cl_dev_err_code TaskDispatcher::init()
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "m_pTaskExecutor->Activate();");
    // create root device in flat mode with maximum threads, support for
    // masters joining and one reserved position for master in device
    auto TaskExecutorMastersJoinMode = TE_ENABLE_MASTERS_JOIN;

    // disable masters joining because kernels can contain infinite loops
    if (FPGA_EMU_DEVICE == m_pCPUDeviceConfig->GetDeviceMode())
        TaskExecutorMastersJoinMode = TE_DISABLE_MASTERS_JOIN;

    const size_t numMasters = 1;
    m_pRootDevice = m_pTaskExecutor->CreateRootDevice(
        RootDeviceCreationParam(TE_AUTO_THREADS, TaskExecutorMastersJoinMode,
                                numMasters), nullptr, this);

    m_bTEActivated = (0 != m_pRootDevice);
    if ( !m_bTEActivated )
    {
        return CL_DEV_ERROR_FAIL;
    }

    unsigned int uiNumThreads = m_pRootDevice->GetConcurrency();

    // Init WGContexts
    // Allocate required number of working contexts
    if ( 0 == m_uiNumThreads )
    {
        m_uiNumThreads = uiNumThreads;
    }

#if defined(__INCLUDE_MKL__) && !defined(__OMP2TBB__)
    m_pOMPExecutionThread = Intel::OpenCL::BuiltInKernels::OMPExecutorThread::Create(m_uiNumThreads);
    if ( nullptr == m_pOMPExecutionThread )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    m_pOMPExecutionThread->Start();
#endif

    m_pTaskExecutor->CreateDebugDeviceQueue(m_pRootDevice);

    return CL_DEV_SUCCESS;
}

bool TaskDispatcher::isDestributedAllocationRequired()
{
    return clIsNumaAvailable();
}

bool TaskDispatcher::isThreadAffinityRequired()
{
#if ( defined(WIN32) ) // Not pinning threads for Windows
    return false;
#else
    return true;
#endif
}

cl_dev_err_code TaskDispatcher::SetDefaultCommandList(const SharedPtr<ITaskList> IN list)
{
    m_pDefaultQueue = list;
    return CL_DEV_SUCCESS;
}
/*******************************************************************************************************************
createCommandList
    Description
        This function creates a dependent command list on a device. This function performs an implicit retain of the command list.
    Input
        props                        Determines whether the out-of-order optimization could be applied on items in the command list.
    Output
        list                        A valid handle to device command list, on error the value is NULL
    Returns
        CL_DEV_SUCCESS                The command queue successfully created
        CL_DEV_INVALID_VALUE        If values specified in properties are not valid
        CL_DEV_INVALID_PROPERTIES    If values specified in properties are valid but are not supported by the device
        CL_DEV_OUT_OF_MEMORY        If there is a failure to allocate resources required by the OCL device driver
**************************************************************************************************************************/
cl_dev_err_code TaskDispatcher::createCommandList( cl_dev_cmd_list_props IN props, ITEDevice* IN pDevice, SharedPtr<ITaskList>* OUT list)
{
    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Enter"));

    assert(nullptr != list && "Called with null pointer as output");

    // NULL device, meaning submit to Root device
    if (nullptr == pDevice)
    {
        pDevice = m_pRootDevice.GetPtr();
    }

    bool isInPlace   = (0 != ((int)props & (int)CL_DEV_LIST_IN_PLACE));
    if (!isInPlace)
    {
        bool isOOO       = (0 != ((int)props & (int)CL_DEV_LIST_ENABLE_OOO));
        *list = pDevice->CreateTaskList(CommandListCreationParam(
                                            isOOO ? TE_CMD_LIST_OUT_OF_ORDER : TE_CMD_LIST_IN_ORDER,
                                            TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC,
                                            props & CL_DEV_LIST_PROFILING, props & CL_DEV_LIST_QUEUE_DEFAULT)
                                        );
        if (props & CL_DEV_LIST_QUEUE_DEFAULT)
        {
            m_pDefaultQueue = *list;
        }
    }
    else
    {
        //Todo: handle non-immediate lists
        *list = InPlaceTaskList::Allocate(GetTaskExecutor()->CreateTaskGroup(pDevice));
    }
    if ( 0 == *list )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("TaskList creation failed"), list->GetPtr());
        return CL_DEV_OUT_OF_MEMORY;
    }

    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - List:%X"), list->GetPtr());
    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
commandListExecute
    Description
        Passes a list of dependent commands into a specified command list for execution.
        The commands are depended by the list index: item[n] depends on item[n-1].
        First item (item[0]) is dependent on the last item that was passed during previous  function call on with same list identifier.
    Input
        list                            A valid handle to device command list, where to add list of commands. If value is NULL,
                                        the new independent list is created for given commands
        cmds                            A vector of dependent commands, each entry is described by cl_dev_cmd_desc structure
        count                            Number of entries in cmds parameter
    Output
        None
     Returns
        CL_DEV_SUCCESS                    The function is executed successfully.
        CL_DEV_INVALID_COMMAND_LIST        If command list is not a valid command list
        CL_DEV_INVALID_COMMAND_TYPE        If command type specified in one of the cmds entries is not a valid command.
        CL_DEV_INVALID_COMMAND_PARAM    If one of the parameters submitted within command structure is invalid.
        CL_DEV_INVALID_MEM_OBJECT        If one or more memory objects specified in parameters in one or more of cmds entries
                                        are not valid or are not buffer objects.
        CL_DEV_INVALID_KERNEL            If kernel identifier specified in execution parameters is not valid.
        CL_DEV_INVALID_OPERATION        If specific device cannot execute native kernel.
        CL_DEV_OUT_OF_RESOURCES            Is a failure to queue the execution instance of kernel because of insufficient resources
                                        needed to execute the kernel.
        CL_DEV_INVALID_WRK_DIM            If work_dim is not a valid value (i.e. a value between 1 and 3).
        CL_DEV_INVALID_WG_SIZE            If lcl_wrk_size is specified and number of work items specified by glb_wrk_size is
                                        not evenly divisable by size of work-group given by lcl_wrk_size or does not match
                                        the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z)))
                                        qualifier in program source.
        CL_DEV_INVALID_GLB_OFFSET        If glb_wrk_offset is not (0, 0, 0).
        CL_DEV_INVALID_WRK_ITEM_SIZE    If the number of work-items specified in any of lcl_wrk_size[] is greater than the corresponding
                                        values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[]
********************************************************************************************************************/
cl_dev_err_code TaskDispatcher::commandListExecute( const SharedPtr<ITaskList>& IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter - List:%X"), list.GetPtr());

    ITaskList* pList = list.GetPtr();

    cl_dev_err_code ret = CL_DEV_SUCCESS;
    // If list id is 0, submit tasks directly to execution
    if ( nullptr != pList )
    {
        ret = SubmitTaskArray(pList, cmds, count);
    }
    else
    {
      // Create temporary list
        SharedPtr<ITaskList> pLocalList;
        pLocalList = m_pRootDevice->CreateTaskList(TE_CMD_LIST_IN_ORDER );
        ret = SubmitTaskArray(pLocalList.GetPtr(), cmds, count);
        pLocalList->Flush();
    }

    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - List:%X"), list.GetPtr());
    return ret;
}

//---------------------------------------------------------------------------
// Private functions

cl_dev_err_code TaskDispatcher::NotifyFailure(ITaskList* pList, cl_dev_cmd_desc* pCmd, cl_int iRetCode)
{
    CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to submit command[id:%d,type:%d] to execution, Err:<%d>"), pCmd->id, pCmd->type, iRetCode);

    SharedPtr<ITaskBase> pTask = TaskFailureNotification::Allocate(this, pCmd, iRetCode);
    if ( 0 == pTask )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    pList->Enqueue(pTask);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code TaskDispatcher::SubmitTaskArray(ITaskList* pList, cl_dev_cmd_desc* *cmds, cl_uint count)
{
    for (unsigned int i=0; i<count; ++i)
    {
        assert(cmds[i]->type < CL_DEV_CMD_MAX_COMMAND_TYPE);
        fnDispatcherCommandCreate_t*    fnCreate = m_vCommands[cmds[i]->type];

        // Create appropriate command
        SharedPtr<ITaskBase> pCommand;
        cl_dev_err_code    rc = fnCreate(this, cmds[i], &pCommand, pList);
        if ( CL_DEV_SUCCEEDED(rc) )
        {
            pCommand->IncRefCnt();
            cmds[i]->device_agent_data = static_cast<ITaskBase*>(pCommand.GetPtr());
            pList->Enqueue(SharedPtr<ITaskBase>(pCommand));
        }
        else
        {
            // Try to notify about the error in the same list
            cmds[i]->device_agent_data = nullptr;
            rc = NotifyFailure(pList, cmds[i], rc);
            if ( CL_DEV_FAILED(rc) )
            {
                return rc;
            }
        }
    }

    return CL_DEV_SUCCESS;
}

void TaskDispatcher::NotifyCommandStatusChange(const cl_dev_cmd_desc* pCmd, unsigned uStatus, int iErr)
{
    cl_ulong timer = 0;

    //if profiling enabled for the command get timer
    //notify framework on status change
    if(pCmd->profiling)
    {
        timer = HostTime();
    }
    m_pFrameworkCallBacks->clDevCmdStatusChanged(pCmd->id, pCmd->data, uStatus, iErr, timer);
}

bool TaskDispatcher::TaskFailureNotification::Shoot(cl_dev_err_code err)
{
    cl_ulong timer = 0;

    //if profiling enabled for the command get timer
    //notify framework on status change
    if(m_pCmd->profiling)
    {
        timer = HostTime();
    }

    m_pTaskDispatcher->m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmd->id, m_pCmd->data, CL_COMPLETE, (cl_int)err, timer);
    return true;
}

void* TaskDispatcher::OnThreadEntry()
{
    const unsigned int position_in_device = m_pTaskExecutor->GetPosition();

    if ( !m_pTaskExecutor->IsMaster() )
    {
        // We don't affinitize application threads
        if ( isThreadAffinityRequired() )
        {
            // Only enter if affinity, in general, is required (OS-dependent)

            //We notify only for sub-devices by NAMES - in other cases, the user is not interested which cores to use
            cl_dev_internal_subdevice_id* pSubDevID = reinterpret_cast<cl_dev_internal_subdevice_id*>(m_pTaskExecutor->GetCurrentDevice().user_handle);

            if (nullptr != pSubDevID && nullptr != pSubDevID->legal_core_ids)
            {
                assert((nullptr != pSubDevID->legal_core_ids) && "For BY NAMES there should be an allocated array of legal core indices");
                m_pObserver->NotifyAffinity( clMyThreadId(), pSubDevID->legal_core_ids[position_in_device] );
            }
            else
                m_pObserver->NotifyAffinity(clMyThreadId(), position_in_device);
        }

        if (isDestributedAllocationRequired())
        {
            // Set NUMA node
            clNUMASetLocalNodeAlloc();
        }
    }

    // TODO: What should be return here instead, we can't return NULL
    return m_pTaskExecutor;
}

void  TaskDispatcher::OnThreadExit( void* currentThreadData )
{
}

TE_BOOLEAN_ANSWER TaskDispatcher::MayThreadLeaveDevice( void* currentThreadData )
{
#ifdef __SOFT_TRAPPING__ // Moving to "hard" trapping
    cl_dev_internal_subdevice_id* pSubDevID = reinterpret_cast<cl_dev_internal_subdevice_id*>(m_pTaskExecutor->GetCurrentDevice().user_handle);
    if ( (nullptr!=pSubDevID) && (nullptr!=pSubDevID->legal_core_ids) )
    {
        return pSubDevID->is_acquired ? TE_NO : TE_YES;
    }
#endif
    return TE_USE_DEFAULT;
}
