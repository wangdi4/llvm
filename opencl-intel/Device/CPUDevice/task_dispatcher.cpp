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
//  Scheduler.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "task_dispatcher.h"
#include "cl_logger.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

using namespace Intel::OpenCL::CPUDevice;

// Declare static members
static size_t schParamSize[] =
	{
		sizeof(cl_dev_cmd_param_rw),		//Read buffer parameters size
		sizeof(cl_dev_cmd_param_rw),		//Write buffer parameters size
		sizeof(cl_dev_cmd_param_copy),		//Copy buffer parameters size
		sizeof(cl_dev_cmd_param_map),		//Map parameters size
		sizeof(cl_dev_cmd_param_unmap),		//UnMap parameters size
		sizeof(cl_dev_cmd_param_kernel),	//Execute Kernel parameters size
		sizeof(cl_dev_cmd_param_kernel),	//Execute task parameters size
		sizeof(cl_dev_cmd_param_native)		//Execute native kernel parameters size
	};

Scheduler::TCheckCmdFunc*	Scheduler::m_checkCmdTable[] =
	{
		NULL,		//Read buffer function
		NULL,		//Write buffer function
		NULL,		//Copy buffer function
		NULL,		//Map function
		NULL,		//UnMap function
		NULL,		//Execute Kernel function
		NULL,		//Execute task function
		&Scheduler::checkNativeKernelParam		//Execute native kernel function
	};

Scheduler::TExecCmdFunc*	Scheduler::m_execCmdTable[] =
	{
		NULL,		//Read buffer function
		NULL,		//Write buffer function
		NULL,		//Copy buffer function
		NULL,		//Map function
		NULL,		//UnMap function
		NULL,		//Execute Kernel function
		NULL,		//Execute task function
		&Scheduler::execNativeKernel		//Execute native kernel function
	};

Scheduler::Scheduler(cl_int devId, cl_dev_call_backs *devCallbacks, ProgramService	*programService,
					 MemoryAllocator *memAlloc, cl_dev_log_descriptor *logDesc) :
		m_iDevId(devId), m_programService(programService), m_memoryAllocator(memAlloc),
		m_listIdAlloc(1, UINT_MAX)
{
	// Set Callbacks into the framework: Logger + Info
	if ( NULL == logDesc )
	{
		memset(&m_logDescriptor, 0, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memcpy_s(&m_logDescriptor, sizeof(cl_dev_log_descriptor), logDesc, sizeof(cl_dev_log_descriptor));
	}
	

	cl_int ret = m_logDescriptor.pfnclLogCreateClient(m_iDevId, L"CPU Device: Scheduler", &m_iLogHandle);
	if(CL_DEV_SUCCESS != ret)
	{
		//TBD
		m_iLogHandle = 0;
	}

	InfoLog(m_logDescriptor, m_iLogHandle, L"Scheduler Created");

	assert(devCallbacks);	// We assume that pointer to callback functions always must be provided
	memcpy_s(&m_frameWorkCallBacks, sizeof(cl_dev_call_backs), devCallbacks, sizeof(cl_dev_call_backs));
}



Scheduler::~Scheduler()
{
	cl_int ret = m_logDescriptor.pfnclLogReleaseClient(m_iLogHandle);
	if(CL_DEV_SUCCESS != ret)
	{
		//TBD
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
cl_int Scheduler::createCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"createCommandList enter");
	if ( NULL == list )
	{
		return CL_DEV_INVALID_VALUE;
	}

	// Allocate new handle
	unsigned int uiNewId;
	if ( !m_listIdAlloc.AllocateHandle(&uiNewId) )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	TCmdListItem* data = new TCmdListItem;
	if ( NULL == data )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	data->props = props;
	data->cmdDescList.clear();
	data->refrenceCount = 1;


	*list = (cl_dev_cmd_list)uiNewId;

	m_commandList[uiNewId] = data;

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
cl_int Scheduler::retainCommandList( cl_dev_cmd_list IN list)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"retainCommandList enter");
	// TODO: add thread safe operation to this function, Critical section
	TCmdListMap::iterator	it;
	unsigned int listId = (unsigned int)list;

	it = m_commandList.find(listId);
	if( it == m_commandList.end())
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}

	++it->second->refrenceCount;

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
cl_int Scheduler::releaseCommandList( cl_dev_cmd_list IN list )
{
	// TODO: add thread safe operation to this function, Critical section
	TCmdListMap::iterator	it;
	TCmdListItem* data;
	unsigned int listId = (unsigned int)list;

	InfoLog(m_logDescriptor, m_iLogHandle, L"releaseCommandList enter");

	it = m_commandList.find(listId);
	if( it == m_commandList.end())
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}

	data = it->second;
	if( 0 == data->refrenceCount )
	{
		return CL_DEV_INVALID_OPERATION;
	}

	//Decrements the command list refrence count
	--data->refrenceCount;
	if(data->refrenceCount > 0)
	{
		return CL_DEV_SUCCESS;
	}

	//Reference count is 0 and no items to execute -> the command list is deleted
	if ( !data->cmdDescList.empty() )
	{
		return CL_DEV_SUCCESS;
	}

	delete data;
	m_commandList.erase(it);
	
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
		CL_DEV_INVALID_OPERATION		If specific device can’t execute native kernel.
		CL_DEV_OUT_OF_RESOURCES			Is a failure to queue the execution instance of kernel because of insufficient resources
										needed to execute the kernel.
		CL_DEV_INVALID_WRK_DIM			If work_dim is not a valid value (i.e. a value between 1 and 3).
		CL_DEV_INVALID_WG_SIZE			If lcl_wrk_size is specified and number of workitems specified by glb_wrk_size is
										not evenly divisable by size of work-group given by lcl_wrk_size or does not match
										the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z)))
										qualifier in program source.
		CL_DEV_INVALID_GLB_OFFSET		If glb_wrk_offset is not (0, 0, 0).
		CL_DEV_INVALID_WRK_ITEM_SIZE	If the number of work-items specified in any of lcl_wrk_size[] is greater than the corresponding
										values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[]
