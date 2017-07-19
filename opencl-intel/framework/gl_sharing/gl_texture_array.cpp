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
