// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "gl_context.h"
#include "gl_mem_objects.h"
#include "sync_graphics_api_objects.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class SyncGLObjects : public SyncGraphicsApiObjects
	{
	public:
		SyncGLObjects(cl_command_type cmdType, SharedPtr<GLContext> pContext, SharedPtr<GraphicsApiMemoryObject> *pMemObjects, unsigned int uiMemObjNum, SharedPtr<IOclCommandQueueBase> cmdQueue);
		virtual ~SyncGLObjects();
		virtual cl_err_code             Execute();        
		virtual const char*             GetCommandName() const;		

	private:
		SharedPtr<GLContext>		m_pContext;		

		void ExecGLSync(bool bMainGLThread);
	};

}}}
