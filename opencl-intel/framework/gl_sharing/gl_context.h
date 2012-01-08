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

#include "context.h"
#ifdef _MSC_VER
#include <Windows.h> // Required for gl.h
#endif
#include "gl/gl.h"
#include "gl/glext.h"

namespace Intel { namespace OpenCL { namespace Framework {

	typedef void APIENTRY pFnglBindBuffer(GLenum target, GLuint buffer);
	typedef GLvoid* APIENTRY pFnglMapBuffer(GLenum target, GLenum   access);
	typedef GLboolean APIENTRY pFnglUnmapBuffer(GLenum target);
	typedef void APIENTRY pFnglBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage); 
	typedef void APIENTRY pFnglGetBufferParameteriv(GLenum target, GLenum value, GLint* data);
	typedef void APIENTRY pFnglTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void APIENTRY pFnglGetRenderbufferParameterivEXT(GLenum target, GLenum param, GLint* value);
	typedef void APIENTRY pFnglBindRenderbufferEXT(GLenum target, GLuint id);
	typedef void APIENTRY pFnglGenFramebuffersEXT(GLsizei n, GLuint* framebuffers);
	typedef void APIENTRY pFnglDeleteFramebuffersEXT(GLsizei n, GLuint* framebuffers);
	typedef void APIENTRY pFnglBindFramebufferEXT(GLenum target, GLuint framebuffer);
	typedef void APIENTRY pFnglFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);;
	typedef void APIENTRY pFnglFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	typedef GLenum APIENTRY pFnglCheckFramebufferStatusEXT(GLenum target);
	typedef void APIENTRY pFnglGenBuffers(GLsizei n, GLuint *buffers); 
	typedef void APIENTRY pFnglDeleteBuffers(GLsizei n, const GLuint *   buffers);


	class GLContext : public Context
	{
	public:
		GLContext(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices, FissionableDevice **ppDevices,
			logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints,
			cl_context_properties hDC, cl_context_properties hGLCtx, ocl_gpa_data * pGPAData);

		cl_context_properties GetDC() const { return m_hDC;}
		cl_context_properties GetGLCtx() const { return m_hGLCtx;}

		// create GL buffer object
		cl_err_code CreateGLBuffer(cl_mem_flags clFlags, GLuint glBufObj, MemoryObject ** ppBuffer);
		cl_err_code CreateGLTexture2D(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, MemoryObject* *ppImage);
		cl_err_code CreateGLTexture3D(cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, MemoryObject* *ppImage);
		cl_err_code CreateGLRenderBuffer(cl_mem_flags clMemFlags, GLuint glRenderBuffer, MemoryObject* *ppImage);

		pFnglBindBuffer*			glBindBuffer;
		pFnglMapBuffer*				glMapBuffer;
		pFnglUnmapBuffer*			glUnmapBuffer;
		pFnglGetBufferParameteriv*	glGetBufferParameteriv;
		pFnglBufferData*			glBufferData;
		pFnglTexImage3D*			glTexImage3D;
		pFnglGetRenderbufferParameterivEXT*	glGetRenderbufferParameterivEXT;
		pFnglBindRenderbufferEXT*	glBindRenderbufferEXT;
		pFnglGenFramebuffersEXT*	glGenFramebuffersEXT;
		pFnglDeleteFramebuffersEXT*	glDeleteFramebuffersEXT;
		pFnglBindFramebufferEXT*	glBindFramebufferEXT;
		pFnglFramebufferRenderbufferEXT* glFramebufferRenderbufferEXT;
		pFnglFramebufferTexture2DEXT*	glFramebufferTexture2DEXT;
		pFnglCheckFramebufferStatusEXT*	glCheckFramebufferStatusEXT;
		pFnglGenBuffers*				glGenBuffers;
		pFnglDeleteBuffers*				glDeleteBuffers;

	protected:
		~GLContext();
		cl_context_properties m_hDC;
		cl_context_properties m_hGLCtx;
	};
}}}
