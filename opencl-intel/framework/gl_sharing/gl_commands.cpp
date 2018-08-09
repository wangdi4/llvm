// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
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

#include <cassert>
#include "gl_commands.h"
#include "queue_event.h"

using namespace Intel::OpenCL::Framework;

SyncGLObjects::SyncGLObjects(cl_command_type cmdType, SharedPtr<GLContext> pContext, SharedPtr<GraphicsApiMemoryObject>*pMemObjects, unsigned int uiMemObjNum, SharedPtr<IOclCommandQueueBase> cmdQueue) :
SyncGraphicsApiObjects(cmdType, uiMemObjNum, cmdQueue, pMemObjects, cmdType == CL_COMMAND_ACQUIRE_GL_OBJECTS), m_pContext(pContext) { }

SyncGLObjects::~SyncGLObjects()
{
}

const char* SyncGLObjects::GetCommandName() const
{
	if ( CL_COMMAND_ACQUIRE_GL_OBJECTS == GetCommandType())
	{
		return "CL_COMMAND_ACQUIRE_GL_OBJECTS";
	}
	else
	{
		return "CL_COMMAND_RELEASE_GL_OBJECTS";
	}
}

cl_err_code SyncGLObjects::Execute()
{
	{
		// Attach GL context to thread, need to be done in separate stack frame
		GLContext::GLContextSync glSync(m_pContext.GetPtr());

		// Acquired GL context must be valid
		if ( !glSync.IsValid() )
		{
			m_returnCode = CL_INVALID_OPERATION;
			return RuntimeCommand::Execute();
		}

		cl_err_code err = CL_SUCCESS;
		// Do the actual work here
		if ( CL_COMMAND_ACQUIRE_GL_OBJECTS == GetCommandType() )
		{
			for (unsigned int i=0; i<GetNumMemObjs(); ++i)
			{
				err = dynamic_cast<GLMemoryObject&>(GetMemoryObject(i)).AcquireGLObject();
				if ( CL_FAILED(err) )
				{
					m_returnCode = err;
				}
			}
		}
		else
		{
			for (unsigned int i=0; i<GetNumMemObjs(); ++i)
			{
				err = dynamic_cast<GLMemoryObject&>(GetMemoryObject(i)).ReleaseGLObject();
				if ( CL_FAILED(err) )
				{
					m_returnCode = err;
				}
			}
		}
	}

	return RuntimeCommand::Execute();
}

