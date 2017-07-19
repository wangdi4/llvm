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


#include "gl_texture1D.h"
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture1D
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE2D, GLTexture1D)
GLTexture1D::GLTexture1D(SharedPtr<Context> pContext, cl_gl_object_type clglObjType) :
	GLTexture(pContext, clglObjType)
{
	m_uiNumDim = 1;
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE1D;
}

cl_err_code GLTexture1D::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	cl_err_code err;

	err = GLTexture::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr, creation_flags);
	if ( CL_FAILED(err) )
	{
		return err;
	}

	// We need to create a frame buffer to read the data from the texture
	// Now we need to create PBO
	m_pGLContext->glGenBuffers(1, &m_glPBO);
	if ( 0 != glGetError() )
	{
		return CL_OUT_OF_RESOURCES;
	}

	// Now we need to create a frame buffer
	m_pGLContext->glGenFramebuffersEXT( 1, &m_glFramebuffer );
	if ( 0 != glGetError() )
	{
		return CL_OUT_OF_RESOURCES;
	}

	return CL_SUCCESS;
}

GLint GLTexture1D::CalculateTextureDimensions()
{
	// We need to create a frame buffer to read the data from the texture
	GLint realWidth, glErr = 0;
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();

	if ( 0 == glErr)
	{
		m_stDimensions[1] = 1;
		m_stDimensions[0] = realWidth;

		m_stPitches[0] = m_stDimensions[0] * m_stElementSize;

		// create buffer for image data
		m_stMemObjSize = m_stPitches[0];
	}

	return 	glErr;
}

cl_err_code GLTexture1D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > m_stDimensions[0])
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

void GLTexture1D::BindFramebuffer2Texture()
{
	m_pGLContext->glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_txtDescriptor.glTextureTarget, m_txtDescriptor.glTexture, m_txtDescriptor.glMipLevel);
}

void GLTexture1D::TexSubImage()
{
	glTexSubImage1D(m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, 0, (GLsizei)m_stDimensions[0], m_glReadBackFormat, m_glReadBackType, nullptr);
}
