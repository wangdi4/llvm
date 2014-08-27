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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  gl_context.cpp
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>

#include "cl_logger.h"

#include "gl_context.h"
#include "gl_shr_utils.h"
#include "device.h"
#include "gl_mem_objects.h"

#include <cl\cl_gl.h>
#include <cl_utils.h>

#ifdef WIN32
#pragma comment (lib, "opengl32.lib")
#define GL_GET_PROC_ADDRESS wglGetProcAddress
#else
#define GL_GET_PROC_ADDRESS glXGetProcAddressARB
#endif

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

GLContext::GLContext(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint numRootDevices, SharedPtr<FissionableDevice>*ppDevices, logging_fn pfnNotify,
					 void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints,
					 cl_context_properties hGLCtx, cl_context_properties hDC, ocl_gpa_data * pGPAData, ContextModule& contextModule) :
Context(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData, contextModule), m_hGLBackupCntx(NULL)
{
	if (*pclErr != CL_SUCCESS)
	{
		return;
	}
    if (NULL == hGLCtx)
    {
        *pclErr = CL_INVALID_VALUE;
        return;
    }
	// Check if any device is attached to context
	for(cl_uint idx = 0; idx < uiNumDevices; idx++)
	{
        if (!ppDevices[idx]->IsRootLevelDevice())
        {
            continue;
        }
		cl_context_properties devProp;
		ppDevices[idx]->GetInfo(CL_GL_CONTEXT_KHR, sizeof(cl_context_properties), &devProp, NULL);
        if ( (NULL != devProp) && (devProp != hGLCtx) )
        {
            *pclErr = CL_INVALID_OPERATION;
            return;
        }
		ppDevices[idx]->GetInfo(CL_WGL_HDC_KHR, sizeof(cl_context_properties), &devProp, NULL);
		if (NULL != devProp && devProp != hDC)
		{
			*pclErr = CL_INVALID_OPERATION;
				return;
			}
		}
	m_hGLCtx = hGLCtx;
	m_hDC = hDC;

	// All device passed, update GL info
    for (cl_uint idx = 0; idx < m_uiNumRootDevices; ++idx)
    {
        m_ppExplicitRootDevices[idx]->SetGLProperties(m_hGLCtx, m_hDC);
    }
	// Init GL extension functions
	this->glBindBuffer = (pFnglBindBuffer*)GL_GET_PROC_ADDRESS("glBindBuffer");
	this->glMapBuffer = (pFnglMapBuffer*)GL_GET_PROC_ADDRESS("glMapBuffer");
	this->glUnmapBuffer = (pFnglUnmapBuffer*)GL_GET_PROC_ADDRESS("glUnmapBuffer");
	this->glGetBufferParameteriv = (pFnglGetBufferParameteriv*)GL_GET_PROC_ADDRESS("glGetBufferParameteriv");
	this->glBufferData = (pFnglBufferData*)GL_GET_PROC_ADDRESS("glBufferData");
	this->glTexImage3D = (pFnglTexImage3D*)GL_GET_PROC_ADDRESS("glTexImage3D");
	this->glGetRenderbufferParameterivEXT = (pFnglGetRenderbufferParameterivEXT*)GL_GET_PROC_ADDRESS("glGetRenderbufferParameterivEXT");
	this->glBindRenderbufferEXT = (pFnglBindRenderbufferEXT*)GL_GET_PROC_ADDRESS("glBindRenderbufferEXT");
	this->glGenFramebuffersEXT = (pFnglGenFramebuffersEXT*)GL_GET_PROC_ADDRESS("glGenFramebuffersEXT");
	this->glDeleteFramebuffersEXT = (pFnglDeleteFramebuffersEXT*)GL_GET_PROC_ADDRESS("glDeleteFramebuffersEXT");
	this->glBindFramebufferEXT = (pFnglBindFramebufferEXT*)GL_GET_PROC_ADDRESS("glBindFramebufferEXT");
	this->glFramebufferRenderbufferEXT = (pFnglFramebufferRenderbufferEXT*)GL_GET_PROC_ADDRESS("glFramebufferRenderbufferEXT");
	this->glFramebufferTexture1DEXT = (pFnglFramebufferTextureXDEXT*)GL_GET_PROC_ADDRESS("glFramebufferTexture1DEXT");
	this->glFramebufferTexture2DEXT = (pFnglFramebufferTextureXDEXT*)GL_GET_PROC_ADDRESS("glFramebufferTexture2DEXT");
	this->glCheckFramebufferStatusEXT = (pFnglCheckFramebufferStatusEXT*)GL_GET_PROC_ADDRESS("glCheckFramebufferStatusEXT");
	this->glGenBuffers = (pFnglGenBuffers*)GL_GET_PROC_ADDRESS("glGenBuffers");
	this->glDeleteBuffers = (pFnglDeleteBuffers*)GL_GET_PROC_ADDRESS("glDeleteBuffers");
#ifdef WIN32
	this->wglCreateContextAttribsARB = (pFnwglCreateContextAttribsARB*)GL_GET_PROC_ADDRESS("wglCreateContextAttribsARB");
	if ( NULL == this->wglCreateContextAttribsARB )
	{
		*pclErr = CL_OUT_OF_RESOURCES;
        return;
	}
#endif

	// Need allocate new one
	m_hGLBackupCntx = wglCreateContextAttribsARB((HDC)m_hDC, (HGLRC)m_hGLCtx, NULL);
	if ( NULL == m_hGLBackupCntx )
	{
		*pclErr = CL_OUT_OF_RESOURCES;
        return;
	}
}

