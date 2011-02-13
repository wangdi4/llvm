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

#include "cl_synch_objects.h"
#include "cpu_logger.h"
#include "cpu_config.h"
#include "cl_sys_info.h"
#include "task_executor.h"
#include "task_dispatcher.h"
#include "dispatcher_commands.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::TaskExecutor;

// Constructor/Dispatcher
TaskDispatcher::TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *devCallbacks, ProgramService	*programService,
					 MemoryAllocator *memAlloc, IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig) :
		m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_pFrameworkCallBacks(devCallbacks),
		m_pProgramService(programService), m_pMemoryAllocator(memAlloc),
		m_pCPUDeviceConfig(cpuDeviceConfig), m_pWGContexts(NULL)
{
	// Set Callbacks into the framework: Logger + Info
	if ( NULL != logDesc )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, L"CPU Device: TaskDispatcher", &m_iLogHandle);
		if(CL_DEV_SUCCESS != ret)
		{
			//TBD
			m_iLogHandle = 0;
		}
	}

	m_bUseTaskalyzer = m_pCPUDeviceConfig->UseTaskalyzer();


	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("TaskDispatcher Created"));

	m_pTaskExecutor = GetTaskExecutor();

	assert(devCallbacks);	// We assume that pointer to callback functions always must be provided

	// Init Command dispatcher array
	memset(m_vCommands, 0, sizeof(m_vCommands));
	m_vCommands[CL_DEV_CMD_READ] = &ReadWriteMemObject::Create;
	m_vCommands[CL_DEV_CMD_WRITE] = &ReadWriteMemObject::Create;
	m_vCommands[CL_DEV_CMD_EXEC_KERNEL] = &NDRange::Create;
	m_vCommands[CL_DEV_CMD_EXEC_NATIVE] = &NativeFunction::Create;
	m_vCommands[CL_DEV_CMD_COPY] = &CopyMemObject::Create;
	m_vCommands[CL_DEV_CMD_MAP] = &MapMemObject::Create;
	m_vCommands[CL_DEV_CMD_UNMAP] = &UnmapMemObject::Create;

	// Init WGContexts
	// Allocate required number of working contexts
	unsigned int uiNumOfThread = TaskExecutor::GetTaskExecutor()->GetNumWorkingThreads();
	m_pWGContexts = new WGContext[uiNumOfThread];
}

TaskDispatcher::~TaskDispatcher()
{
	if ( NULL != m_pWGContexts )
	{
		delete []m_pWGContexts;
		m_pWGContexts = NULL;
	}
	if (NULL != m_pTaskExecutor)
	{
		m_pTaskExecutor->ReleasePerThreadData();
	}
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("TaskDispatcher Released"));
	if (0 != m_iLogHandle)
	{
		m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	}
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
cl_dev_err_code TaskDispatcher::createCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Enter"));
	assert( list );

	ITaskList* pList = m_pTaskExecutor->CreateTaskList(CL_DEV_LIST_ENABLE_OOO == props);
	*list = (cl_dev_cmd_list)pList;
	if ( NULL == pList )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("TaskList creation failed"), list);
		return CL_DEV_OUT_OF_MEMORY;
	}

	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - List:%X"), pList);
	return CL_DEV_SUCCESS;
}
/******************************************************************************************************************
retainCommandList
	Description
		Increments the command list reference count.
	Input
		list						A valid handle to device command list
	Output
		None
	Returns
		CL_DEV_SUCCESS				The function is executed successfully
		CL_DEV_INVALID_COMMAND_LIST	If command list is not a valid command list
*******************************************************************************************************************/
cl_dev_err_code TaskDispatcher::retainCommandList( cl_dev_cmd_list IN list)
{
	CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Not supported List:%X"), list);
	return CL_DEV_INVALID_OPERATION;		// Not support retain list
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

	ITaskList* pList = (ITaskList*)list;
	pList->Release();
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

cl_dev_err_code TaskDispatcher::commandListWaitCompletion( cl_dev_cmd_list IN list)
{
	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter - list %X"), list);

	// No need in lock
	bool waitSuccess = ((ITaskList*)list)->WaitForCompletion();

	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - list %X"), list);
	if (!waitSuccess)
	{
		return CL_DEV_NOT_SUPPORTED;
	}
	return CL_DEV_SUCCESS;
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

	ITaskList*	pList;

	pList = (ITaskList*)list;
	// If list id is 0, submit tasks directly to execution
	if ( NULL == pList )
	{
		// Create temporary list
		pList = m_pTaskExecutor->CreateTaskList();
	}

	// Lock current list for insert operations
	cl_dev_err_code ret;

	ret = SubmitTaskArray(pList, cmds, count);
	// If in place created list, release it
	if ( NULL == list )
	{
		pList->Flush();
		pList->Release();
	}

	CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Exit - List:%X"), list);
	return CL_DEV_SUCCESS;
}

//---------------------------------------------------------------------------
// Private functions
cl_dev_err_code TaskDispatcher::NotifyFailure(ITaskList* pList, cl_dev_cmd_desc* pCmd, cl_int iRetCode)
{
	CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to submit command[id:%d,type:%d] to execution, Err:<%d>"),
		pCmd->id, pCmd->type, iRetCode);

	TaskFailureNotification* pTask = new TaskFailureNotification(this, pCmd, iRetCode);
	if ( NULL == pTask )
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
		fnDispatcherCommandCreate_t*	fnCreate = m_vCommands[cmds[i]->type];

		// Create appropriate command
		ITaskBase* pCommand;
		cl_dev_err_code	rc = fnCreate(this, cmds[i], &pCommand);
		if ( CL_DEV_SUCCEEDED(rc) )
		{
			pList->Enqueue(static_cast<ITaskBase*>(pCommand));
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

void TaskDispatcher::TaskFailureNotification::Execute()
{
	cl_ulong timer = 0;

	//if profiling enabled for the command get timer
	//notify framework on status change
	if(m_pCmd->profiling)
	{
		timer = HostTime();
	}

	m_pTaskDispatcher->m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmd->id, m_pCmd->data, CL_COMPLETE, CL_DEV_ERROR_FAIL, timer);
}
