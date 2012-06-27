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


#include "gl_mem_objects.h"
#include "ocl_event.h"
#include "memobj_event.h"

#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

GLMemoryObject::GLMemoryObject(Context * pContext, cl_gl_object_type clglObjectType) : 
GraphicsApiMemoryObject(pContext), m_glObjHandle(NULL), m_glMemFlags(0), m_clglObjectType(clglObjectType)
{
	m_pGLContext = static_cast<GLContext*>(pContext);
}

cl_err_code GLMemoryObject::GetGLObjectInfo(cl_gl_object_type * pglObjectType, GLuint * pglObjectName)
{
	if ( NULL != pglObjectType)
	{
		*pglObjectType = m_clglObjectType;
	}

	if ( NULL != pglObjectName)
	{
		*pglObjectName = m_glObjHandle;
	}
	return CL_SUCCESS;
}

cl_err_code GLMemoryObject::GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize)
{
	return CL_INVALID_GL_OBJECT;
}

cl_err_code GLMemoryObject::ReadData(void * pOutData, const size_t *  pszOrigin, const size_t *  pszRegion,
					 size_t          szRowPitch, size_t          szSlicePitch)
{
	if ( NULL == m_pChildObject)
	{
		return CL_INVALID_GL_OBJECT;
	}

	return m_pChildObject->ReadData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

cl_err_code GLMemoryObject::WriteData(	const void * pOutData, const size_t *  pszOrigin, const size_t *  pszRegion,
					  size_t          szRowPitch, size_t          szSlicePitch)
{
	if ( NULL == m_pChildObject)
	{
		return CL_INVALID_GL_OBJECT;
	}

	return m_pChildObject->WriteData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

cl_err_code	GLMemoryObject::SetGLMemFlags()
{
	m_glMemFlags = 0;

	if ( m_clFlags & CL_MEM_READ_ONLY )
		m_glMemFlags |= GL_READ_ONLY;
	if ( m_clFlags & CL_MEM_WRITE_ONLY )
		m_glMemFlags |= GL_WRITE_ONLY;
	if ( m_clFlags & CL_MEM_READ_WRITE )
		m_glMemFlags |= GL_READ_WRITE;

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture
///////////////////////////////////////////////////////////////////////////////////////////////////
GLTexture::~GLTexture()
{
	if ( 0 != m_glFramebuffer )
	{
		m_pGLContext->glDeleteFramebuffersEXT( 1, &m_glFramebuffer );
	}

	if ( 0 != m_glPBO )
	{
		m_pGLContext->glDeleteBuffers( 1, &m_glPBO );
	}

}

cl_err_code	GLTexture::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t  szSize = 0;
	const void * pValue = NULL;
	switch (clParamName)
	{
	case CL_IMAGE_FORMAT:
		szSize = sizeof(cl_image_format);
		pValue = &m_clFormat.clType;
		break;
	case CL_IMAGE_ELEMENT_SIZE:
		szSize = sizeof(size_t);
		pValue = &m_stElementSize;
		break;
	case CL_IMAGE_ROW_PITCH:
		szSize = sizeof(size_t);
		pValue = &m_stPitches[0];
		break;
	case CL_IMAGE_WIDTH:
		szSize = sizeof(size_t);
		pValue = &m_stDimensions[0];
		break;
	case CL_IMAGE_HEIGHT:
		szSize = sizeof(size_t);
		pValue = &m_stDimensions[1];
		break;
	default:
        return CL_INVALID_VALUE;
	}

	if (NULL != pParamValue && szParamValueSize < szSize)
	{
		return CL_INVALID_VALUE;
	}

	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szSize;
	}

	if (NULL != pParamValue && szSize > 0)
	{
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
	}
	
	return CL_SUCCESS;
}

cl_err_code GLTexture::GetDimensionSizes(size_t* pszRegion) const
{
    assert(pszRegion);
    if (NULL == pszRegion)
    {
        return CL_INVALID_VALUE;
    }
    pszRegion[0] = m_stDimensions[0];
    pszRegion[1] = m_stDimensions[1];
    return CL_SUCCESS;
}

cl_err_code GLTexture::GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize)
{
	void* pIntVal;
	size_t intSize;

	switch (glTextInfo)
	{
	case CL_GL_TEXTURE_TARGET:
		pIntVal = &m_txtDescriptor.glTextureTarget;
		intSize = sizeof(m_txtDescriptor.glTextureTarget);
		break;
	case CL_GL_MIPMAP_LEVEL:
		pIntVal = &m_txtDescriptor.glMipLevel;
		intSize = sizeof(m_txtDescriptor.glMipLevel);
		break;
	default:
		return CL_INVALID_VALUE;
	}

	if ( NULL != pRetSize )
	{
		*pRetSize = valSize;
		return CL_SUCCESS;
	}

	if ( valSize < intSize )
	{
		return CL_INVALID_VALUE;
	}

	if ( NULL != pVal)
	{
		memcpy(pVal, pIntVal, valSize);
	}
	return CL_SUCCESS;
}

