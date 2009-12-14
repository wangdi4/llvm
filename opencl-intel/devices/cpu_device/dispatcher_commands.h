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
#include "program_service.h"
#include "task_executor.h"
#include "wg_context.h"

using namespace Intel::OpenCL::TaskExecutor;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher;

typedef cl_int fnDispatcherCommandCreate_t(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

// Base class for handling dispatcher command execution
// All Commands will be implement this interface
class DispatcherCommand
{
public:
	DispatcherCommand(TaskDispatcher* pTD);

protected:
	void NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr);

	TaskDispatcher*				m_pTaskDispatcher;
	MemoryAllocator*			m_pMemAlloc;
	cl_dev_log_descriptor&		m_logDescriptor;
	cl_int						m_iLogHandle;
	cl_dev_cmd_desc*			m_pCmd;
	cl_bool						m_bUseTaskalizer;
};

// OCL Read/Write buffer execution
class ReadWriteMemObject : public DispatcherCommand, public ITask
{
public:
	static cl_int Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

	// ITask interface
	void	Execute();
	void	Release() {delete this;}

protected:
	ReadWriteMemObject(TaskDispatcher* pTD);
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);
};

//OCL Copy Mem Obj Command
class CopyMemObject : public DispatcherCommand, public Intel::OpenCL::TaskExecutor::ITask
{
public:
	static cl_int Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

	// DispatcherCommand interface
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);

	// ITask interface
	void	Execute();
	void	Release() {delete this;}

protected:
	CopyMemObject(TaskDispatcher* pTD);
};

// OCL Native function execution
class NativeFunction : public DispatcherCommand, public Intel::OpenCL::TaskExecutor::ITask
{
public:
	static cl_int Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

	// DispatcherCommand interface
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);

	// ITask interface
	void	Execute();
	void	Release() {delete this;}

protected:
	NativeFunction(TaskDispatcher* pTD);

	void*				m_pArgV;
};

// OCL Map function execution
class MapMemObject : public DispatcherCommand, public Intel::OpenCL::TaskExecutor::ITask
{
public:
	static cl_int Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

	// DispatcherCommand interface
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);

	// ITask interface
	void	Execute();
	void	Release() {delete this;}

protected:
	MapMemObject(TaskDispatcher* pTD);
};

// OCL UnMap function execution
class UnmapMemObject : public DispatcherCommand, public Intel::OpenCL::TaskExecutor::ITask
{
public:
	static cl_int Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

	// DispatcherCommand interface
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);

	// ITask interface
	void	Execute();
	void	Release() {delete this;}

protected:
	UnmapMemObject(TaskDispatcher* pTD);
};

// OCL Kernel execution
class NDRange : public DispatcherCommand, public ITaskSet
{
public:
	static cl_int Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

	// DispatcherCommand interface
	cl_int	CheckCommandParams(cl_dev_cmd_desc* cmd);

	// ITaskSet interface
	int		Init(unsigned int region[]);
	int		AttachToThread(unsigned int uiWorkerId);
	void	ExecuteIteration(unsigned int x, unsigned y, unsigned int z, unsigned int uiWorkerId);
	void	Finish(FINISH_REASON reason);
	void	Release();

protected:
	NDRange(TaskDispatcher* pTD);

	cl_int						m_lastError;
	char*						m_pLockedParams;
	ICLDevBackendBinary*		m_pBinary;

	// Executable information
	size_t						m_MemBuffCount;
	size_t*						m_pMemBuffSizes;

	WGContext*					*m_pWGContexts;

	void	UnlockMemoryBuffers();
};

}}}