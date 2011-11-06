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

cl_err_code GLTexture2D::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr)
{
	GLTextureDescriptor* pTxtDescriptor = (GLTextureDescriptor*)pHostPtr;

	// Retrieve open GL buffer size
	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(pTxtDescriptor->glTextureTarget);
	GLint glErr = 0;
	glGetIntegerv(targetBinding, &currTexture);
	glErr |= glGetError();

	GLenum glBaseTarget = GetBaseTarget(pTxtDescriptor->glTextureTarget);
	glBindTexture(glBaseTarget, pTxtDescriptor->glTexture);
	glErr |= glGetError();

	// Read results from the GL texture
	GLint realWidth, realHeight;
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_BORDER, &m_glBorder );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat );
	glErr |= glGetError();
	glBindTexture(glBaseTarget, currTexture);
	glErr |= glGetError();

	if ( 0 != glErr)
	{
		return CL_INVALID_GL_OBJECT;
	}

	if (0 == m_glInternalFormat)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	// Setup internal parameters
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D;

	m_szImageWidth = realWidth;
	m_szImageHeight = realHeight;

	m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_clFormat.clType.image_channel_order)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	// We need to create a frame buffer to read the data from the texture
	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	// Now we need to create PBO
	pGLContext->glGenBuffers(1, &m_glPBO);
	if ( 0 != glGetError() )
	{
		return CL_OUT_OF_RESOURCES;
	}

	m_clFlags = clMemFlags;
	SetGLMemFlags();

	if ( (m_clFlags & CL_MEM_READ_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{

		// Now we need to create a frame buffer
		pGLContext->glGenFramebuffersEXT( 1, &m_glFramebuffer );
		if ( 0 != glGetError() )
		{
			return CL_OUT_OF_RESOURCES;
		}
	}

	m_uiNumDim = 2;
	m_szElementSize = m_pContext->GetPixelBytesCount(&m_clFormat.clType);
	m_szImageRowPitch = m_szImageWidth * m_szElementSize;

	// create buffer for image data
	m_stMemObjSize = m_szImageRowPitch * m_szImageHeight;

	m_txtDescriptor = *pTxtDescriptor;
	m_glObjHandle = m_txtDescriptor.glTexture;

	return CL_SUCCESS;
}

cl_err_code GLTexture2D::AcquireGLObject()
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);

	if (NULL != m_pChildObject && CL_SUCCEEDED(GetAcquireState()))
	{
		// We have already acquired object
		return CL_SUCCESS;
	}

	m_muAcquireRelease.Lock();

	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	if ( (m_clFlags & CL_MEM_READ_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{
		GLenum readBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
		GLenum readBackType = GetGLType(m_clFormat.clType.image_channel_data_type); 
	
		GLint	currFBO;
		GLint glErr = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currFBO);
		glErr |= glGetError();

		// Create and bind a frame buffer to texture
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_glFramebuffer );
		if( glGetError() != GL_NO_ERROR )
		{
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		pGLContext->glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_txtDescriptor.glTextureTarget, m_txtDescriptor.glTexture, m_txtDescriptor.glMipLevel);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		if( pGLContext->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT )
		{
			pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);

		// read pixels from framebuffer to PBO
		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		GLenum glUsage = m_clFlags & CL_MEM_READ_ONLY ? GL_STREAM_READ_ARB : GL_STREAM_COPY_ARB;
		pGLContext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, m_stMemObjSize, NULL, glUsage );
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		// glReadPixels() should return immediately, the transfer is in background by DMA
		glReadPixels(0, 0, (GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, readBackFormat, readBackType, 0);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);

		// Bind Old FBO
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );

	}
	else if ( m_clFlags & CL_MEM_WRITE_ONLY )
	{
		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB, &pboBinding);

		// bind PBO to update texture source
		pGLContext->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, m_glPBO);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		// Note that glMapBufferARB() causes sync issue.
		// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
		// until GPU to finish its job. To avoid waiting (idle), you can call
		// first glBufferDataARB() with NULL pointer before glMapBufferARB().
		// If you do that, the previous data in PBO will be discarded and
		// glMapBufferARB() returns a new allocated pointer immediately
		// even if GPU is still working with the previous data.
		pGLContext->glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, m_stMemObjSize, NULL, GL_STREAM_DRAW_ARB);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		pGLContext->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pboBinding);
	}

	SetAcquireState(CL_NOT_READY);

	return CL_SUCCESS;
}

cl_err_code GLTexture2D::ReleaseGLObject()
{
	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);
	MemoryObject* pChild = m_pChildObject.exchange(NULL);

	if ( NULL == pChild )
	{
		return CL_INVALID_OPERATION;
	}

	pChild->Release();

	// Update texture context
	GLenum readBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
	GLenum readBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

	if ( (m_clFlags & CL_MEM_WRITE_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{
		GLenum pboBinding = m_clFlags & CL_MEM_WRITE_ONLY ? GL_PIXEL_UNPACK_BUFFER_BINDING_ARB : GL_PIXEL_PACK_BUFFER_BINDING_ARB;
		GLenum pboTarget = m_clFlags & CL_MEM_WRITE_ONLY ? GL_PIXEL_UNPACK_BUFFER_ARB : GL_PIXEL_PACK_BUFFER_ARB;

		GLint pboCurrent;
		glGetIntegerv(pboBinding, &pboCurrent);

		// bind PBO to update texture source
		pGLContext->glBindBuffer(pboTarget, m_glPBO);
		pGLContext->glUnmapBuffer(pboTarget);

		// If texture is read/write we need to remap the PBO as an UNPACK buffer
		if ( CL_MEM_READ_WRITE == m_clFlags )
		{
			// Bind old PACK buffer
			pGLContext->glBindBuffer(pboTarget, pboCurrent);

			pboBinding = GL_PIXEL_UNPACK_BUFFER_BINDING_ARB;
			pboTarget = GL_PIXEL_UNPACK_BUFFER_ARB;
			glGetIntegerv(pboBinding, &pboCurrent);

			// now bind PBO as UNPACK to update texture source
			pGLContext->glBindBuffer(pboTarget, m_glPBO);
		}

		GLenum	targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);

		GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
		GLint	currTexture;
		glGetIntegerv(targetBinding, &currTexture);
		glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

		glTexSubImage2D(m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, 0, 0, (GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, readBackFormat, readBackType, NULL);

		pGLContext->glBindBuffer(pboTarget, pboCurrent);
		glBindTexture(glBaseTarget, currTexture);
	}
	else if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);

		// bind PBO to update texture source
		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);

		pGLContext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
	}

	return CL_SUCCESS;
}

cl_err_code GLTexture2D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > m_szImageWidth ||
        pszOrigin[1] + pszRegion[1] > m_szImageHeight)
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}
