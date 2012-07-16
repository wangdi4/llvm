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
//  user_event.h
//  Implementation of the Class UserEvent
//  Created on:      04-Jul-2010 
//  Original author: Singer, Doron
///////////////////////////////////////////////////////////

#include <cl_types.h>
#include <cl_object.h>
#include "ocl_event.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class Context;

	class BuildEvent : public OclEvent
	{
	public:
        BuildEvent(_cl_context_int* context);

		SharedPtr<Context>  GetContext() const { return m_pContext; }

		// OCLObject implementation
		cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet) const { return CL_INVALID_VALUE; }

		void        SetComplete(cl_int returnCode);

	protected:

		SharedPtr<Context>   m_pContext;
	};

}}}    // Intel::OpenCL::Framework
