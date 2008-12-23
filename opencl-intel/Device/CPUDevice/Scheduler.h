
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
//  Scheduler.h
//  Implementation of the Class Scheduler
////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "ProgramService.h"
#include "MemoryAllocator.h"
#include "HandleAllocator.h"

//should be hash_map but cant compile #include <hash_map>
#include <map>
#include <list>
using namespace std;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class Scheduler
{

public:
	Scheduler(cl_int devId, cl_dev_call_backs *devCallbacks,
		ProgramService	*programService, MemoryAllocator *memAlloc,
		cl_dev_log_descriptor *logDesc);
	virtual ~Scheduler();
	cl_int createCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	cl_int retainCommandList( cl_dev_cmd_list IN list);
	cl_int releaseCommandList( cl_dev_cmd_list IN list );
	cl_int commandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count);

protected:

	// Private type declaration
	typedef list<cl_dev_cmd_desc>				cmdDescList_t;
	typedef struct _CmdListItem_t {
		cl_dev_cmd_list_props	props;
		cmdDescList_t			cmdDescList;
		unsigned int			refrenceCount;
	} CmdListItem_t;
	typedef map<unsigned int, CmdListItem_t*>	cmdListMap_t;

	cl_int							m_devId;
	cl_dev_log_descriptor			m_logDesc;
	cl_dev_call_backs				m_frameWorkCallBacks;
	ProgramService*					m_programService;
	MemoryAllocator*				m_memoryAllocator;
	HandleAllocator<unsigned int>	m_listIdAlloc;

	cmdListMap_t			m_commandList;

	// Internal implementation of functions
	typedef cl_int	CheckCmdFunc_t(void*);
	typedef cl_int	ExecCmdFunc_t(Scheduler*, cl_dev_cmd_desc*);
	static	CheckCmdFunc_t*	m_checkCmdTable[];
	static	ExecCmdFunc_t*	m_execCmdTable[];
	
	// Native Kernel execution functions
	static cl_int	checkNativeKernelParam(void* param);
	static cl_int	execNativeKernel(Scheduler* _this,cl_dev_cmd_desc* cmd);
};

}}}