// Copyright (c) 2006-2012 Intel Corporation
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

#include "gl_context.h"
#include "gl_mem_objects.h"
#include "sync_graphics_api_objects.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class SyncGLObjects : public SyncGraphicsApiObjects
	{
	public:
		SyncGLObjects(cl_command_type cmdType, GLContext* pContext, GLMemoryObject* *pMemObjects, unsigned int uiMemObjNum, IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints);
		virtual ~SyncGLObjects();
		virtual cl_err_code             Init();        
		virtual cl_err_code             Execute();        
		virtual const char*             GetCommandName() const;		

	private:
		GLContext*		m_pContext;		
		void*			m_hCallingThread;

		static void __stdcall ExecGLSync(SyncGraphicsApiObjects* _this);
	};

}}}
