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
//  gl_context.cpp
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: efiksman
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

GLContext::GLContext(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint numRootDevices, FissionableDevice **ppDevices, logging_fn pfnNotify,
					 void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints,
					 cl_context_properties hGLCtx, cl_context_properties hDC, ocl_gpa_data * pGPAData) :
	Context(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData)
{
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
		if (devProp != hDC)
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
        m_ppRootDevices[idx]->SetGLProperties(m_hGLCtx, m_hDC);
    }
	// Init GL extension functions
#ifdef WIN32
	this->glBindBuffer = (pFnglBindBuffer*)wglGetProcAddress("glBindBuffer");
	this->glMapBuffer = (pFnglMapBuffer*)wglGetProcAddress("glMapBuffer");
	this->glUnmapBuffer = (pFnglUnmapBuffer*)wglGetProcAddress("glUnmapBuffer");
	this->glGetBufferParameteriv = (pFnglGetBufferParameteriv*)wglGetProcAddress("glGetBufferParameteriv");
	this->glBufferData = (pFnglBufferData*)wglGetProcAddress("glBufferData");
	this->glTexImage3D = (pFnglTexImage3D*)wglGetProcAddress("glTexImage3D");
	this->glGetRenderbufferParameterivEXT = (pFnglGetRenderbufferParameterivEXT*)wglGetProcAddress("glGetRenderbufferParameterivEXT");
	this->glBindRenderbufferEXT = (pFnglBindRenderbufferEXT*)wglGetProcAddress("glBindRenderbufferEXT");
	this->glGenFramebuffersEXT = (pFnglGenFramebuffersEXT*)wglGetProcAddress("glGenFramebuffersEXT");
	this->glDeleteFramebuffersEXT = (pFnglDeleteFramebuffersEXT*)wglGetProcAddress("glDeleteFramebuffersEXT");
	this->glBindFramebufferEXT = (pFnglBindFramebufferEXT*)wglGetProcAddress("glBindFramebufferEXT");
	this->glFramebufferRenderbufferEXT = (pFnglFramebufferRenderbufferEXT*)wglGetProcAddress("glFramebufferRenderbufferEXT");
	this->glFramebufferTexture2DEXT = (pFnglFramebufferTexture2DEXT*)wglGetProcAddress("glFramebufferTexture2DEXT");
	this->glCheckFramebufferStatusEXT = (pFnglCheckFramebufferStatusEXT*)wglGetProcAddress("glCheckFramebufferStatusEXT");
	this->glGenBuffers = (pFnglGenBuffers*)wglGetProcAddress("glGenBuffers");
	this->glDeleteBuffers = (pFnglDeleteBuffers*)wglGetProcAddress("glDeleteBuffers");
#endif

}

GLContext::~GLContext()
{
	// All device passed, update GL info
    for (cl_uint idx = 0; idx < m_uiNumRootDevices; ++idx)
    {
        m_ppRootDevices[idx]->SetGLProperties(NULL, NULL);
    }
}

// create GL buffer object
cl_err_code GLContext::CreateGLBuffer(cl_mem_flags clFlags, GLuint glBufObj, MemoryObject ** ppBuffer)
{
	LOG_DEBUG(TEXT("Enter - (cl_mem_flags=%d, glBufObj=%d, ppBuffer=%d)"),
		clFlags, glBufObj, ppBuffer);

	assert ( NULL != ppBuffer );

	MemoryObject* pBuffer;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_GL_OBJECT_BUFFER, CL_MEMOBJ_GFX_SHARE_GL, this, &pBuffer);
	if ( CL_FAILED(clErr) )
	{
		LOG_ERROR(TEXT("Error creating new GL buffer, returned: %S"), ClErrTxt(clErr));
		return clErr;
	}

	clErr = pBuffer->Initialize(clFlags, NULL, 1, NULL, NULL, (void*)glBufObj);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Failed to initialize data, pBuffer->Initialize(pHostPtr = %S"), ClErrTxt(clErr));
		pBuffer->Release();
		return clErr;
	}

	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLTexture2D(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, MemoryObject* *ppImage)
{

	LOG_DEBUG(TEXT("Enter - (cl_mem_flags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%d ppImage=%d)"), 
		clMemFlags, glTextureTarget, glMipLevel, glTexture, ppImage);

	assert ( NULL != ppImage );

	MemoryObject * pImage;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_GL_OBJECT_TEXTURE2D, CL_MEMOBJ_GFX_SHARE_GL, this, &pImage);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new GLTexture2D, returned: %S"), ClErrTxt(clErr));
		return clErr;
	}

	GLTexture::GLTextureDescriptor txtDesc;
	txtDesc.glTexture = glTexture;
	txtDesc.glMipLevel = glMipLevel;
	txtDesc.glTextureTarget = glTextureTarget;

	clErr = pImage->Initialize(clMemFlags, NULL, 2, NULL, NULL, &txtDesc);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initialize data, pImage->Initialize(pHostPtr = %S", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLTexture3D(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, MemoryObject* *ppImage)
{
	LOG_DEBUG(TEXT("Enter - (cl_mem_flags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%d ppImage=%d)"), 
		clMemFlags, glTextureTarget, glMipLevel, glTexture, ppImage);

	assert ( NULL != ppImage );

	MemoryObject * pImage;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_GL_OBJECT_TEXTURE3D, CL_MEMOBJ_GFX_SHARE_GL, this, &pImage);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new GLTexture3D, returned: %S"), ClErrTxt(clErr));
		return clErr;
	}

	GLTexture::GLTextureDescriptor txtDesc;
	txtDesc.glTexture = glTexture;
	txtDesc.glMipLevel = glMipLevel;
	txtDesc.glTextureTarget = glTextureTarget;

	clErr = pImage->Initialize(clMemFlags, NULL, 3, NULL, NULL, &txtDesc);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Failed to initialize data, pImage->Initialize(pHostPtr = %S"), ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}

cl_err_code GLContext::CreateGLRenderBuffer(cl_mem_flags clMemFlags, GLuint glRednderBuffer, MemoryObject* *ppImage)
{
	LOG_DEBUG(TEXT("Enter - (cl_mem_flags=%d, glRednderBuffer=%d, ppImage=%d)"),
		clMemFlags, glRednderBuffer, ppImage);

	assert ( NULL != ppImage );

	MemoryObject * pImage;
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_GL_OBJECT_RENDERBUFFER, CL_MEMOBJ_GFX_SHARE_GL, this, &pImage);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new GLRederBuffer, returned: %S"), ClErrTxt(clErr));
		return clErr;
	}

	clErr = pImage->Initialize(clMemFlags, NULL, 2, NULL, NULL, (void*)glRednderBuffer);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initialize data, pImage->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
		pImage->Release();
		return clErr;
	}

	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)pImage);

	*ppImage = pImage;
	return CL_SUCCESS;
}
