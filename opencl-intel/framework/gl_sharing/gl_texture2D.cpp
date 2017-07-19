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


#include "gl_texture2D.h"
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture2D
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE2D, GLTexture2D)
GLTexture2D::GLTexture2D(SharedPtr<Context> pContext, cl_gl_object_type clglObjType) :
	GLTexture1D(pContext, clglObjType)
{
	m_uiNumDim = 2;
	m_clMemObjectType = CL_GL_OBJECT_TEXTURE1D_ARRAY == clglObjType ? CL_MEM_OBJECT_IMAGE1D_ARRAY : CL_MEM_OBJECT_IMAGE2D;
}

GLint GLTexture2D::CalculateTextureDimensions()
{
	GLint realWidth, realHeight, glErr = 0;
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();

	if ( 0 == glErr)
	{
		m_stDimensions[0] = realWidth;
		m_stDimensions[1] = realHeight;

		m_stPitches[0] = m_stDimensions[0] * m_stElementSize;

		// create buffer for image data
		m_stMemObjSize = m_stPitches[0] * m_stDimensions[1];
	}

	return 	glErr;
}

cl_err_code GLTexture2D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > m_stDimensions[0] ||
        pszOrigin[1] + pszRegion[1] > m_stDimensions[1])
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

void GLTexture2D::BindFramebuffer2Texture()
{
	m_pGLContext->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_txtDescriptor.glTextureTarget, m_txtDescriptor.glTexture, m_txtDescriptor.glMipLevel);
}

void GLTexture2D::TexSubImage()
{
	glTexSubImage2D(m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, 0, 0, (GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], m_glReadBackFormat, m_glReadBackType, nullptr);
}
