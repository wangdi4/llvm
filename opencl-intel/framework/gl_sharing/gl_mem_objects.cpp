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

GLMemoryObject::GLMemoryObject(SharedPtr<Context> pContext, cl_gl_object_type clglObjectType) : 
GraphicsApiMemoryObject(pContext), m_glObjHandle(NULL), m_glMemFlags(0), m_clglObjectType(clglObjectType)
{
    m_pGLContext = pContext.DynamicCast<GLContext>();
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
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
	if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
	{
		return CL_INVALID_GL_OBJECT;
	}

	return m_itCurrentAcquriedObject->second->ReadData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

cl_err_code GLMemoryObject::WriteData(	const void * pOutData, const size_t *  pszOrigin, const size_t *  pszRegion,
					  size_t          szRowPitch, size_t          szSlicePitch)
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
	if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
	{
		return CL_INVALID_GL_OBJECT;
	}

	return m_itCurrentAcquriedObject->second->WriteData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
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
	SharedPtr<GLContext> pGLContext = m_pContext.DynamicCast<GLContext>();

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
	cl_uint uiZero = 0;
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
	case CL_IMAGE_NUM_SAMPLES:
		szSize = sizeof(cl_uint);
		pValue = &uiZero;
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

void GLTexture::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
{
    if (NULL != dimensions)
    {
        GetDimensionSizes(dimensions);
    }
    if (NULL != rowPitch)
    {
        *rowPitch = GetRowPitchSize();
    }
    if (NULL != slicePitch)
    {
        *slicePitch = GetSlicePitchSize();
    }
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

	if ( (NULL != pVal && valSize < intSize) ||
		 (NULL == pVal && NULL == pRetSize) )
	{
		return CL_INVALID_VALUE;
	}

	if ( NULL != pRetSize )
	{
		*pRetSize = intSize;
	}

	if ( NULL != pVal)
	{
		MEMCPY_S(pVal, valSize, pIntVal, intSize);
	}
	return CL_SUCCESS;
}

cl_err_code GLTexture::CreateChildObject()
{
    GLint glErr = 0;
	GLint glBindingType = 0;
	GLint glBind = 0;	
	GLint pboBinding;

	SharedPtr<GLContext> pGLContext = m_pContext.DynamicCast<GLContext>();
	GLContext::GLContextSync sync(pGLContext.GetPtr());

    glBindingType = GL_PIXEL_PACK_BUFFER_BINDING_ARB;
	glBind = GL_PIXEL_PACK_BUFFER_ARB;

	glGetIntegerv(glBindingType, &pboBinding);
	glErr = glGetError();
	assert( (GL_NO_ERROR==glErr) && "Failed to retrieved current PBO" );
	if ( GL_NO_ERROR!=glErr )
	{
		return CL_INVALID_OPERATION;
	}

	// read pixels from framebuffer to PBO
	// glReadPixels() should return immediately, the transfer is in background by DMA
	m_pGLContext->glBindBuffer(glBind, m_glPBO);
	glErr = glGetError();
	assert( (GL_NO_ERROR==glErr) && "Failed to bind PBO" );
	if ( GL_NO_ERROR!=glErr )
	{
		return CL_INVALID_OPERATION;
	}

	void *pBuffer = pGLContext->glMapBuffer(glBind, m_glMemFlags);
	if ( NULL == pBuffer )
	{
		assert( 0 && "Map failed, buffer == NULL" );
		pGLContext->glBindBuffer(glBind, pboBinding);
		m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
		return CL_INVALID_OPERATION;
	}

	// Now we need to create child object
	SharedPtr<MemoryObject> pChild;
    cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, m_clMemObjectType, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
	if (CL_FAILED(res))
	{
		assert( 0 && "Failed to create a new object" );
		pGLContext->glUnmapBuffer(glBind);
		pGLContext->glBindBuffer(glBind, pboBinding);
		m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
		return res;
	}

	size_t dim[] = {m_stDimensions[0], m_stDimensions[1]};
	res = pChild->Initialize(m_clFlags, &m_clFormat.clType, GetNumDimensions(), dim, &m_stPitches[0], pBuffer, CL_RT_MEMOBJ_FORCE_BS);
	if (CL_FAILED(res))
	{
		assert( 0 && "Failed to initialize a new object" );
		pGLContext->glUnmapBuffer(glBind);
		pGLContext->glBindBuffer(glBind, pboBinding);
		pChild->Release();
		m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
		return CL_OUT_OF_RESOURCES;
	}

	m_itCurrentAcquriedObject->second = pChild;
	pGLContext->glBindBuffer(glBind, pboBinding);

	glErr = glGetError();
	assert( (GL_NO_ERROR==glErr) && "Failed to bind old PBO" );

	return GL_NO_ERROR==glErr ? CL_SUCCESS : CL_INVALID_OPERATION;
}

cl_err_code GLTexture::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	GLTextureDescriptor* pTxtDescriptor = (GLTextureDescriptor*)pHostPtr;

	assert(0!=pTxtDescriptor->glTexture && "Invalid texture descriptor == 0");
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
	assert ( (GL_NO_ERROR==glErr) && "Failed to retrive texture information");

	m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_clFormat.clType.image_channel_order)
	{
        LOG_ERROR(TEXT("Can't match CL format, glTextureTarget=%d, glTexture=%d, glInternalFormat=%d"),
			pTxtDescriptor->glTextureTarget, pTxtDescriptor->glTexture, m_glInternalFormat);
		assert(0 && "Can't match texture format");
		glBindTexture(glBaseTarget, currTexture);
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	m_stElementSize = clGetPixelBytesCount(&m_clFormat.clType);

	m_glReadBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
	m_glReadBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

	LOG_INFO("glTexture=%d, glInternalFormat=0x%X, clType.image_channel_data_type=0x%x, clType.image_channel_order=0x%x, ReadBackFormat=0x%x ReadBackType=0x%x",
		m_txtDescriptor.glTexture, m_glInternalFormat, m_clFormat.clType.image_channel_data_type, m_clFormat.clType.image_channel_order, m_glReadBackFormat, m_glReadBackType);


	glErr = CalculateTextureDimensions();
	assert ( (GL_NO_ERROR==glErr) && "Failed to calculate texture dimensions");

	glBindTexture(glBaseTarget, currTexture);
	glErr |= glGetError();

	if ( GL_NO_ERROR != glErr)
	{
		assert(0 && "Failed in binding original texture");
		return CL_INVALID_GL_OBJECT;
	}

	if (0 == m_glInternalFormat)
	{
		assert(0 && "GL internal format is not recognized");
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	m_clFlags = clMemFlags;
	SetGLMemFlags();

	return CL_SUCCESS;
}

cl_err_code GLTexture::AcquireGLObject()
{
    Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);

    if ( m_lstAcquiredObjectDescriptors.end() != m_itCurrentAcquriedObject && 
          ( (CL_GFX_OBJECT_NOT_ACQUIRED != m_itCurrentAcquriedObject->second) &&
            (CL_GFX_OBJECT_NOT_READY != m_itCurrentAcquriedObject->second) &&
        (CL_GFX_OBJECT_FAIL_IN_ACQUIRE != m_itCurrentAcquriedObject->second) )
        )
    {
        // We have already acquired object
        return CL_SUCCESS;
    }

    GLint	currFBO;
    GLint glErr = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currFBO);
    glErr |= glGetError();

    // Create and bind a frame buffer to texture
    m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_glFramebuffer );
    if( glGetError() != GL_NO_ERROR )
    {
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    BindFramebuffer2Texture();
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }
    if( m_pGLContext->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT )
    {
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    GLint pboBinding;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);
    assert( glGetError() == GL_NO_ERROR && "error after glFlush" );

    // read pixels from framebuffer to PBO
    m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }
    GLenum glUsage = m_clFlags & CL_MEM_READ_ONLY ? GL_STREAM_READ_ARB : GL_STREAM_COPY_ARB;
    m_pGLContext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, m_stMemObjSize, NULL, glUsage );
    if( (glErr = glGetError()) != GL_NO_ERROR )
    {
        m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }
    // glReadPixels() should return immediately, the transfer is in background by DMA
    glReadPixels(0, 0, (GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], m_glReadBackFormat, m_glReadBackType, 0);
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);

    // Bind Old FBO
    m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currFBO );

    m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_NOT_READY;

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLTexture::ReleaseGLObject()
{
    GLint pboCurrent;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboCurrent);

    // unmap the PBO
    m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);
    m_pGLContext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboCurrent);

    if ( (m_clFlags & CL_MEM_WRITE_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
    {
        // If texture has WRITE memory flag then we need to update the texture in GL
        // Bind PBO as UNPACK then TexSubImage will do the updating from it
        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB, &pboCurrent);

        m_pGLContext->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, m_glPBO);

        GLenum targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);

        GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
        GLint currTexture;
        glGetIntegerv(targetBinding, &currTexture);
        glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

        TexSubImage();

        m_pGLContext->glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pboCurrent);
        glBindTexture(glBaseTarget, currTexture);
    }

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}
