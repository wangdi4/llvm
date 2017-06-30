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

#pragma once

#include "command_queue.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class represents an on-device command-queue
 */
class DeviceQueue : public OclCommandQueue
{
public:

	PREPARE_SHARED_PTR(DeviceQueue)

	static SharedPtr<DeviceQueue> Allocate(const SharedPtr<Context>& pContext, cl_device_id clDefaultDeviceID, cl_command_queue_properties clProperties, EventsManager* pEventManager,
										   bool bEnableProfiling, bool bIsDefault, cl_uint uiSize)
	{
		return new DeviceQueue(pContext, clDefaultDeviceID, clProperties, pEventManager, bEnableProfiling, bIsDefault, uiSize);
	}

	// overriden methods:

	virtual cl_err_code Initialize();

	virtual size_t GetInfoInternal(cl_int iParamName, void* pBuf, size_t szBuf) const;

	virtual void BecomeVisible();

	cl_err_code SetDefaultOnDevice(SharedPtr<FissionableDevice> pDevice)
	{
		return pDevice->SetDefaultDeviceQueue(this, m_clDevCmdListId);
	}

protected:
	virtual ~DeviceQueue()
	{
		// If the queue is a default device queue
		// the following call will unset it
		m_pDefaultDevice->UnsetDefaultQueueIfEqual(this);
	}

private:

	/**
	 * Constructor
	 * @param pContext				the context in which the command-queue is to be created
	 * @param clDefaultDeviceID		the device ID of the device in which the command-queue is to be created
	 * @param clProperties			the command-queue's properties
	 * @param pEventManager			a pointer to the EventManager
	 * @param bEnableProfiling		whether to enable profiling of commands in the command-queue
	 * @param bIsDefault			whether this command-queue is the default device queue
	 * @param uiSize				size of the device queue in bytes
	 */
	DeviceQueue(const SharedPtr<Context>& pContext, cl_device_id clDefaultDeviceID, cl_command_queue_properties clProperties, EventsManager* pEventManager, bool bEnableProfiling, bool bIsDefault,
		cl_uint uiSize) : OclCommandQueue(pContext, clDefaultDeviceID, clProperties, pEventManager), m_bIsDefault(bIsDefault), m_uiSize(uiSize) { }

	const bool m_bIsDefault;
	const cl_uint m_uiSize;

};

}}}