GLContext::~GLContext()
{
#ifdef WIN32
	// Remove previously allocated GL contexts
	if ( NULL != m_hGLBackupCntx )
	{
		wglDeleteContext(m_hGLBackupCntx);
		m_hGLBackupCntx = NULL;
	}
#endif

    if (NULL != m_ppExplicitRootDevices)
    {
        // All device passed, update GL info
        for (cl_uint idx = 0; idx < m_uiNumRootDevices; ++idx)
        {
            m_ppExplicitRootDevices[idx]->SetGLProperties(NULL, NULL);
        }
    }
}

// create GL buffer object
cl_err_code GLContext::CreateGLBuffer(cl_mem_flags clFlags, GLuint glBufObj, SharedPtr<MemoryObject>* ppBuffer)
{
	LOG_DEBUG(TEXT("Enter - (cl_mem_flags=%d, glBufObj=%d, ppBuffer=%d)"),
		clFlags, glBufObj, ppBuffer);

	assert ( NULL != ppBuffer );

	SharedPtr<MemoryObject> pBuffer;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_GL_OBJECT_BUFFER, CL_MEMOBJ_GFX_SHARE_GL, this, &pBuffer);
	if ( CL_FAILED(clErr) )
	{
		LOG_ERROR(TEXT("Error creating new GL buffer, returned: %s"), ClErrTxt(clErr));
		return clErr;
	}

	{	// Operation requires nested stack frame, GLContext should be release after Initialize
		GLContext::GLContextSync sync(this);

		clErr = pBuffer->Initialize(clFlags, NULL, 1, NULL, NULL, (void*)glBufObj, CL_RT_MEMOBJ_FORCE_BS);
		if (CL_FAILED(clErr))
		{
			LOG_ERROR(TEXT("Failed to initialize data, pBuffer->Initialize(pHostPtr = %s"), ClErrTxt(clErr));
			pBuffer->Release();
			return clErr;
		}
	}
	m_mapMemObjects.AddObject(pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLTexture(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, cl_mem_object_type clObjType, SharedPtr<MemoryObject>* ppImage)
{
	LOG_INFO(TEXT("Enter - params(cl_mem_flags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%d ppImage=%p)"), 
		clMemFlags, glTextureTarget, glMipLevel, glTexture, ppImage);

	assert ( NULL != ppImage );

	SharedPtr<MemoryObject> pImage;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, clObjType, CL_MEMOBJ_GFX_SHARE_GL, this, &pImage);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new GLTexture2D, returned: %s"), ClErrTxt(clErr));
		return clErr;
	}

	GLMemoryObject::GLTextureDescriptor txtDesc;
	txtDesc.glTexture = glTexture;
	txtDesc.glMipLevel = glMipLevel;
	txtDesc.glTextureTarget = glTextureTarget;

	{	// Operation requires nested stack frame, GLContext should be release after Initialize
		GLContext::GLContextSync sync(this);
		clErr = pImage->Initialize(clMemFlags, NULL, pImage->GetNumDimensions(), NULL, NULL, &txtDesc, CL_RT_MEMOBJ_FORCE_BS);
		if (CL_FAILED(clErr))
		{
			LOG_ERROR(TEXT("Failed to initialize data, pImage->Initialize() returns %s"), ClErrTxt(clErr));
			pImage->Release();
			return clErr;
		}
	}

	m_mapMemObjects.AddObject(pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLRenderBuffer(cl_mem_flags clMemFlags, GLuint glRednderBuffer, SharedPtr<MemoryObject> *ppImage)
{
	LOG_DEBUG(TEXT("Enter - (cl_mem_flags=%d, glRednderBuffer=%d, ppImage=%d)"),
		clMemFlags, glRednderBuffer, ppImage);

	assert ( NULL != ppImage );

	SharedPtr<MemoryObject> pImage;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_GL_OBJECT_RENDERBUFFER, CL_MEMOBJ_GFX_SHARE_GL, this, &pImage);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new GLRederBuffer, returned: %s"), ClErrTxt(clErr));
		return clErr;
	}

	{	// Operation requires nested stack frame, GLContext should be release after Initialize
		GLContext::GLContextSync sync(this);
		clErr = pImage->Initialize(clMemFlags, NULL, 2, NULL, NULL, (void*)glRednderBuffer, CL_RT_MEMOBJ_FORCE_BS);
		if (CL_FAILED(clErr))
		{
			LOG_ERROR(TEXT("Failed to initialize data, pImage->Initialize(pHostPtr = %s"), ClErrTxt(clErr));
			pImage->Release();
			return clErr;
		}
	}

	m_mapMemObjects.AddObject(pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}

