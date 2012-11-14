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
#include "gl_texture3D.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class GLTextureArray : public GLTexture3D
	{
	public:
		
        PREPARE_SHARED_PTR(GLTextureArray);

        static SharedPtr<GLTextureArray> Allocate(SharedPtr<Context> pContext, cl_gl_object_type clglObjType)
        {
            return SharedPtr<GLTextureArray>(new GLTextureArray(pContext, clglObjType));
        }

		GLint CalculateTextureDimensions();

        cl_err_code GetDimensionSizes( size_t* pszRegion ) const;

	protected:

        GLTextureArray(SharedPtr<Context> pContext, cl_gl_object_type clglObjType);

		// do not implement
        GLTextureArray(const GLTextureArray&);
        GLTextureArray& operator=(const GLTextureArray&);

		void FillTextureWithData();
	};
}}}