********************************************************************************************************************/
cl_int Scheduler::commandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count)
{
	//The first stupid solution just execute the commands one after the other and even dont insert the commands into the list
	// TODO: add thread safe operation to this function, Critical section
	TCmdListMap::iterator	it;
	unsigned int listId = (unsigned int)list;

	InfoLog(m_logDescriptor, m_iLogHandle, L"commandListExecute enter");

	// If list id is 0, we need to create new list item
	if ( 0 == listId )
	{
		cl_dev_cmd_list	tmpList;
		cl_int	rc = createCommandList(CL_DEV_LIST_NONE, &tmpList);
		if ( CL_DEV_FAILED(rc) )
		{
			return rc;
		}
		listId = (unsigned int)tmpList;
	}

	it = m_commandList.find(listId);
	if( it == m_commandList.end())
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}

	// Test command parameters before execution
	for (unsigned int i=0; i<count; ++i)
	{
		if ( NULL == cmds[i].params )
		{
			// TODO: ADD Log
			return CL_DEV_INVALID_COMMAND_PARAM;
		}
		if ( CL_DEV_CMD_MAX_COMMAND_TYPE <= cmds[i].type )
		{
			// TODO: ADD Log
			return CL_DEV_INVALID_COMMAND_TYPE;
		}
		if ( schParamSize[cmds[i].type] != cmds[i].param_size )
		{
			// TODO: ADD Log
			return CL_DEV_INVALID_COMMAND_PARAM;
		}
		// Check if requested operation supported by the device
		// Only one table could be checked
		TCheckCmdFunc* checkFunc = m_checkCmdTable[cmds[i].type];
		if ( NULL ==  checkFunc)
		{
			// TODO: ADD Log
			return CL_DEV_INVALID_OPERATION;
		}

		// Check execution parameters
		cl_int	rc = checkFunc(cmds[i].params);
		if ( CL_DEV_FAILED(rc) )
		{
			// TODO: Add log
			return rc;
		}
	}

	// TODO: Add commands into list
	// TODO: Call Release list that was just created

	// Parameters check done -> Execute
	for (unsigned int i=0; i<count; ++i)
	{
		// Retrive appopriate execution function
		TExecCmdFunc* execFunc = m_execCmdTable[cmds[i].type];
		assert(execFunc);	// We assume that execution function always found beside the check function

		cl_int rc = execFunc(this, &cmds[i]);
		if ( CL_DEV_FAILED(rc) )
		{
			// TODO: Add log
			return rc;
		}
	}

	return CL_DEV_SUCCESS;
}

/************************************************************************************************************
*	Internal implementation of commands
*************************************************************************************************************/

//
// Native Kernel execution functions
//
cl_int	Scheduler::checkNativeKernelParam(void* params)
{
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)params;

	if( NULL == cmdParams->func_ptr )
	{
		return CL_DEV_INVALID_COMMAND_PARAM;
	}

	// Check memory object handles
	for(unsigned int i=0; i<cmdParams->mem_num; ++i )
	{
		cl_dev_mem *memObj = (cl_dev_mem*)cmdParams->mem_loc[i];
		//if ( m_memoryAllocator->
	}

	return CL_DEV_SUCCESS;
}

cl_int	Scheduler::execNativeKernel(Scheduler* _this, cl_dev_cmd_desc* cmd)
{
	cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)cmd->params;

	fn_clNativeKernel *func = (fn_clNativeKernel*)cmdParams->func_ptr;

	func(cmdParams->argv);
	//notify framework on status change
	_this->m_frameWorkCallBacks.pclDevCmdStatusChanged(cmd->id, CL_COMPLETE);

	return CL_DEV_SUCCESS;
}