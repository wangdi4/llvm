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

#include "Scheduler.h"

#include<stdlib.h>

using namespace Intel::OpenCL::CPUDevice;




Scheduler::Scheduler(cl_int devId, cl_dev_call_backs *devCallbacks, ProgramService	*programService, cl_dev_log_descriptor *logDesc) :
	m_devId(devId), m_logDesc(logDesc)
{
	m_uiListId = 0;
	memcpy(&m_frameWorkCallBacks, devCallbacks, sizeof(m_frameWorkCallBacks));
	m_programService = programService;
}



Scheduler::~Scheduler()
{
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
	listData_t* data = new listData_t;
	if(NULL == data)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
	data->props = props;
	data->cmds = NULL;
	data->refrenceCount = 1;
	data->count = 0;
	*(unsigned int*)list = m_uiListId;
	m_commandList[m_uiListId] = data;
	m_uiListId++;

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
	listData_t* data;
	unsigned int listId = (unsigned int)list;
	if(listId >= m_uiListId)
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}
	data = m_commandList[listId];
	data->refrenceCount++;
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
	
	listData_t* data;
	unsigned int listId = (unsigned int)list;
	if(listId != (m_uiListId-1))
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}
	data = m_commandList[listId];
	//Decrements the command list refrence count
	data->refrenceCount--;
	if(data->refrenceCount > 0)
	{
		return CL_DEV_SUCCESS;
	}

	//Reference count is 0 command list is deleted
	delete data->cmds;
	delete data;
	
	m_commandList.erase(listId);
	m_uiListId--;
	
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
	//The first stupid solution just execute the commands one after the other and even dont insert the commands into the qeueu
	listData_t* data;
	unsigned int listId = (unsigned int)list;
	if(listId >= m_uiListId)
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}
	data = m_commandList[listId];
	unsigned int i, paramSize;
	cl_dev_cmd_desc cmd;
	
	for (i=0;i<count;i++)
	{
		cmd = cmds[i];
		if(cmd.type == CL_DEV_CMD_EXEC_NATIVE)
		{
			cl_dev_cmd_native_param *nativeCmdParam;
			paramSize = sizeof(cl_dev_cmd_native_param);
			if(cmd.params == NULL || cmd.param_size !=paramSize)
			{
				return CL_DEV_INVALID_OPERATION;
			}
			nativeCmdParam = (cl_dev_cmd_native_param*)cmd.params;
			//currently support only no params
			if(nativeCmdParam->cb_args != 0)
			{
				return CL_DEV_INVALID_OPERATION;
			}
			fn_knative_kernel_api *func = (fn_knative_kernel_api*)nativeCmdParam->func_ptr;
			func(nativeCmdParam->cb_args, nativeCmdParam->args, nativeCmdParam->args_type);
			//notify framework on status change
			m_frameWorkCallBacks.pclDevCmdStatusChanged(cmd.id, CL_COMPLETE);
			return CL_DEV_SUCCESS;
		}
	}
	return CL_DEV_INVALID_COMMAND_LIST;
}