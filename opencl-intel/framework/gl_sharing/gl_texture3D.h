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
