// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "gl_texture1D.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class GLTexture2D : public GLTexture1D
	{
	public:
		
        PREPARE_SHARED_PTR(GLTexture2D)

        static SharedPtr<GLTexture2D> Allocate(SharedPtr<Context> pContext, cl_gl_object_type clglObjType)
        {
            return SharedPtr<GLTexture2D>(new GLTexture2D(pContext, clglObjType));
        }

		GLint CalculateTextureDimensions();

        cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

	protected:

        GLTexture2D(SharedPtr<Context> pContext, cl_gl_object_type clglObjType);

		// do not implement
        GLTexture2D(const GLTexture2D&);
        GLTexture2D& operator=(const GLTexture2D&);

		void BindFramebuffer2Texture();
		void TexSubImage();

	};

}}}
