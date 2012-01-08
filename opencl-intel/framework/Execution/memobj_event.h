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
///////////////////////////////////////////////////////////
//  memobj_event.h
//  Implementation of the Class MemoryObject
//  Created on:      29-May-2011 
//  Original author: Fisksman, Evgeny
///////////////////////////////////////////////////////////

#include "ocl_event.h"
#include "MemoryObject.h"

#include <cl_types.h>
#include <cl_object.h>
#include <cl_device_api.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class MemoryObjectEvent : public OclEvent
	{
	public:
		MemoryObjectEvent( IOCLDevMemoryObject* *ppDevMemObj, MemoryObject* pMemObject, FissionableDevice* pDevice );

		// Get the context to which the event belongs.
		cl_context GetContextHandle() const { return NULL;}
		// Get the return code of the command associated with the event.
		cl_int     GetReturnCode() const {return 0;}
		cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
			{return CL_INVALID_OPERATION;}

		// IEventDoneObserver
		cl_err_code NotifyEventDone(OclEvent* pEvent, cl_int returnCode = CL_SUCCESS);

	protected:
		virtual ~MemoryObjectEvent();        

		IOCLDevMemoryObject*	*m_ppDevMemObj;
		MemoryObject*			m_pMemObject;
		FissionableDevice*		m_pDevice;

		// A MemObjectEvent object cannot be copied
		MemoryObjectEvent(const MemoryObjectEvent&);           // copy constructor
		MemoryObjectEvent& operator=(const MemoryObjectEvent&);// assignment operator
	};

}}}    // Intel::OpenCL::Framework
