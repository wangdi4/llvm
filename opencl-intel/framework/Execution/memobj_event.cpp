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

///////////////////////////////////////////////////////////
//  memobj_event.cpp
//  Implementation of the Class MemoryObject
//  Created on:      29-May-2011 
//  Original author: Fisksman, Evgeny
///////////////////////////////////////////////////////////

#include "memobj_event.h"

#include <assert.h>
#include <cl_sys_info.h>

using namespace Intel::OpenCL::Framework;

MemoryObjectEvent::MemoryObjectEvent(IOCLDevMemoryObject* *ppDevMemObj, MemoryObject* pMemObject,FissionableDevice* pDevice) :
	OclEvent(), m_ppDevMemObj(ppDevMemObj) , m_pMemObject(pMemObject), m_pDevice(pDevice)
{
	SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
}

MemoryObjectEvent::~MemoryObjectEvent()
{
}

cl_err_code MemoryObjectEvent::ObservedEventStateChanged(OclEvent* pEvent, cl_int returnCode )
{
	if ( returnCode == CL_SUCCESS )
	{
		returnCode = m_pMemObject->GetDeviceDescriptor(m_pDevice, m_ppDevMemObj, NULL);
		assert(CL_SUCCESS == returnCode);
	}

	OclEvent::NotifyComplete(returnCode);
	return CL_SUCCESS;
}
