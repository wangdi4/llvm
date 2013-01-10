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

#include "stdafx.h"
#include "cpu_logger.h"
#include "cpu_config.h"
#include "task_executor.h"
#include "task_dispatcher.h"
#include "dispatcher_commands.h"
#include "cl_shared_ptr.hpp"
#include <cl_synch_objects.h>
#include <cl_sys_info.h>
#include <cl_utils.h>
#include <ocl_itt.h>

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#define SUB_DEVICE_SYNC_COMMAND ((void*)0x1)

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::Utils::SharedPtr;

class InPlaceTaskList : public ITaskList
{
public:

    PREPARE_SHARED_PTR(InPlaceTaskList)

    static SharedPtr<InPlaceTaskList> Allocate(WGContextBase* pMasterWGContext, bool bImmediate = true)
    {
        return SharedPtr<InPlaceTaskList>(new InPlaceTaskList(pMasterWGContext, bImmediate));
    }
	
	virtual ~InPlaceTaskList();

	virtual unsigned int	Enqueue(const SharedPtr<ITaskBase>& pTaskBase);
	virtual bool			Flush();
	//Todo: WaitForCompletion only immediately returns if bImmediate is true
	virtual te_wait_result  WaitForCompletion(const SharedPtr<ITaskBase>& pTask) { return TE_WAIT_COMPLETED; };
    virtual void            Retain();
	virtual void			Release();
    std::string GetTypeName() const { return "InPlaceTaskList"; }

protected:
	WGContextBase* const m_pMasterWGContext;
    bool m_immediate;    

    InPlaceTaskList(WGContextBase* pMasterWGContext, bool bImmediate = true);

    virtual void ExecuteInPlace(const SharedPtr<ITaskBase>& pTaskBase);
};

InPlaceTaskList::InPlaceTaskList(WGContextBase* pMasterWGContext, bool bImmediate) : m_pMasterWGContext(pMasterWGContext), m_immediate(bImmediate)
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
        res = pTaskSet->AttachToThread(m_pMasterWGContext, dim[0] * dim[1] * dim[2], firstWGID, dim);
        if (res != 0)
        {
            pTaskSet->Finish(FINISH_INIT_FAILED);
        }
        pTaskSet->ExecuteAllIterations(dim, m_pMasterWGContext);
        pTaskSet->DetachFromThread(m_pMasterWGContext);
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
	NULL,
	&ReadWriteMemObject::Create,    // 	CL_DEV_CMD_READ = 1,
	&ReadWriteMemObject::Create,    //	CL_DEV_CMD_WRITE,
	&CopyMemObject::Create,         //	CL_DEV_CMD_COPY,
	&MapMemObject::Create,          //	CL_DEV_CMD_MAP,
	&UnmapMemObject::Create,        //	CL_DEV_CMD_UNMAP,
	&NDRange::Create,               //	CL_DEV_CMD_EXEC_KERNEL,
	&NDRange::Create,               //	CL_DEV_CMD_EXEC_TASK,
	&NativeFunction::Create,        //	CL_DEV_CMD_EXEC_NATIVE,
	&FillMemObject::Create,         //	CL_DEV_CMD_FILL_BUFFER
	&FillMemObject::Create,         //	CL_DEV_CMD_FILL_IMAGE
	&MigrateMemObject::Create       //  CL_DEV_CMD_MIGRATE
};

// Constructor/Dispatcher
TaskDispatcher::TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *devCallbacks, ProgramService	*programService,
					 MemoryAllocator *memAlloc, IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig, IAffinityChangeObserver* pObserver) :
		m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_pFrameworkCallBacks(devCallbacks),
		m_pProgramService(programService), m_pMemoryAllocator(memAlloc),
		m_pCPUDeviceConfig(cpuDeviceConfig), m_uiNumThreads(0), m_bTEActivated(false), m_pWGContexts(NULL), m_pObserver(pObserver), m_pAffinityPermutation(NULL)
{
	// Set Callbacks into the framework: Logger + Info
	if ( NULL != logDesc )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, "CPU Device: TaskDispatcher", &m_iLogHandle);
		if(CL_DEV_SUCCESS != ret)
		{
			//TBD
			m_iLogHandle = 0;
		}
	}

	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("TaskDispatcher Created"));
	
	m_pTaskExecutor = GetTaskExecutor();	
	m_pGPAData = m_pTaskExecutor->GetGPAData();

	assert(devCallbacks);	// We assume that pointer to callback functions always must be provided
}

