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
#include "memory_allocator.h"
#include "task_executor.h"

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher;

// Base class for handling despacther commnad execution
// All Commands will be implement this interface
class DispatcherCommand
{
public:
	DispatcherCommand(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle);
	virtual cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd) = 0;
	virtual cl_int	ExecuteCommand(cl_dev_cmd_desc* cmd) = 0;

protected:
	TaskDispatcher*				m_pTaskDispatcher;
	MemoryAllocator*			m_pMemAlloc;
	TaskExecutor*				m_pTaskExec;
	fn_clDevCmdStatusChanged*	m_pfnclDevCmdStatusChanged;
	cl_dev_log_descriptor		m_logDescriptor;
	cl_int						m_iLogHandle;
	cl_dev_cmd_desc*			m_pCmd;
};

// OCL Read/Write buffer execution
class ReadWriteBuffer : public DispatcherCommand
{
public:
	ReadWriteBuffer(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle);
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);
	cl_int	ExecuteCommand(cl_dev_cmd_desc* cmd);
protected:
	static	void	NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData);
};

// OCL Native function execution
class NativeFunction : public DispatcherCommand
{
public:
	NativeFunction(TaskDispatcher* pTD, cl_dev_log_descriptor* pLogDesc, cl_int iLogHandle);
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);
	cl_int	ExecuteCommand(cl_dev_cmd_desc* cmd);
protected:
	static	void	NotifyCommandCompletion(TTaskHandle hTask, void* pParams, size_t size, void* pData);
};

}}}