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

SyncGLObjects::SyncGLObjects(cl_command_type cmdType, GLContext* pContext, GLMemoryObject**pMemObjects, unsigned int uiMemObjNum, IOclCommandQueueBase* cmdQueue) :
SyncGraphicsApiObjects(cmdType, uiMemObjNum, cmdQueue, (GraphicsApiMemoryObject**)(pMemObjects),
                       cmdType == CL_COMMAND_ACQUIRE_GL_OBJECTS), m_pContext(pContext) { }

SyncGLObjects::~SyncGLObjects()
{
	if ( NULL != m_hCallingThread)
	{
		CloseHandle(m_hCallingThread);
	}
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

cl_err_code SyncGLObjects::Init()
{
    // Get calling thread handle
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
        GetCurrentProcess(), &m_hCallingThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    return SyncGraphicsApiObjects::Init();
}

cl_err_code SyncGLObjects::Execute()
{
    // Attach GL context to thread
    HGLRC hCurrentGL = wglGetCurrentContext();
    HDC hCurrentDC = wglGetCurrentDC();
	HGLRC hCntxGL = (HGLRC)m_pContext->GetGLCtx();
	HDC hCntxDC = (HDC)m_pContext->GetDC();
	HGLRC hBackupGL = NULL;

    if ( ( hCntxGL != hCurrentGL) || ( hCntxDC != hCurrentDC))
    {
		hBackupGL = m_pContext->GetBackupGLCntx();
		if ( NULL == hBackupGL )
		{
			return CL_OUT_OF_RESOURCES;
		}

		wglMakeCurrent(hCntxDC, hBackupGL);
    }

    // We are in the same thread as the context, so execute directly
    ExecGLSync(this);

	if ( NULL != hBackupGL )
	{
		wglMakeCurrent(hCurrentDC, hCurrentGL);
		m_pContext->RecycleBackupGLCntx(hBackupGL);
	}

    return CL_SUCCESS;
}

void SyncGLObjects::ExecGLSync(SyncGraphicsApiObjects* _this)
{
	// Do the actual work here
	if ( CL_COMMAND_ACQUIRE_GL_OBJECTS == _this->GetCommandType() )
	{
		for (unsigned int i=0; i<_this->GetNumMemObjs(); ++i)
		{
			dynamic_cast<GLMemoryObject&>(_this->GetMemoryObject(i)).AcquireGLObject();
		}
	}
	else
	{
		for (unsigned int i=0; i<_this->GetNumMemObjs(); ++i)
		{
			dynamic_cast<GLMemoryObject&>(_this->GetMemoryObject(i)).ReleaseGLObject();
		}
//		glFinish();
	}

	_this->RuntimeCommand::Execute();
}
