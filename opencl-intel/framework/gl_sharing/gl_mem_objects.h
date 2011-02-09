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

#pragma once

#include "cl_buffer.h"
#include "image.h"
#include "gl_context.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class GLMemoryObject
	{
	public:
		GLMemoryObject(GLuint glObj) : m_glBufObj(glObj) {}
		virtual void GetGLObjectData() = 0;
		virtual void SetGLObjectData() = 0;
		virtual cl_gl_object_type GetObjectType() = 0;
		cl_err_code GetGLObjectInfo(cl_gl_object_type * pglObjectType, GLuint * pglObjectName);

	protected:
		GLuint m_glBufObj;
	};

	class GLBuffer : public Buffer, public GLMemoryObject
	{
	public:
		GLBuffer(GLContext * pContext, cl_mem_flags clMemFlags, GLuint glBufObj,
			ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode);

		void GetGLObjectData();
		void SetGLObjectData();
		cl_gl_object_type GetObjectType() {return CL_GL_OBJECT_BUFFER;}
	};

	class GLTexture : public GLMemoryObject
	{
	public:
		GLTexture(GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture):
		  GLMemoryObject(glTexture), m_glTextureTarget(glTextureTarget),
		  m_glMipLevel(glMipLevel){}

		  cl_err_code GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize);

	protected:
		GLenum	m_glTextureTarget;
		GLint	m_glMipLevel;
		GLint	m_glBorder;
		GLint	m_glInternalFormat;
	};

	class GLTexture2D : public Image2D, public GLTexture
	{
	public:
		GLTexture2D(GLContext * pContext, cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture,
			ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode);

		void GetGLObjectData();
		void SetGLObjectData();
		cl_gl_object_type GetObjectType() {return CL_GL_OBJECT_TEXTURE2D;}
	};

	class GLTexture3D : public Image3D, public GLTexture
	{
	public:
		GLTexture3D(GLContext * pContext, cl_mem_flags clMemFlags,
			GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture,
			ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode);

		void GetGLObjectData();
		void SetGLObjectData();
		cl_gl_object_type GetObjectType() {return CL_GL_OBJECT_TEXTURE3D;}
	};

	class GLRenderBuffer : public Image2D, public GLMemoryObject
	{
	public:
		GLRenderBuffer(GLContext * pContext, cl_mem_flags clMemFlags, GLuint glBufObj,
			ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode);
		~GLRenderBuffer();

		void GetGLObjectData();
		void SetGLObjectData();
		cl_gl_object_type GetObjectType() {return CL_GL_OBJECT_RENDERBUFFER;}
	protected:
		GLint	m_glInternalFormat;
		GLuint	m_glFramebuffer;
	};
}}}