cl_err_code GLTexture::CreateChildObject()
{
	GLint glBindingType = 0;
	GLint glBind = 0;
	GLint glErr = 0;
	GLint pboBinding;

	if ( (m_clFlags & CL_MEM_READ_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{
		glBindingType = GL_PIXEL_PACK_BUFFER_BINDING_ARB;
		glBind = GL_PIXEL_PACK_BUFFER_ARB;
	} else if ( m_clFlags & CL_MEM_WRITE_ONLY )
	{
		glBindingType = GL_PIXEL_UNPACK_BUFFER_BINDING_ARB;
		glBind = GL_PIXEL_UNPACK_BUFFER_ARB;
	}

	glGetIntegerv(glBindingType, &pboBinding);

	// read pixels from framebuffer to PBO
	// glReadPixels() should return immediately, the transfer is in background by DMA
	m_pGLContext->glBindBuffer(glBind, m_glPBO);
	void *pBuffer = ((GLContext*)m_pContext)->glMapBuffer(glBind, m_glMemFlags);
	if ( NULL == pBuffer )
	{
		((GLContext*)m_pContext)->glBindBuffer(glBind, pboBinding);
        SetAcquireState(CL_INVALID_OPERATION);
		return CL_INVALID_OPERATION;
	}

	// Now we need to create child object
	MemoryObject* pChild;
	cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, m_clMemObjectType, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
	if (CL_FAILED(res))
	{
		((GLContext*)m_pContext)->glUnmapBuffer(glBind);
		((GLContext*)m_pContext)->glBindBuffer(glBind, pboBinding);
		SetAcquireState(res);
		return res;
	}

	size_t dim[] = {m_stDimensions[0], m_stDimensions[1]};
	res = pChild->Initialize(m_clFlags, &m_clFormat.clType, GetNumDimensions(), dim, &m_stPitches[0], pBuffer, CL_RT_MEMOBJ_FORCE_BS);
	if (CL_FAILED(res))
	{
		((GLContext*)m_pContext)->glUnmapBuffer(glBind);
		((GLContext*)m_pContext)->glBindBuffer(glBind, pboBinding);
		pChild->Release();
		SetAcquireState(CL_OUT_OF_RESOURCES);
		return CL_OUT_OF_RESOURCES;
	}

	SetAcquireState(CL_SUCCESS);
	m_pChildObject.exchange(pChild);

	((GLContext*)m_pContext)->glBindBuffer(glBind, pboBinding);

	return CL_SUCCESS;
}

cl_err_code GLTexture::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	GLTextureDescriptor* pTxtDescriptor = (GLTextureDescriptor*)pHostPtr;

	// Retrieve open GL texture information
	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(pTxtDescriptor->glTextureTarget);
	GLint glErr = 0;
	glGetIntegerv(targetBinding, &currTexture);
	glErr |= glGetError();

	GLenum glBaseTarget = GetBaseTarget(pTxtDescriptor->glTextureTarget);
	glBindTexture(glBaseTarget, pTxtDescriptor->glTexture);
	glErr |= glGetError();

	m_txtDescriptor = *pTxtDescriptor;
	m_glObjHandle = m_txtDescriptor.glTexture;

	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_BORDER, &m_glBorder );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat );
	glErr |= glGetError();

	m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_clFormat.clType.image_channel_order)
	{
		glBindTexture(glBaseTarget, currTexture);
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	m_stElementSize = clGetPixelBytesCount(&m_clFormat.clType);

	m_glReadBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
	m_glReadBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

	glErr |= CalculateTextureDimensions();

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

	m_clFlags = clMemFlags;
	SetGLMemFlags();

	return CL_SUCCESS;
}

