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

#include "gl_mem_objects.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class GLRenderBuffer : public GLTexture
	{
	public:
		GLRenderBuffer::GLRenderBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints):
			GLTexture(pContext, pOclEntryPoints)
			{
			}
		~GLRenderBuffer();

		cl_err_code AcquireGLObject();
		cl_err_code ReleaseGLObject();
		cl_gl_object_type GetObjectType() {return CL_GL_OBJECT_RENDERBUFFER;}

		// MemoryObject Interface
		cl_err_code Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr);

		size_t GetSlicePitchSize() const  { return 0;}

	protected:
		// do not implement
        GLRenderBuffer(const GLRenderBuffer&);
        GLRenderBuffer& operator=(const GLRenderBuffer&);
	};
}}}