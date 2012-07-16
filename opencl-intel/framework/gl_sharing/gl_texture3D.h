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

        PREPARE_SHARED_PTR(GLTexture3D)

		static SharedPtr<GLTexture3D> Allocate(SharedPtr<Context> pContext, cl_gl_object_type clglObjType)
        {
            return SharedPtr<GLTexture3D>(new GLTexture3D(pContext, clglObjType));
        }

		// GLMemoryObject interface
		cl_err_code AcquireGLObject();
		cl_err_code ReleaseGLObject();

		cl_err_code Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags );

		GLint CalculateTextureDimensions();

        cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

        cl_err_code GetDimensionSizes( size_t* pszRegion ) const;

	protected:

        GLTexture3D(SharedPtr<Context> pContext, cl_gl_object_type clglObjType);

		// do not implement
        GLTexture3D(const GLTexture3D&);
        GLTexture3D& operator=(const GLTexture3D&);

		virtual	void FillTextureWithData();

	};

}}}