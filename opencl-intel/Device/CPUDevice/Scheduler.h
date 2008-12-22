
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

//should be hash_map but cant compile #include <hash_map>
#include <map>
using namespace std;




#include"cl_device_api.h"
#include "ProgramService.h"

typedef struct listData_t {
		cl_dev_cmd_list_props props;
		cl_dev_cmd_desc  *cmds;	
		cl_int  count;
		unsigned int refrenceCount;
}listData_t;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class Scheduler
{

public:
	Scheduler(cl_int devId, cl_dev_call_backs *devCallbacks, ProgramService	*programService, cl_dev_log_descriptor *logDesc);
	virtual ~Scheduler();
	cl_int createCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	cl_int retainCommandList( cl_dev_cmd_list IN list);
	cl_int releaseCommandList( cl_dev_cmd_list IN list );
	cl_int commandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count);
protected:
	cl_int					m_devId;
	cl_dev_log_descriptor*	m_logDesc;
	cl_dev_call_backs m_frameWorkCallBacks;
	ProgramService			*m_programService;
	unsigned int m_uiListId;
	map<unsigned int, listData_t*> m_commandList;

};

}}}