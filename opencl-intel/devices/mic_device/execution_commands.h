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

#pragma once

#include "command.h"

#include "mic_dev_limits.h"

#include <vector>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

class NDRange : public Command
{

public:

	/* static function for NDRange Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

	void fireCallBack(void* arg);

protected:

	virtual ~NDRange();

private:

	/* information about the buffer arguments in kernel blob */
	struct kernel_arg_buffer_info
	{
		// The offset of the buffer in the blob
		size_t offsetInBlob;
		// The index of the buffer in 'cl_kernel_argument' array
		unsigned int index;

		kernel_arg_buffer_info(size_t offset, unsigned int indx)
		{
			offsetInBlob = offset;
			index = indx;
		}
	};

	/* Private constructor because We like to create Commands only by the factory method */
    NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

	/* Initialize NDRange command. 
	   If the initialization process succeeded, memory will allocate for (*ppOutCoiBuffsArr) and (*ppAccessFlagArr)
	   IT IS THE CALLER RESPONSABILITY TO FREE THIS MEMORY (In case that the initialization method return CL_DEV_SUCCESS). */
	cl_dev_err_code init(COIBUFFER** ppOutCoiBuffsArr, COI_ACCESS_FLAGS** ppAccessFlagArr, unsigned int* pOutNumBuffers);

	void getKernelArgBuffersCount(const unsigned int numArgs, const cl_kernel_argument* pArgs, vector<kernel_arg_buffer_info>& oBuffsInfo);

	/* Release resources. */
	void releaseResources();

	COIBUFFER m_dispatcherDataBuffer;
	COIBUFFER m_printfBuffer;
	COIBUFFER m_profilingBuffer;

};

}}}