TaskDispatcher::~TaskDispatcher()
{
	if ( NULL != m_pWGContexts )
	{
		delete []m_pWGContexts;
		m_pWGContexts = NULL;
	}    
	if (m_bTEActivated)
	{	
		CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "TaskExecutor::GetTaskExecutor()->Deactivate();");
        if (NULL != TaskExecutor::GetTaskExecutor())
        {
		    TaskExecutor::GetTaskExecutor()->Deactivate();
        }
	}
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("TaskDispatcher Released"));
	if (0 != m_iLogHandle)
	{
		m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	}
	if (NULL != m_pAffinityPermutation)
	{
		delete[] m_pAffinityPermutation;
	}
}

cl_dev_err_code TaskDispatcher::init()
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "m_pTaskExecutor->Activate();");
	m_bTEActivated = m_pTaskExecutor->Activate();
	if ( !m_bTEActivated )
	{
		return CL_DEV_ERROR_FAIL;
	}

	unsigned int uiNumThreads = m_pTaskExecutor->GetNumWorkingThreads();
	// Init WGContexts
	// Allocate required number of working contexts
	if ( 0 == m_uiNumThreads )
	{
		m_uiNumThreads = uiNumThreads;
	}
	
    // master thread(s) get their context from their TLS
    // WorkerIds are not realocated for Sub-Device,
    // Therefore, need allocate full size array, even for SubDevices.
	m_pWGContexts = new WGContext[uiNumThreads];
	if ( NULL == m_pWGContexts )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	bool bInitTasksRequired = isDestributedAllocationRequried() || isThreadAffinityRequried();
	
	if (!bInitTasksRequired)
	{
		// On linux we should initialize contexts
		// On Windows it should be done by the pinning threads
		// Otherwise, init on Affinize threads
		if ( NULL != m_pWGContexts )
		{
			for(cl_uint i =0; i < m_uiNumThreads; ++i)
			{
				m_pWGContexts[i].Init();
			}
		}
		return CL_DEV_SUCCESS;
	}

	//Pin threads
    SharedPtr<ITaskSet> pAffinitizeThreads = AffinitizeThreads::Allocate(m_uiNumThreads, 0, m_pObserver, this);
	if (NULL == pAffinitizeThreads)
	{
		//Todo
		assert(0);
		return CL_DEV_OUT_OF_MEMORY;
	}
    m_pTaskExecutor->Execute(SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>(pAffinitizeThreads), NULL);
	m_pTaskExecutor->WaitForCompletion(NULL, NULL);
	
	return CL_DEV_SUCCESS;
}

bool TaskDispatcher::isDestributedAllocationRequried()
{
	return clIsNumaAvailable();
}

bool TaskDispatcher::isThreadAffinityRequried()
{
#ifdef WIN32 //Not pinning threads for Windows
	return false;
#else
	return true;
#endif
}

/*******************************************************************************************************************
createCommandList
	Description
		This function creates a dependent command list on a device. This function performs an implicit retain of the command list.
	Input
		props						Determines whether the out-of-order optimization could be applied on items in the command list.
	Output
		list						A valid handle to device command list, on error the value is NULL
	Returns
		CL_DEV_SUCCESS				The command queue successfully created
		CL_DEV_INVALID_VALUE		If values specified in properties are not valid
		CL_DEV_INVALID_PROPERTIES	If values specified in properties are valid but are not supported by the device
		CL_DEV_OUT_OF_MEMORY		If there is a failure to allocate resources required by the OCL device driver
**************************************************************************************************************************/
cl_dev_err_code TaskDispatcher::createCommandList( cl_dev_cmd_list_props IN props, void* IN pSubdevTaskExecData, void** OUT list)
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Enter"));
	assert( list );
    SharedPtr<ITaskList> pList = NULL;

    bool isInPlace   = (0 != ((int)props & (int)CL_DEV_LIST_IN_PLACE));
    if (!isInPlace)
    {
        bool isSubdevice = (0 != ((int)props & (int)CL_DEV_LIST_SUBDEVICE));
        bool isOOO       = (0 != ((int)props & (int)CL_DEV_LIST_ENABLE_OOO));
        CommandListCreationParam p;
        p.isOOO       = isOOO;
        p.isSubdevice = isSubdevice;
	    pList = m_pTaskExecutor->CreateTaskList(&p, pSubdevTaskExecData);
    }
    else
    {
        //Todo: handle non-immediate lists
        pList = InPlaceTaskList::Allocate(m_pTaskExecutor->GetWGContext(true));
    }
	*list = pList.GetPtr();    
	if ( NULL == pList )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("TaskList creation failed"), list);
		return CL_DEV_OUT_OF_MEMORY;
	}
    pList.IncRefCnt();

	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - List:%X"), pList.GetPtr());
	return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
