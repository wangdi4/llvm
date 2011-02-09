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
//  TaskDispatcher.h
//  Implementation of the Class TaskDispatcher
////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "program_service.h"
#include "memory_allocator.h"
#include "handle_allocator.h"
#include "task_executor.h"
#include "dispatcher_commands.h"
#include "cpu_config.h"
#include "wg_context.h"

//should be hash_map but cant compile #include <hash_map>
#include <map>
#include <list>

using namespace Intel::OpenCL::TaskExecutor;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher
{
	friend class DispatcherCommand;

public:
	TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *pDevCallbacks,
		ProgramService	*programService, MemoryAllocator *memAlloc,
		IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig);
	virtual ~TaskDispatcher();
	cl_int createCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	cl_int retainCommandList( cl_dev_cmd_list IN list);
	cl_int releaseCommandList( cl_dev_cmd_list IN list );
	cl_int flushCommandList( cl_dev_cmd_list IN list);
	cl_int commandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
	cl_int commandListWaitCompletion(cl_dev_cmd_list IN list);

protected:
	cl_int							m_iDevId;
	IOCLDevLogDescriptor*           m_pLogDescriptor;
	cl_int							m_iLogHandle;
	cl_bool							m_bUseTaskalyzer;
	IOCLFrameworkCallbacks*			m_pFrameworkCallBacks;
	ProgramService*					m_pProgramService;
	MemoryAllocator*				m_pMemoryAllocator;
	CPUDeviceConfig*				m_pCPUDeviceConfig;
	ITaskExecutor*					m_pTaskExecutor;

	// Contexts required for execution of NDRange
	WGContext*						m_pWGContexts;
	WGContext*						GetWGContext(unsigned int id)
										{ return m_pWGContexts ? &m_pWGContexts[id] : NULL; }

	// Internal implementation of functions
	fnDispatcherCommandCreate_t*		m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];

	cl_int	SubmitTaskArray(ITaskList* pList, cl_dev_cmd_desc* *cmds, cl_uint count);

	void	NotifyCommandStatusChange(const cl_dev_cmd_desc* pCmd, unsigned uStatus, int iErr);

	// Task failure notification
	class TaskFailureNotification : public ITask
	{
	public:
		TaskFailureNotification(TaskDispatcher* _this, const cl_dev_cmd_desc* pCmd, cl_int retCode) :
		  m_pTaskDispatcher(_this), m_pCmd(pCmd), m_retCode(retCode) {}

		// ITask interface
		void	Execute();
		void	Release() {delete this;}
	protected:
		TaskDispatcher*			m_pTaskDispatcher;
		const cl_dev_cmd_desc*	m_pCmd;
		cl_int					m_retCode;
	};
	cl_int NotifyFailure(ITaskList* pList, cl_dev_cmd_desc* cmd, cl_int iRetCode);
};

}}}