cl_err_code GLTexture::AcquireGLObject()
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);

	if (NULL != m_pChildObject && CL_SUCCEEDED(GetAcquireState()))
	{
		// We have already acquired object
		return CL_SUCCESS;
	}

	m_muAcquireRelease.Lock();

	if ( (m_clFlags & CL_MEM_READ_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{
		GLint	currFBO;
		GLint glErr = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currFBO);
		glErr |= glGetError();

		// Create and bind a frame buffer to texture
		m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_glFramebuffer );
		if( glGetError() != GL_NO_ERROR )
		{
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		BindFramebuffer2Texture();
		if( glGetError() != GL_NO_ERROR )
		{
			m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		if( m_pGLContext->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT )
		{
			m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);

		// read pixels from framebuffer to PBO
		m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);
		if( glGetError() != GL_NO_ERROR )
		{
			m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		GLenum glUsage = m_clFlags & CL_MEM_READ_ONLY ? GL_STREAM_READ_ARB : GL_STREAM_COPY_ARB;
		m_pGLContext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, m_stMemObjSize, NULL, glUsage );
		if( glGetError() != GL_NO_ERROR )
		{
			m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		// glReadPixels() should return immediately, the transfer is in background by DMA
		glReadPixels(0, 0, (GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], m_glReadBackFormat, m_glReadBackType, 0);
		if( glGetError() != GL_NO_ERROR )
		{
			m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);

		// Bind Old FBO
		m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );

	}
	else if ( m_clFlags & CL_MEM_WRITE_ONLY )
	{
		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB, &pboBinding);

		// bind PBO to update texture source
		m_pGLContext->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, m_glPBO);
		if( glGetError() != GL_NO_ERROR )
		{
			m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
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
		m_pGLContext->glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, m_stMemObjSize, NULL, GL_STREAM_DRAW_ARB);
		if( glGetError() != GL_NO_ERROR )
		{
			m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		m_pGLContext->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pboBinding);
	}

	SetAcquireState(CL_NOT_READY);

	return CL_SUCCESS;
}

cl_err_code GLTexture::ReleaseGLObject()
{
	MemoryObject* pChild = m_pChildObject.exchange(NULL);

	if ( NULL == pChild )
	{
		return CL_INVALID_OPERATION;
	}

	pChild->Release();

	if ( (m_clFlags & CL_MEM_WRITE_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{
		GLenum pboBinding = m_clFlags & CL_MEM_WRITE_ONLY ? GL_PIXEL_UNPACK_BUFFER_BINDING_ARB : GL_PIXEL_PACK_BUFFER_BINDING_ARB;
		GLenum pboTarget = m_clFlags & CL_MEM_WRITE_ONLY ? GL_PIXEL_UNPACK_BUFFER_ARB : GL_PIXEL_PACK_BUFFER_ARB;

		GLint pboCurrent;
		glGetIntegerv(pboBinding, &pboCurrent);

		// bind PBO to update texture source
		m_pGLContext->glBindBuffer(pboTarget, m_glPBO);
		m_pGLContext->glUnmapBuffer(pboTarget);

		// If texture is read/write we need to remap the PBO as an UNPACK buffer
		if ( CL_MEM_READ_WRITE == m_clFlags )
		{
			// Bind old PACK buffer
			m_pGLContext->glBindBuffer(pboTarget, pboCurrent);

			pboBinding = GL_PIXEL_UNPACK_BUFFER_BINDING_ARB;
			pboTarget = GL_PIXEL_UNPACK_BUFFER_ARB;
			glGetIntegerv(pboBinding, &pboCurrent);

			// now bind PBO as UNPACK to update texture source
			m_pGLContext->glBindBuffer(pboTarget, m_glPBO);
		}

		GLenum	targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);

		GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
		GLint	currTexture;
		glGetIntegerv(targetBinding, &currTexture);
		glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

		TexSubImage();

		m_pGLContext->glBindBuffer(pboTarget, pboCurrent);
		glBindTexture(glBaseTarget, currTexture);
	}
	else if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);

		// bind PBO to update texture source
		m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);

		m_pGLContext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
		m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
	}

	return CL_SUCCESS;
}