releaseCommandList
	Description
		Decrements the command list reference count. After the command list reference count becomes zero and
		all commands of the command list have completed (eg. Kernel executions, memory object updates etc.),
		the command queue is deleted.
	Input
		list						A valid handle to device command list
	Output
		None
	Returns
		CL_DEV_SUCCESS				The function is executed successfully
		CL_DEV_INVALID_COMMAND_LIST	If command list is not a valid command list
********************************************************************************************************************/
cl_dev_err_code TaskDispatcher::releaseCommandList( cl_dev_cmd_list IN list )
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter - list %X"), list);

	SharedPtr<ITaskList> pList = (ITaskList*)list;
    pList.DecRefCnt();
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - list %X"), list);
	return CL_DEV_SUCCESS;
}

/******************************************************************************************************************
flushCommandList
	Description
		This function flushes the content of a list, all waiting commands are sent to execution.
	Input
		list						A valid handle to device command list
	Output
		None
	Returns
		CL_DEV_SUCCESS				The function is executed successfully
		CL_DEV_INVALID_COMMAND_LIST	If command list is not a valid command list
*******************************************************************************************************************/
cl_dev_err_code TaskDispatcher::flushCommandList( cl_dev_cmd_list IN list)
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter - list %X"), list);
	// No need in lock
	((ITaskList*)list)->Flush();
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - list %X"), list);
	return CL_DEV_SUCCESS;
}

cl_dev_err_code TaskDispatcher::commandListWaitCompletion( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait)
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter - list %X"), list);

	SharedPtr<ITaskBase> pTaskToWait = NULL;
	if ( NULL != cmdToWait )
	{
		// At this stage we assume that cmdToWait is a valid pointer
		// Appropriate reference count is done in runtime
		void* pTaskPtr = cmdToWait->device_agent_data;
		// Check if the command is already completed but it can be just before a call to the notification function.
		// and we are not sure that RT got the completion notification.
		// Therefore, we MUST return error value and RT will take appropriate action in order to monitor event status
		if ( NULL == pTaskPtr )
		{
			return CL_DEV_NOT_SUPPORTED;
		}
		pTaskToWait = static_cast<ITaskBase*>(pTaskPtr);
	}

	// No need in lock
	te_wait_result res = ((ITaskList*)list)->WaitForCompletion(pTaskToWait);

	cl_dev_err_code retVal;
	if ( NULL != pTaskToWait )
	{
		// Try to wait for command
		if ( (!pTaskToWait->IsCompleted() && (TE_WAIT_COMPLETED == res)) || TE_WAIT_NOT_SUPPORTED == res)
		{
			((ITaskList*)list)->Flush();
			res = TE_WAIT_MASTER_THREAD_BLOCKING;
		}
		pTaskToWait->Release();
		// If the task is not completed at this stage we can't make further call to blocking wait
		// becaue we are not having the task pointer and we can't set it back because its not thread safe
		retVal = (TE_WAIT_COMPLETED == res) ? CL_DEV_SUCCESS : CL_DEV_NOT_SUPPORTED;
	}
	else
	{
		retVal = (TE_WAIT_COMPLETED == res) ? CL_DEV_SUCCESS :
				(TE_WAIT_MASTER_THREAD_BLOCKING == res) ? CL_DEV_BUSY : CL_DEV_NOT_SUPPORTED;
	}

	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - list %X, res = %d"), list, retVal);
	return retVal;
}

