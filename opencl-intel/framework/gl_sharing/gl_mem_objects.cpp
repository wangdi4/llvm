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


#include "gl_mem_objects.h"
#include "ocl_event.h"
#include "memobj_event.h"

#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;


struct fmt_cvt
{
	cl_image_format_ext		clType;
	GLuint					glInternalType;
};

fmt_cvt formatConvert[] =
{
	{{{CL_RGBA, CL_UNSIGNED_INT8}, false}, GL_RGBA},
	{{{CL_RGBA, CL_UNORM_INT8}, false}, GL_RGBA8},
	{{{CL_RGBA, CL_UNORM_INT16}, false}, GL_RGBA16},
	{{{CL_RGBA, CL_SIGNED_INT8}, true}, GL_RGBA8I},
	{{{CL_RGBA, CL_SIGNED_INT16}, true}, GL_RGBA16I},
	{{{CL_RGBA, CL_SIGNED_INT32}, true}, GL_RGBA32I},
	{{{CL_RGBA, CL_UNSIGNED_INT8}, true}, GL_RGBA8UI},
	{{{CL_RGBA, CL_UNSIGNED_INT16}, true}, GL_RGBA16UI},
	{{{CL_RGBA, CL_UNSIGNED_INT32}, true}, GL_RGBA32UI},
	{{{CL_RGBA, CL_HALF_FLOAT}, true}, GL_RGBA16F},
	{{{CL_RGBA, CL_FLOAT}, false}, GL_RGBA32F},
	{{{0,0},false}, 0}
};

cl_image_format_ext Intel::OpenCL::Framework::ImageFrmtConvertGL2CL(GLuint glFrmt)
{
	unsigned int i=0;
	while (formatConvert[i].glInternalType != 0)
	{
		if (glFrmt == formatConvert[i].glInternalType)
		{
			return formatConvert[i].clType;
		}
		++i;
	}
	return formatConvert[i].clType;
}

GLuint Intel::OpenCL::Framework::ImageFrmtConvertCL2GL(cl_image_format clFrmt)
{
	unsigned int i=0;
	while (formatConvert[i].glInternalType != 0)
	{
		if ( (clFrmt.image_channel_order == formatConvert[i].clType.clType.image_channel_order) &&
			 (clFrmt.image_channel_data_type == formatConvert[i].clType.clType.image_channel_data_type) )
		{
			return formatConvert[i].glInternalType;
		}
		++i;
	}
	return formatConvert[i].glInternalType;
}

GLenum Intel::OpenCL::Framework::GetTargetBinding( GLenum target )
{
	switch( target )
	{
	case GL_TEXTURE_2D:
		return GL_TEXTURE_BINDING_2D;
	case GL_TEXTURE_RECTANGLE:
		return GL_TEXTURE_BINDING_RECTANGLE;
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	
		return GL_TEXTURE_BINDING_CUBE_MAP;
	default:
		return target;
	}
}

GLenum Intel::OpenCL::Framework::GetBaseTarget( GLenum target )
{
	switch( target )
	{
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	
		return GL_TEXTURE_CUBE_MAP;
	default:
		return target;
	}
}

GLenum Intel::OpenCL::Framework::GetGLType(cl_channel_type clType)
{
	switch (clType)
	{
	case CL_UNORM_INT8:
		return GL_UNSIGNED_BYTE;
	case CL_UNORM_INT16:
		return GL_UNSIGNED_SHORT;
	case CL_SIGNED_INT8:
		return GL_BYTE;
	case CL_SIGNED_INT16:
		return GL_SHORT;
	case CL_SIGNED_INT32:
		return GL_INT;
	case CL_UNSIGNED_INT8:
		return GL_UNSIGNED_BYTE;
	case CL_UNSIGNED_INT16:
		return GL_UNSIGNED_SHORT;
	case CL_UNSIGNED_INT32:
		return GL_UNSIGNED_INT;
	case CL_FLOAT:
		return GL_FLOAT;
	}
	return 0;
}

GLenum Intel::OpenCL::Framework::GetGLFormat(cl_channel_type clType, bool isExt)
{
	if ( isExt )
	{
		return GL_RGBA_INTEGER_EXT;
	}
	else
	{
		return GL_RGBA;
	}

	/*
	switch (clType)
	{
	case CL_UNORM_INT8:
	case CL_UNORM_INT16:
		return GL_RGBA;
	case CL_SIGNED_INT8:
	case CL_SIGNED_INT16:
	case CL_SIGNED_INT32:
	case CL_UNSIGNED_INT8:
	case CL_UNSIGNED_INT16:
	case CL_UNSIGNED_INT32:
	case CL_HALF_FLOAT:
		return GL_RGBA_INTEGER_EXT;
	case CL_FLOAT:
		return GL_RGBA;
	}
	*/
	return 0;
}

GLMemoryObject::GLMemoryObject(Context * pContext, ocl_entry_points * pOclEntryPoints) : 
GraphicsApiMemoryObject(pContext, pOclEntryPoints), m_glObjHandle(NULL), m_glMemFlags(0)
{
}

cl_err_code GLMemoryObject::GetGLObjectInfo(cl_gl_object_type * pglObjectType, GLuint * pglObjectName)
{
	if ( NULL != pglObjectType)
	{
		*pglObjectType = GetObjectType();
	}

	if ( NULL != pglObjectName)
	{
		*pglObjectName = m_glObjHandle;
	}
	return CL_SUCCESS;
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
	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	if ( 0 != m_glFramebuffer )
	{
		pGLContext->glDeleteFramebuffersEXT( 1, &m_glFramebuffer );
	}

	if ( 0 != m_glPBO )
	{
		pGLContext->glDeleteBuffers( 1, &m_glPBO );
	}

}

cl_err_code	GLTexture::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t  szSize = 0;
	void * pValue = NULL;
	switch (clParamName)
	{
	case CL_IMAGE_FORMAT:
		szSize = sizeof(cl_image_format);
		pValue = &m_clFormat.clType;
		break;
	case CL_IMAGE_ELEMENT_SIZE:
		szSize = sizeof(size_t);
		pValue = &m_szElementSize;
		break;
	case CL_IMAGE_ROW_PITCH:
		szSize = sizeof(size_t);
		pValue = &m_szImageRowPitch;
		break;
	case CL_IMAGE_WIDTH:
		szSize = sizeof(size_t);
		pValue = &m_szImageWidth;
		break;
	case CL_IMAGE_HEIGHT:
		szSize = sizeof(size_t);
		pValue = &m_szImageHeight;
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

	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

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
	pGLContext->glBindBuffer(glBind, m_glPBO);
	void *pBuffer = ((GLContext*)m_pContext)->glMapBuffer(glBind, m_glMemFlags);
	if ( NULL == pBuffer )
	{
		((GLContext*)m_pContext)->glBindBuffer(glBind, pboBinding);
        SetAcquireState(CL_INVALID_OPERATION);
		return CL_INVALID_OPERATION;
	}

	// Now we need to create child object
	MemoryObject* pChild;
	cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, CL_MEM_OBJECT_IMAGE2D, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
	if (CL_FAILED(res))
	{
		((GLContext*)m_pContext)->glUnmapBuffer(glBind);
		((GLContext*)m_pContext)->glBindBuffer(glBind, pboBinding);
		SetAcquireState(res);
		return res;
	}

	size_t dim[] = {m_szImageWidth, m_szImageHeight};
	res = pChild->Initialize(m_clFlags, &m_clFormat.clType, GetNumDimensions(), dim, &m_szImageRowPitch, pBuffer);
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
