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

#include "gl_context.h"
#include "gl_texture2D.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class GLTexture3D : public GLTexture2D
	{
	public:
		GLTexture3D(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType):
		  GLTexture2D(pContext, pOclEntryPoints, clObjType) {};

		// GLMemoryObject interface
		cl_err_code AcquireGLObject();
		cl_err_code ReleaseGLObject();
		cl_gl_object_type GetObjectType() {return CL_GL_OBJECT_TEXTURE3D;}

		// MemoryObject interface
		cl_err_code Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr);

		size_t GetSlicePitchSize() const  { return m_szImageSlicePitch;}

        cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

	protected:
		size_t	m_szImageDepth;
		size_t	m_szImageSlicePitch;

		// do not implement
        GLTexture3D(const GLTexture3D&);
        GLTexture3D& operator=(const GLTexture3D&);
	};

}}}