/********************************************************************************************************************
commandListExecute
	Description
		Passes a list of dependent commands into a specified command list for execution.
		The commands are depended by the list index: item[n] depends on item[n-1].
		First item (item[0]) is dependent on the last item that was passed during previous  function call on with same list identifier.
	Input
		list							A valid handle to device command list, where to add list of commands. If value is NULL,
										the new independent list is created for given commands
		cmds							A vector of dependent commands, each entry is described by cl_dev_cmd_desc structure
		count							Number of entries in cmds parameter
	Output
		None
	 Returns
		CL_DEV_SUCCESS					The function is executed successfully.
		CL_DEV_INVALID_COMMAND_LIST		If command list is not a valid command list
		CL_DEV_INVALID_COMMAND_TYPE		If command type specified in one of the cmds entries is not a valid command.
		CL_DEV_INVALID_COMMAND_PARAM	If one of the parameters submitted within command structure is invalid.
		CL_DEV_INVALID_MEM_OBJECT		If one or more memory objects specified in parameters in one or more of cmds entries
										are not valid or are not buffer objects.
		CL_DEV_INVALID_KERNEL			If kernel identifier specified in execution parameters is not valid.
		CL_DEV_INVALID_OPERATION		If specific device cannot execute native kernel.
		CL_DEV_OUT_OF_RESOURCES			Is a failure to queue the execution instance of kernel because of insufficient resources
										needed to execute the kernel.
		CL_DEV_INVALID_WRK_DIM			If work_dim is not a valid value (i.e. a value between 1 and 3).
		CL_DEV_INVALID_WG_SIZE			If lcl_wrk_size is specified and number of work items specified by glb_wrk_size is
										not evenly divisable by size of work-group given by lcl_wrk_size or does not match
										the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z)))
										qualifier in program source.
		CL_DEV_INVALID_GLB_OFFSET		If glb_wrk_offset is not (0, 0, 0).
		CL_DEV_INVALID_WRK_ITEM_SIZE	If the number of work-items specified in any of lcl_wrk_size[] is greater than the corresponding
										values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[]
********************************************************************************************************************/
cl_dev_err_code TaskDispatcher::commandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter - List:%X"), list);

	SharedPtr<ITaskList> pList = (ITaskList*)list;
	// If list id is 0, submit tasks directly to execution
	if ( NULL == pList )
	{
        CommandListCreationParam p;
        p.isOOO = false;
        p.isSubdevice = false;
		// Create temporary list
		pList = m_pTaskExecutor->CreateTaskList(&p, NULL);
	}

	// Lock current list for insert operations
	cl_dev_err_code ret;

	ret = SubmitTaskArray(pList, cmds, count);
	// If in place created list, release it
	if ( NULL == list )
	{
		pList->Flush();
	}

	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - List:%X"), list);
	return CL_DEV_SUCCESS;
}

//---------------------------------------------------------------------------
// Private functions

cl_dev_err_code TaskDispatcher::NotifyFailure(SharedPtr<ITaskList> pList, cl_dev_cmd_desc* pCmd, cl_int iRetCode)
{
	CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to submit command[id:%d,type:%d] to execution, Err:<%d>"),
		pCmd->id, pCmd->type, iRetCode);

    SharedPtr<TaskFailureNotification> pTask = TaskFailureNotification::Allocate(this, pCmd, iRetCode);
	if ( NULL == pTask )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	pList->Enqueue(SharedPtr<ITaskBase>(pTask));

	return CL_DEV_SUCCESS;
}

