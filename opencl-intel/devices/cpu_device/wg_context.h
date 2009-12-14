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

/*
*
* File wg_executor.h
* Declares class WGExecutor, class that manages execution of single Work-Group
*
*/

#pragma once

#include <cl_device_api.h>
#include <cl_dev_backend_api.h>
using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class WGContext
{
public:
	WGContext();
	virtual ~WGContext();

	int						CreateContext(cl_dev_cmd_id cmdId, ICLDevBackendBinary* pExec, size_t* pBuffSizes, size_t count);
	cl_dev_cmd_id			GetCmdId() const {return m_cmdId;}
	inline ICLDevBackendExecutable*	GetContext() const {return m_pContext;}

protected:
	ICLDevBackendExecutable*	m_pContext;
	char*					m_pLocalMem;
	cl_dev_cmd_id			m_cmdId;
};

}}}