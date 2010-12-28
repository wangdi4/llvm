// Copyright (c) 2008-2009 Intel Corporation
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

#include <stdafx.h>
#include <cassert>
#include "gl_commands.h"
#include "queue_event.h"

using namespace Intel::OpenCL::Framework;

SyncGLObjects::SyncGLObjects(cl_command_type cmdType, GLContext* pContext, GLMemoryObject* *pMemObjects, unsigned int uiMemObjNum) :
m_uiMemObjNum(uiMemObjNum),m_pContext(pContext), m_cmdType(cmdType), m_hCallingThread(NULL)
{
	m_pMemObjects = new GLMemoryObject*[uiMemObjNum];
	memcpy_s(m_pMemObjects, sizeof(GLMemoryObject*)*uiMemObjNum, pMemObjects, sizeof(GLMemoryObject*)*uiMemObjNum);
}

SyncGLObjects::~SyncGLObjects()
{
	if ( NULL != m_hCallingThread)
	{
		CloseHandle(m_hCallingThread);
	}
	delete []m_pMemObjects;
}

const char* SyncGLObjects::GetCommandName() const
{
	if ( CL_COMMAND_ACQUIRE_GL_OBJECTS == m_cmdType)
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

	return CL_SUCCESS;
}

cl_err_code SyncGLObjects::Execute()
{
	// Attach GL context to thread
	HGLRC hGL = wglGetCurrentContext();
	HDC hDC = wglGetCurrentDC();

	if ( ((HGLRC)m_pContext->GetGLCtx() == hGL) &&
		 ((HDC)m_pContext->GetDC() == hDC))
	{
		// We are in the same thread as the context, so execute directly
		ExecGLSync(this);
		return CL_SUCCESS;
	}

	// Set event to RED
	assert(m_pEvent);
	m_pEvent->SetColor(EVENT_STATE_LIME);
	QueueUserAPC((PAPCFUNC)ExecGLSync, m_hCallingThread, (ULONG_PTR)this);
	return CL_NOT_READY;
}

void SyncGLObjects::ExecGLSync(SyncGLObjects* _this)
{
	// Do the actual work here
	if ( CL_COMMAND_ACQUIRE_GL_OBJECTS == _this->m_cmdType )
	{
		for (unsigned int i=0; i<_this->m_uiMemObjNum; ++i)
		{
			_this->m_pMemObjects[i]->GetGLObjectData();
		}
	}
	else
	{
		for (unsigned int i=0; i<_this->m_uiMemObjNum; ++i)
		{
			_this->m_pMemObjects[i]->SetGLObjectData();
		}
		glFinish();
	}

	_this->RuntimeCommand::Execute();
}