cl_dev_err_code TaskDispatcher::SubmitTaskArray(SharedPtr<ITaskList> pList, cl_dev_cmd_desc* *cmds, cl_uint count)
{
	for (unsigned int i=0; i<count; ++i)
	{
		assert(cmds[i]->type < CL_DEV_CMD_MAX_COMMAND_TYPE);
		fnDispatcherCommandCreate_t*	fnCreate = m_vCommands[cmds[i]->type];

		// Create appropriate command
		SharedPtr<ITaskBase> pCommand;
		cl_dev_err_code	rc = fnCreate(this, cmds[i], &pCommand);        
		if ( CL_DEV_SUCCEEDED(rc) )
		{
            pCommand.IncRefCnt();   // since the device commands are stored as void*, we need to manually increment their reference counter here (DecRefCnt is called in CPUDevice::clDevReleaseCommand)
			pList->Enqueue(SharedPtr<ITaskBase>(pCommand));
		} else
		{
			// Try to notify about the error in the same list
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

bool TaskDispatcher::TaskFailureNotification::Execute()
{
	cl_ulong timer = 0;

	//if profiling enabled for the command get timer
	//notify framework on status change
	if(m_pCmd->profiling)
	{
		timer = HostTime();
	}

	m_pTaskDispatcher->m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmd->id, m_pCmd->data, CL_COMPLETE, (cl_int)CL_DEV_ERROR_FAIL, timer);
	return true;
}

void* TaskDispatcher::createSubdevice(unsigned int uiNumSubdevComputeUnits, const unsigned int* pLegalCores, IAffinityChangeObserver& observer)
{
    return m_pTaskExecutor->CreateSubdevice(uiNumSubdevComputeUnits, pLegalCores, observer);
}

void TaskDispatcher::waitUntilEmpty(void* pSubdevData)
{
    m_pTaskExecutor->WaitUntilEmpty(pSubdevData);
}

void TaskDispatcher::releaseSubdevice(void* pSubdevData)
{
    m_pTaskExecutor->ReleaseSubdevice(pSubdevData);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SubdeviceTaskDispatcher::SubdeviceTaskDispatcher(int numThreads, unsigned int* legalCoreIDs, cl_int devId, IOCLFrameworkCallbacks *pDevCallbacks, ProgramService *programService,
    MemoryAllocator *memAlloc, IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig, IAffinityChangeObserver* pObserver, void* pSubdevTaskExecData) 
	: TaskDispatcher(devId, pDevCallbacks, programService, memAlloc, logDesc, cpuDeviceConfig, pObserver),
	m_legalCoreIDs(legalCoreIDs), m_pSubdevTaskExecData(pSubdevTaskExecData)
{
	m_uiRefCount = 2; // We start with reference count of two
					 // The first item is for cpu device level release
					 // The second item is for sub-device worker release
	m_uiNumThreads = numThreads;
	m_evWaitForCompletion.Init(true); // Auto-reset event
}

SubdeviceTaskDispatcher::~SubdeviceTaskDispatcher()
{
}

void SubdeviceTaskDispatcher::Release()
{
	// Must wait until SubDevice worker thread finishes shutdown sequence.
	// Use thread reentered mutex, to be able to not to wait while shuting down from the same thread
	m_muDestructReady.Lock();
	// Check if this call should destroy the task dispatcher
	bool bDeleteThis = ( (m_uiRefCount)-- == 1);
	m_muDestructReady.Unlock();

	if ( bDeleteThis )
	{
		delete this;
	}
}

cl_dev_err_code SubdeviceTaskDispatcher::init()
{
	return CL_DEV_SUCCESS;
}

cl_dev_err_code SubdeviceTaskDispatcher::createCommandList(cl_dev_cmd_list_props props, void* IN pSubdevTaskExecData, cl_dev_cmd_list *list)
{
	props = (cl_dev_cmd_list_props)((unsigned int)props | (unsigned int)CL_DEV_LIST_SUBDEVICE);
	return TaskDispatcher::createCommandList(props, pSubdevTaskExecData, list);
}

cl_dev_err_code SubdeviceTaskDispatcher::internalFlushCommandList(cl_dev_cmd_list IN list)
{
    cl_dev_err_code err = TaskDispatcher::flushCommandList(list);
    releaseCommandList(list);
    return err;
}

bool SubdeviceTaskDispatcher::isThreadAffinityRequried()
{
	// Threads already affinitized
	return false;
}

bool SubdeviceTaskDispatcher::isDestributedAllocationRequried()
{
	// Always partially allocate resources
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AffinitizeThreads::AffinitizeThreads(unsigned int numThreads, cl_ulong timeOutInTicks, IAffinityChangeObserver* pObserver, TaskDispatcher* pTD) :
	m_numThreads(numThreads), m_timeOut(timeOutInTicks), m_pObserver(pObserver), m_pTD(pTD), m_failed(false)
{
}

AffinitizeThreads::~AffinitizeThreads()
{
}

int AffinitizeThreads::Init(size_t region[], unsigned int &dimCount)
{
	// copy execution parameters
	unsigned int i;
	for (i = 1; i < MAX_WORK_DIM; ++i)
	{
		region[i] = 1;
	}
	dimCount = 1;
	region[0] = m_numThreads;
	m_barrier = m_numThreads;

	return CL_DEV_SUCCESS;
}

int AffinitizeThreads::AttachToThread(WGContextBase* pWgContext, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[])
{
	return CL_DEV_SUCCESS;
}

int AffinitizeThreads::DetachFromThread(WGContextBase* pWgContext)
{
	return CL_DEV_SUCCESS;
}

void AffinitizeThreads::ExecuteIteration(size_t x, size_t y, size_t z, WGContextBase* pWgContext)
{
	if (m_failed)
	{
		return;
	}
	
    WGContext* pContext = (WGContext*)pWgContext;
    assert (NULL!= pContext);
    if (!pContext->DoesBelongToMasterThread())
	{
		if ( m_pTD->isThreadAffinityRequried() )
		{
            m_pObserver->NotifyAffinity(pContext->GetThreadId(), Intel::OpenCL::Utils::GetCpuId());
		}
		
		// Set NUMA node prior to allocation
		clNUMASetLocalNodeAlloc();
		pContext->Init();
	}		

	m_barrier--;
	cl_ulong start = Intel::OpenCL::Utils::HostTime();
	while ((m_barrier > 0) && (!m_failed))
	{
		if (m_timeOut > 0)
		{
			cl_ulong now = Intel::OpenCL::Utils::HostTime();
			if (now - start > m_timeOut)
			{
				m_failed = true;
				return;
			}
		}
		clSleep(0);
	}
}
