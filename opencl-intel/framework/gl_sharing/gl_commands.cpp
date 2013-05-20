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

#include <cassert>
#include "gl_commands.h"
#include "queue_event.h"
#include <stdafx.h>

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

