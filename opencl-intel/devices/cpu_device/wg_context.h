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

#include <vector>
#include <cl_device_api.h>
#include <cl_dev_backend_api.h>
#include "cl_shared_ptr.h"
#include "kernel_command.h"
#include "cl_synch_objects.h"

using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::DeviceCommands::KernelCommand;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class NDRange;

class WGContext : public WGContextBase
{
public:
    WGContext();
    virtual ~WGContext();

    cl_dev_err_code         CreateContext(long ndrCmdId, Intel::OpenCL::DeviceBackend::ICLDevBackendBinary_* pExec, size_t* pBuffSizes, size_t count);
    long					GetNDRCmdId() const {return m_lNDRangeId;}
    Intel::OpenCL::DeviceBackend::ICLDevBackendExecutable_* GetExecutable() const {return m_pContext;}
    // Initialize context internal memory
    cl_dev_err_code		Init();
    // This function is used by master threads when they're done executing, to prevent a race condition where the library is next shut down and reloaded
    // and invalid, seemingly-valid data is still present in the master thread's TLS
    void                        InvalidateContext();    
		
	/**
	 * @return a vector of waiting children for this work group
	 */
	std::vector<SharedPtr<KernelCommand> >& GetWaitingChildrenForWg() { return m_waitingChildrenForWg; }	

	/**
	 * @return a vector of waiting children for the parent
	 */
	std::vector<SharedPtr<KernelCommand> >& GetWaitingChildrenForParent() { return m_waitingChildrenForParent; }

	/**
	 * @return the NDRange command that is currently attached to the thread of this WGContext
	 */
	NDRange* GetCurrentNDRange() { return m_pCurrentNDRange; }

	/**
	 * Set the NDRange command that is currently attached to the thread of this WGContext or NULL
	 * @pCurrentNDRange the NDRange command that is currently attached to the thread of this WGContext or NULL
	 */
	void SetCurrentNDRange(NDRange* pCurrentNDRange) { m_pCurrentNDRange = pCurrentNDRange; }

protected:
    Intel::OpenCL::DeviceBackend::ICLDevBackendExecutable_* m_pContext;
    long		m_lNDRangeId;
    size_t		m_stPrivMemAllocSize;
    char*		m_pLocalMem;
    void*		m_pPrivateMem;    
    std::vector<SharedPtr<KernelCommand> > m_waitingChildrenForWg;
    std::vector<SharedPtr<KernelCommand> > m_waitingChildrenForParent;
    NDRange*    m_pCurrentNDRange;
};

}}}