cl_err_code GLContext::AcquiereGLCntx(HGLRC hCntxGL, HDC hDC)
{
	const HGLRC	hMyCntxGL = (HGLRC)m_hGLCtx;
	const HDC	hMyCntxDC = (HDC)m_hDC;

	if ( NULL == m_hGLBackupCntx )
	{
		return CL_OUT_OF_RESOURCES;
	}

	// We are using same context, no need to change
	if ( ((hMyCntxGL == hCntxGL) || (m_hGLBackupCntx==hCntxGL)) && (hMyCntxDC == hDC) )
	{
		return CL_FALSE;
	}

	BOOL err = 0;
#if 0
	// First make a try to choose GL context wich was used during CL context creation
	err = wglMakeCurrent(hMyCntxDC, hMyCntxGL);
	if ( TRUE == err )
	{
		return CL_TRUE;
	}
#endif

	// If a context exists, firts wait until all GL operations are completed
	if ( NULL != hCntxGL )
	{
		glFinish();
	}

	// Acquire backup context, block others
	m_muGLBkpCntx.Lock();

	int iter = 0;
	do
	{
		// Set our context as current one
		err = wglMakeCurrent(hMyCntxDC, m_hGLBackupCntx);
		if ( !err )
		{
			Sleep(1);
		}
	} while ( (FALSE==err) && ((++iter)<10) );

	DWORD wErr = GetLastError();
	if( (TRUE!=err) || (0!= wErr) )
	{
		m_muGLBkpCntx.Unlock();
		LOG_ERROR(TEXT("Failed to active backup GL context after 10 iterations. DC=%p, GL=%p. LastError()=%x"),
			(void*)hMyCntxDC, (void*)m_hGLBackupCntx, wErr);
		return CL_INVALID_OPERATION;
	}

	return CL_TRUE;
}

void GLContext::RestoreGLCntx(HGLRC hCntxGL, HDC hDC)
{
	const HGLRC	hMyCntxGL = (HGLRC)m_hGLCtx;
	const HDC	hMyCntxDC = (HDC)m_hDC;

	// We are using same context
	if ( (hMyCntxGL == hCntxGL) && (hMyCntxDC == hDC) )
	{
		return;
	}

	const HGLRC hCurrentContext = wglGetCurrentContext();

	// Finish pending operations on current context
	glFinish();

	wglMakeCurrent(hDC, hCntxGL);
	assert( (0 == GetLastError()) && "Failed to revert to original GL context");
	if ( hCurrentContext == m_hGLBackupCntx )
	{
		// The internal backup context was used, we need to release it
		m_muGLBkpCntx.Unlock();
	}
}

///////////////////////////////////////////////////////////////////
GLContext::GLContextSync::GLContextSync(GLContext* glCtx) : m_pGLContext(glCtx), m_bUpdated(false)
{
#ifdef WIN32
    m_hCurrentGL = wglGetCurrentContext();
    m_hCurrentDC = wglGetCurrentDC();

	cl_err_code ret = m_pGLContext->AcquiereGLCntx(m_hCurrentGL, m_hCurrentDC);
	m_bUpdated = (CL_TRUE == ret);
	m_bValid = !CL_FAILED(ret);
#endif
}

GLContext::GLContextSync::~GLContextSync()
{
	if (m_bUpdated)
	{
		m_pGLContext->RestoreGLCntx(m_hCurrentGL, m_hCurrentDC);
	}
}

