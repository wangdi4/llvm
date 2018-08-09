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

#include "gl_texture_array.h"

#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;


///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture3D
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE2D, GLTexture3D)
GLTextureArray::GLTextureArray(SharedPtr<Context> pContext, cl_gl_object_type clglObjType) :
	GLTexture3D(pContext, clglObjType)
{
	switch (clglObjType)
	{
	case CL_GL_OBJECT_TEXTURE1D_ARRAY:
		m_clMemObjectType = CL_MEM_OBJECT_IMAGE1D_ARRAY;
		m_uiNumDim = 2;
		break;
	case CL_GL_OBJECT_TEXTURE2D_ARRAY:
		m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D_ARRAY;
		m_uiNumDim = 3;
		break;
	default:
		assert(0 && "Got unexpected CL_GL_XX image type");
		m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D_ARRAY;
	}
}

GLint GLTextureArray::CalculateTextureDimensions()
{
	GLint realWidth, realHeight, realDepth, glErr = 0;
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_DEPTH, &realDepth );
	glErr |= glGetError();

	if ( 0 == glErr)
	{
		m_stDimensions[0] = realWidth;
		m_stDimensions[1] = realHeight;

		m_stPitches[0] = m_stDimensions[0] * m_stElementSize;
		if ( 3 == m_uiNumDim )
		{
			m_stDimensions[2] = realDepth;
			m_stPitches[1] = m_stPitches[0]*m_stDimensions[1];
		} else
		{
			m_stDimensions[2] = 1;
			m_stPitches[1] = 0;
		}

		assert(m_uiNumDim>=2 && "m_uiNumDim expected to be at least 2");
		// create buffer for image data
		m_stMemObjSize = m_stPitches[m_uiNumDim-2]*m_stDimensions[m_uiNumDim-1];
	}

	return 	glErr;
}

cl_err_code GLTextureArray::GetDimensionSizes(size_t* pszRegion) const
{
    assert(pszRegion);
    if (nullptr == pszRegion)
    {
        return CL_INVALID_VALUE;
    }
    GLTexture::GetDimensionSizes(pszRegion);
    pszRegion[2] = m_stDimensions[2];
    return CL_SUCCESS;
}

void GLTextureArray::FillTextureWithData()
{
	if ( 2 == m_uiNumDim )
	{
		glTexImage2D(m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, m_glInternalFormat,
			(GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], m_glBorder, m_glReadBackFormat, m_glReadBackType, m_pMemObjData );
	}
	else
	{
		m_pGLContext->glTexImage3D( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, m_glInternalFormat,
			(GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], (GLsizei)m_stDimensions[2], m_glBorder, m_glReadBackFormat, m_glReadBackType, m_pMemObjData );
	}
}
