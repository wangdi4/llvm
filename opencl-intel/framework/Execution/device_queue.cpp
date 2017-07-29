// Copyright (c) 2008-2012 Intel Corporation
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

#include "device_queue.h"
#include "Context.h"
#include "context_module.h"

using namespace Intel::OpenCL::Framework;

cl_err_code DeviceQueue::Initialize()
{
    if (m_bIsDefault)
        m_pDefaultDevice->SetOrReturnDefaultQueue(this);
	const cl_dev_subdevice_id subdevice_id = m_pContext->GetSubdeviceId(m_clDefaultDeviceHandle);
	const int props = CL_DEV_LIST_ENABLE_OOO |
					  (m_bProfilingEnabled ? CL_DEV_LIST_PROFILING : 0) |
					  (m_bIsDefault ? CL_DEV_LIST_QUEUE_DEFAULT : 0);
	// currently we don't make use of the CL_QUEUE_SIZE value
	const cl_dev_err_code retDev = m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList((cl_dev_cmd_list_props)props, subdevice_id, &m_clDevCmdListId);
	if (CL_DEV_FAILED(retDev))
	{
		m_clDevCmdListId = 0;
		if (m_bIsDefault)
		{
			m_pDefaultDevice->UnsetDefaultQueueIfEqual(this);
		}		
		return CL_OUT_OF_RESOURCES;
	}
	return CL_SUCCESS;
}

size_t DeviceQueue::GetInfoInternal(cl_int iParamName, void* pBuf, size_t szBuf) const
{
	switch (iParamName)
	{
	case CL_QUEUE_SIZE:
		assert(szBuf >= sizeof(cl_uint));
		if (szBuf < sizeof(cl_uint))
		{
			return 0;
		}
		*(cl_uint*)pBuf = m_uiSize;
		return sizeof(cl_uint);
	case CL_QUEUE_PROPERTIES:
		{
			const size_t szSize = OclCommandQueue::GetInfoInternal(iParamName, pBuf, szBuf);
			if (0 == szSize)
			{
				return 0;
			}
			*(cl_command_queue_properties*)pBuf |= CL_QUEUE_ON_DEVICE;
			if (m_bIsDefault)
			{
				*(cl_command_queue_properties*)pBuf |= CL_QUEUE_ON_DEVICE_DEFAULT;
			}
			return szSize;
		}
	default:
		return OclCommandQueue::GetInfoInternal(iParamName, pBuf, szBuf);
	}
}

void DeviceQueue::BecomeVisible()
{
	m_pContext->GetContextModule().CommandQueueCreated(this);
}
