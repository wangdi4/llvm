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

namespace Intel { namespace OpenCL { namespace MICDevice {

class WGContext
{
public:
	WGContext();
	virtual ~WGContext();

	cl_dev_err_code		CreateContext(cl_dev_cmd_id cmdId, ICLDevBackendBinary_* pExec, size_t* pBuffSizes, size_t count);
	cl_dev_cmd_id			GetCmdId() const {return m_cmdId;}
	inline ICLDevBackendExecutable_*	GetExecutable() const {return m_pContext;}
    // This function is used by master threads when they're done executing, to prevent a race condition where the library is next shut down and reloaded
    // and invalid, seemingly-valid data is still present in the master thread's TLS
    void                        InvalidateContext();

protected:
	ICLDevBackendExecutable_*	m_pContext;
	cl_dev_cmd_id				m_cmdId;
	char*						m_pLocalMem;
	void*						m_pPrivateMem;
	size_t						m_stPrivMemAllocSize;
};

}}}
