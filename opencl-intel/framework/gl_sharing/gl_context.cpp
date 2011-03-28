// Copyright (c) 2006-2007 Intel Corporation
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
//  Context.cpp
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "gl_context.h"
#include "gl_shr_utils.h"
#include "device.h"
#include "gl_mem_objects.h"
#include <cl\cl_gl.h>
#include <cl_utils.h>

#pragma comment (lib, "opengl32.lib")

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

GLContext::GLContext(const cl_context_properties * clProperties, cl_uint uiNumDevices, Device **ppDevices, logging_fn pfnNotify,
					 void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints,
					 cl_context_properties hGLCtx, cl_context_properties hDC, bool bUseTaskalyzer, char cStageMarkerFlags) :
	Context(clProperties, uiNumDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, bUseTaskalyzer, m_cStageMarkerFlags)
{
	if ( (NULL == hGLCtx) || (NULL == hDC) )
	{
		*pclErr = CL_INVALID_VALUE;
		return;
	}

	// Check if any device is attached to context
	for(cl_uint idx = 0; idx < uiNumDevices; idx++)
	{
		cl_context_properties devProp;
		ppDevices[idx]->GetInfo(CL_GL_CONTEXT_KHR, sizeof(cl_context_properties), &devProp, NULL);
		if ( (NULL != devProp) && (devProp != hGLCtx) )
		{
			*pclErr = CL_INVALID_OPERATION;
			return;
		}
		ppDevices[idx]->GetInfo(CL_WGL_HDC_KHR, sizeof(cl_context_properties), &devProp, NULL);
		if ( (NULL != devProp) && (devProp != hDC) )
		{
			*pclErr = CL_INVALID_OPERATION;
				return;
			}
		}
	m_hGLCtx = hGLCtx;
	m_hDC = hDC;

	// All device passed, update GL info
	for(cl_uint idx = 0; idx < uiNumDevices; idx++)
	{
		ppDevices[idx]->SetGLProperties(m_hGLCtx, m_hDC);
	}

	// Init GL extension functions
#ifdef WIN32
	this->glBindBuffer = (pFnglBindBuffer*)wglGetProcAddress("glBindBuffer");
	this->glMapBuffer = (pFnglMapBuffer*)wglGetProcAddress("glMapBuffer");
	this->glUnmapBuffer = (pFnglUnmapBuffer*)wglGetProcAddress("glUnmapBuffer");
	this->glGetBufferParameteriv = (pFnglGetBufferParameteriv*)wglGetProcAddress("glGetBufferParameteriv");
	this->glTexImage3D = (pFnglTexImage3D*)wglGetProcAddress("glTexImage3D");
	this->glGetRenderbufferParameterivEXT = (pFnglGetRenderbufferParameterivEXT*)wglGetProcAddress("glGetRenderbufferParameterivEXT");
	this->glBindRenderbufferEXT = (pFnglBindRenderbufferEXT*)wglGetProcAddress("glBindRenderbufferEXT");
	this->glGenFramebuffersEXT = (pFnglGenFramebuffersEXT*)wglGetProcAddress("glGenFramebuffersEXT");
	this->glDeleteFramebuffersEXT = (pFnglDeleteFramebuffersEXT*)wglGetProcAddress("glDeleteFramebuffersEXT");
	this->glBindFramebufferEXT = (pFnglBindFramebufferEXT*)wglGetProcAddress("glBindFramebufferEXT");
	this->glFramebufferRenderbufferEXT = (pFnglFramebufferRenderbufferEXT*)wglGetProcAddress("glFramebufferRenderbufferEXT");
	this->glCheckFramebufferStatusEXT = (pFnglCheckFramebufferStatusEXT*)wglGetProcAddress("glCheckFramebufferStatusEXT");
#endif

}

GLContext::~GLContext()
{
	// All device passed, update GL info
	for(cl_uint idx = 0; idx < m_pDevices->Count(); idx++)
	{
		m_ppDevices[idx]->SetGLProperties(NULL, NULL);
	}
}

// create GL buffer object
cl_err_code GLContext::CreateGLBuffer(cl_mem_flags clFlags, GLuint glBufObj, Buffer ** ppBuffer)
{
	LOG_DEBUG(L"Enter CreateBuffer (cl_mem_flags=%d, glBufObj=%d, ppBuffer=%d)", 
		clFlags, glBufObj, ppBuffer);

	assert ( NULL != ppBuffer );
	assert ( NULL != m_pMemObjects );

	cl_err_code clErr = CL_SUCCESS;
	Buffer * pBuffer = new GLBuffer(this, clFlags, glBufObj, (ocl_entry_points*)m_handle.dispatch, &clErr);
	if ( NULL == pBuffer )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new GL buffer, returned: %ws", ClErrTxt(clErr));
		pBuffer->Release();
		return clErr;
	}

	clErr = pBuffer->Initialize(NULL);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initialize data, pBuffer->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
		pBuffer->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLTexture2D(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, MemoryObject* *ppImage)
{

	LOG_DEBUG(L"Enter CreateGLTexture2D (cl_mem_flags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%d ppImage=%d)", 
		clMemFlags, glTextureTarget, glMipLevel, glTexture, ppImage);

	assert ( NULL != ppImage );
	assert ( NULL != m_pMemObjects );

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pImage = new GLTexture2D(this, clMemFlags, glTextureTarget, glMipLevel, glTexture,(ocl_entry_points*)m_handle.dispatch, &clErr);
	if ( NULL == pImage )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new GLTexture2D, returned: %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	clErr = pImage->Initialize(NULL);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initialize data, pBuffer->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLTexture3D(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, MemoryObject* *ppImage)
{
	LOG_DEBUG(L"Enter CreateGLTexture3D (cl_mem_flags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%d ppImage=%d)", 
		clMemFlags, glTextureTarget, glMipLevel, glTexture, ppImage);

	assert ( NULL != ppImage );
	assert ( NULL != m_pMemObjects );

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pImage = new GLTexture3D(this, clMemFlags, glTextureTarget, glMipLevel, glTexture,(ocl_entry_points*)m_handle.dispatch, &clErr);
	if ( NULL == pImage )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new GLTexture3D, returned: %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	clErr = pImage->Initialize(NULL);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initialize data, pBuffer->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLRenderBuffer(cl_mem_flags clMemFlags, GLuint glRednderBuffer, MemoryObject* *ppImage)
{
	LOG_DEBUG(L"Enter CreateGLRenderBuffer (cl_mem_flags=%d, glRednderBuffer=%d, ppImage=%d)", 
		clMemFlags, glRednderBuffer, ppImage);

	assert ( NULL != ppImage );
	assert ( NULL != m_pMemObjects );

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pImage = new GLRenderBuffer(this, clMemFlags, glRednderBuffer, (ocl_entry_points*)m_handle.dispatch, &clErr);
	if ( NULL == pImage )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new GLRenderBuffer, returned: %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	clErr = pImage->Initialize(NULL);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initialize data, pBuffer->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}