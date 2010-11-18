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
#include "cl_memory_object.h"
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

typedef struct _fmt_cvt
{
	cl_image_format clType;
	GLuint			glInternalType;
} fmt_cvt;

fmt_cvt formatConvert[] =
{
	{{CL_RGBA, CL_UNSIGNED_INT8}, GL_RGBA},
	{{CL_RGBA, CL_UNORM_INT8}, GL_RGBA8},
	{{CL_RGBA, CL_UNORM_INT16}, GL_RGBA16},
	{{CL_RGBA, CL_SIGNED_INT8}, GL_RGBA8I},
	{{CL_RGBA, CL_SIGNED_INT16}, GL_RGBA16I},
	{{CL_RGBA, CL_SIGNED_INT32}, GL_RGBA32I},
	{{CL_RGBA, CL_UNSIGNED_INT8}, GL_RGBA8UI},
	{{CL_RGBA, CL_UNSIGNED_INT16}, GL_RGBA16UI},
	{{CL_RGBA, CL_UNSIGNED_INT32}, GL_RGBA32UI},
	{{CL_RGBA, CL_HALF_FLOAT}, GL_RGBA16F},
	{{CL_RGBA, CL_FLOAT}, GL_RGBA32F},
	{{0,0}, 0}
};

cl_image_format ImageFrmtConvertGL2CL(GLuint glFrmt)
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

GLuint ImageFrmtConvertCL2GL(cl_image_format clFrmt)
{
	unsigned int i=0;
	while (formatConvert[i].glInternalType != 0)
	{
		if ( (clFrmt.image_channel_order == formatConvert[i].clType.image_channel_order) &&
			 (clFrmt.image_channel_data_type == formatConvert[i].clType.image_channel_data_type) )
		{
			return formatConvert[i].glInternalType;
		}
		++i;
	}
	return formatConvert[i].glInternalType;
}

GLenum GetTargetBinding( GLenum target )
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

GLenum GetBaseTarget( GLenum target )
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

GLenum GetGLType(cl_channel_type clType)
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

GLenum GetGLFormat(cl_channel_type clType)
{
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
	return 0;
}

cl_err_code GLMemoryObject::GetGLObjectInfo(cl_gl_object_type * pglObjectType, GLuint * pglObjectName)
{
	if ( NULL != pglObjectType)
	{
		*pglObjectType = GetObjectType();
	}

	if ( NULL != pglObjectName)
	{
		*pglObjectName = m_glBufObj;
	}
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLBuffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GLBuffer::GLBuffer(GLContext * pContext, cl_mem_flags clMemFlags, GLuint glBufObj, ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode) :
	Buffer(pContext, clMemFlags, 0, pOclEntryPoints, pErrCode), GLMemoryObject(glBufObj)
{
	assert ( NULL != pErrCode );
	if (CL_FAILED(*pErrCode))
	{
		return;
	}

	// Retrieve open GL buffer size
	GLint	currBuff;
	GLint	buffSize;
	GLint	glErr = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuff);
	glErr |= glGetError();
	pContext->glBindBuffer(GL_ARRAY_BUFFER, glBufObj);
	glErr |= glGetError();
	pContext->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffSize);
	glErr |= glGetError();
	pContext->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
	glErr |= glGetError();

	if ( 0 != glErr )
	{
		*pErrCode = CL_INVALID_GL_OBJECT;
		return;
	}
	m_clMemObjectType = CL_MEM_OBJECT_BUFFER;

	m_szMemObjSize = buffSize;

	*pErrCode = CL_SUCCESS;
}

void GLBuffer::GetGLObjectData()
{
	GLint	currBuff;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuff);
	((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, m_glBufObj);
	void *pBuffer = ((GLContext*)m_pContext)->glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	if ( NULL == pBuffer )
	{
		if ( 0 != currBuff )
		{
			((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
		}
		return;
	}

	memcpy_s(m_pMemObjData, m_szMemObjSize, pBuffer, m_szMemObjSize);
	m_lDataOnHost = 1;
	((GLContext*)m_pContext)->glUnmapBuffer(GL_ARRAY_BUFFER);
	if ( 0 != currBuff )
	{
		((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
	}
}

void GLBuffer::SetGLObjectData()
{
	if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		return;
	}
	GLint	currBuff;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuff);
	((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, m_glBufObj);
	void *pBuffer = ((GLContext*)m_pContext)->glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if ( NULL == pBuffer )
	{
		if ( 0 != currBuff )
		{
			((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
		}
		return;
	}

	memcpy_s(pBuffer, m_szMemObjSize, m_pMemObjData, m_szMemObjSize);
	((GLContext*)m_pContext)->glUnmapBuffer(GL_ARRAY_BUFFER);
	if ( 0 != currBuff )
	{
		((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture
///////////////////////////////////////////////////////////////////////////////////////////////////

cl_err_code GLTexture::GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize)
{
	void* pIntVal;
	size_t intSize;

	switch (glTextInfo)
	{
	case CL_GL_TEXTURE_TARGET:
		pIntVal = &m_glTextureTarget;
		intSize = sizeof(m_glTextureTarget);
		break;
	case CL_GL_MIPMAP_LEVEL:
		pIntVal = &m_glMipLevel;
		intSize = sizeof(m_glMipLevel);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture2D C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GLTexture2D::GLTexture2D(GLContext * pContext, cl_mem_flags clMemFlags,
						 GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, 
						 ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode) :
	Image2D(pContext, clMemFlags, NULL, NULL, 0, 0, 0, pOclEntryPoints, pErrCode),
	GLTexture(glTextureTarget, glMipLevel, glTexture)
{
	// Retrieve open GL buffer size
	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(glTextureTarget);
	GLint glErr = 0;
	glGetIntegerv(targetBinding, &currTexture);
	glErr |= glGetError();

	GLenum glBaseTarget = GetBaseTarget(glTextureTarget);
	glBindTexture(glBaseTarget, glTexture);
	glErr |= glGetError();

	// Read results from the GL texture
	GLint realWidth, realHeight;
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_BORDER, &m_glBorder );
	glErr |= glGetError();
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat );
	glErr |= glGetError();
	glBindTexture(glBaseTarget, currTexture);
	glErr |= glGetError();

	if ( 0 != glErr)
	{
		*pErrCode = CL_INVALID_GL_OBJECT;
		return;
	}

	if (0 == m_glInternalFormat)
	{
		*pErrCode = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		return;
	}
	// Setup internal parameters
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D;

	m_szImageWidth = realWidth;
	m_szImageHeight = realHeight;

	// set image format (copy data);
	m_pclImageFormat = new cl_image_format();
	*m_pclImageFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_pclImageFormat->image_channel_order)
	{
		*pErrCode = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		return;
	}

	m_szImageRowPitch = m_szImageWidth * GetPixelBytesCount(m_pclImageFormat);

	// create buffer for image data
	m_szMemObjSize = CalcImageSize();

	*pErrCode = CL_SUCCESS;
}

void GLTexture2D::GetGLObjectData()
{
	GLenum readBackFormat = GetGLFormat(m_pclImageFormat->image_channel_data_type);
	GLenum readBackType = GetGLType(m_pclImageFormat->image_channel_data_type); 

	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(m_glTextureTarget);
	glGetIntegerv(targetBinding, &currTexture);

	GLenum glBaseTarget = GetBaseTarget(m_glTextureTarget);
	glBindTexture(glBaseTarget, m_glBufObj);

	glGetTexImage( m_glTextureTarget, m_glMipLevel, readBackFormat, readBackType, m_pMemObjData );

	glBindTexture(glBaseTarget, currTexture);
}

void GLTexture2D::SetGLObjectData()
{
	if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		return;
	}

	GLenum readBackFormat = GetGLFormat(m_pclImageFormat->image_channel_data_type);
	GLenum readBackType = GetGLType(m_pclImageFormat->image_channel_data_type); 

	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(m_glTextureTarget);
	glGetIntegerv(targetBinding, &currTexture);

	GLenum glBaseTarget = GetBaseTarget(m_glTextureTarget);
	glBindTexture(glBaseTarget, m_glBufObj);

	if( glBaseTarget == GL_TEXTURE_CUBE_MAP )
	{
		glTexImage2D( m_glTextureTarget, m_glMipLevel, m_glInternalFormat,
			(GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, m_glBorder, readBackFormat, readBackType, m_pMemObjData );
	}
	else
	{
		glTexImage2D( m_glTextureTarget, m_glMipLevel, m_glInternalFormat,
			(GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, m_glBorder, readBackFormat, readBackType, m_pMemObjData );        
	}

	glBindTexture(glBaseTarget, currTexture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLTexture3D C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GLTexture3D::GLTexture3D(GLContext * pContext, cl_mem_flags clMemFlags,
						 GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, 
						 ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode) :
Image3D(pContext, clMemFlags, NULL, NULL, 0, 0, 0, 0, 0, pOclEntryPoints, pErrCode),
GLTexture(glTextureTarget, glMipLevel, glTexture)
{
	// Retrieve open GL buffer size
	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(glTextureTarget);
	GLint glErr = 0;
	glGetIntegerv(targetBinding, &currTexture);
	glErr |= glGetError();

	GLenum glBaseTarget = GetBaseTarget(glTextureTarget);
	glBindTexture(glBaseTarget, glTexture);
	glErr |= glGetError();

	// Read results from the GL texture
	GLint realWidth, realHeight, realDepth;
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_DEPTH, &realDepth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_BORDER, &m_glBorder);

	glGetTexLevelParameteriv( glTextureTarget, glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat );
	glErr |= glGetError();

	glBindTexture(glBaseTarget, currTexture);
	glErr |= glGetError();

	if ( 0 != glErr )
	{
		*pErrCode = CL_INVALID_GL_OBJECT;
		return;
	}

	if (0 == m_glInternalFormat)
	{
		*pErrCode = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		return;
	}
	// Setup internal parameters
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE3D;

	m_szImageWidth = realWidth;
	m_szImageHeight = realHeight;
	m_szImageDepth = realDepth;

	// set image format (copy data);
	m_pclImageFormat = new cl_image_format();
	*m_pclImageFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_pclImageFormat->image_channel_order)
	{
		*pErrCode = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		return;
	}

	m_szImageRowPitch = m_szImageWidth * GetPixelBytesCount(m_pclImageFormat);
	m_szImageSlicePitch = m_szImageRowPitch*m_szImageHeight;

	// create buffer for image data
	m_szMemObjSize = CalcImageSize();

	*pErrCode = CL_SUCCESS;
}

void GLTexture3D::GetGLObjectData()
{
	GLenum readBackFormat = GetGLFormat(m_pclImageFormat->image_channel_data_type);
	GLenum readBackType = GetGLType(m_pclImageFormat->image_channel_data_type); 

	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(m_glTextureTarget);
	GLint glErr = 0;
	glGetIntegerv(targetBinding, &currTexture);

	GLenum glBaseTarget = GetBaseTarget(m_glTextureTarget);
	glBindTexture(glBaseTarget, m_glBufObj);

	glGetTexImage( m_glTextureTarget, m_glMipLevel, readBackFormat, readBackType, m_pMemObjData );

	glBindTexture(glBaseTarget, currTexture);
}

void GLTexture3D::SetGLObjectData()
{
	if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		return;
	}

	GLenum readBackFormat = GetGLFormat(m_pclImageFormat->image_channel_data_type);
	GLenum readBackType = GetGLType(m_pclImageFormat->image_channel_data_type); 

	GLint	currTexture;
	GLenum	targetBinding = GetTargetBinding(m_glTextureTarget);
	GLint glErr = 0;
	glGetIntegerv(targetBinding, &currTexture);

	GLenum glBaseTarget = GetBaseTarget(m_glTextureTarget);
	glBindTexture(glBaseTarget, m_glBufObj);

	if( glBaseTarget == GL_TEXTURE_CUBE_MAP )
	{
		((GLContext*)m_pContext)->glTexImage3D( m_glTextureTarget, m_glMipLevel, m_glInternalFormat,
			(GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, (GLsizei)m_szImageDepth, m_glBorder, readBackFormat, readBackType, m_pMemObjData );
	}
	else
	{
		((GLContext*)m_pContext)->glTexImage3D( m_glTextureTarget, m_glMipLevel, m_glInternalFormat,
			(GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, (GLsizei)m_szImageDepth, m_glBorder, readBackFormat, readBackType, m_pMemObjData );        
	}

	glBindTexture(glBaseTarget, currTexture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLRenderBuffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////

GLRenderBuffer::GLRenderBuffer(GLContext * pContext, cl_mem_flags clMemFlags, GLuint glBufObj,
							   ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode):
	Image2D(pContext, clMemFlags, NULL, NULL, 0, 0, 0, pOclEntryPoints, pErrCode),
	GLMemoryObject(glBufObj), m_glFramebuffer(0)
{
	GLint	currBuffer;
	GLint glErr = 0;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &currBuffer);
	glErr |= glGetError();

	((GLContext*)m_pContext)->glBindRenderbufferEXT(GL_RENDERBUFFER, m_glBufObj);

	GLint realWidth, realHeight;
	((GLContext*)m_pContext)->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &realWidth);
	glErr |= glGetError();
	((GLContext*)m_pContext)->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &realHeight);
	glErr |= glGetError();
	((GLContext*)m_pContext)->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT_EXT, &m_glInternalFormat);
	glErr |= glGetError();

	((GLContext*)m_pContext)->glBindRenderbufferEXT(GL_RENDERBUFFER, currBuffer);
	glErr |= glGetError();
	if ( 0 != glErr )
	{
		*pErrCode = CL_INVALID_GL_OBJECT;
		return;
	}

	// Now we need to create a frame buffer
	((GLContext*)m_pContext)->glGenFramebuffersEXT( 1, &m_glFramebuffer );
	if ( 0 != glGetError() )
	{
		*pErrCode = CL_INVALID_GL_OBJECT;
		return;
	}

	if (0 == m_glInternalFormat)
	{
		*pErrCode = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		return;
	}
	// Setup internal parameters
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D;

	m_szImageWidth = realWidth;
	m_szImageHeight = realHeight;

	// set image format (copy data);
	m_pclImageFormat = new cl_image_format();
	*m_pclImageFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_pclImageFormat->image_channel_order)
	{
		*pErrCode = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		return;
	}

	m_szImageRowPitch = m_szImageWidth * GetPixelBytesCount(m_pclImageFormat);

	// create buffer for image data
	m_szMemObjSize = CalcImageSize();

	*pErrCode = CL_SUCCESS;
}

GLRenderBuffer::~GLRenderBuffer()
{
	if ( 0 != m_glFramebuffer )
	{
		((GLContext*)m_pContext)->glDeleteFramebuffersEXT( 1, &m_glFramebuffer );
	}
}
void GLRenderBuffer::GetGLObjectData()
{
	GLint	currBuffer;
	GLint glErr = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currBuffer);
	glErr |= glGetError();

	// Create and bind a frame buffer to render with
	((GLContext*)m_pContext)->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
	if( glGetError() != GL_NO_ERROR )
		return;

	// Attach to the framebuffer
	((GLContext*)m_pContext)->glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT,
		GL_RENDERBUFFER, m_glBufObj );
	if( glGetError() != GL_NO_ERROR )
		return;
	if( ((GLContext*)m_pContext)->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		return;
	}

	GLenum readBackFormat = GetGLFormat(m_pclImageFormat->image_channel_data_type);
	GLenum readBackType = GetGLType(m_pclImageFormat->image_channel_data_type); 

	glReadPixels( 0, 0, (GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, readBackFormat, readBackType, m_pMemObjData );

	((GLContext*)m_pContext)->glBindFramebufferEXT( GL_FRAMEBUFFER, currBuffer );

}
void GLRenderBuffer::SetGLObjectData()
{
	GLint	currBuffer;
	GLint glErr = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currBuffer);
	glErr |= glGetError();

	// Create and bind a frame buffer to render with
	((GLContext*)m_pContext)->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
	if( glGetError() != GL_NO_ERROR )
		return;

	// Attach to the framebuffer
	((GLContext*)m_pContext)->glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT,
		GL_RENDERBUFFER, m_glBufObj );
	if( glGetError() != GL_NO_ERROR )
		return;
	if( ((GLContext*)m_pContext)->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		return;
	}

	GLenum glFormat = GetGLFormat(m_pclImageFormat->image_channel_data_type);
	GLenum glType = GetGLType(m_pclImageFormat->image_channel_data_type); 

#if 1
    // Fill a texture with our input data
    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, texture );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0/*Level*/, m_glInternalFormat,
		(GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, 0/*Border*/,
		glFormat, glType, m_pMemObjData );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    // Render fullscreen textured quad 
    glDisable( GL_LIGHTING );
    glViewport(0, 0, (GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    //gluOrtho2D( -1.0, 1.0, -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glBegin( GL_QUADS );
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, 0.0f );
        glTexCoord2f( 0.0f, (float)m_szImageHeight );
        glVertex3f( -1.0f, 1.0f, 0.0f );
        glTexCoord2f( (float)m_szImageWidth, (float)m_szImageHeight );
        glVertex3f( 1.0f, 1.0f, 0.0f );
        glTexCoord2f( (float)m_szImageWidth, 0.0f );
        glVertex3f( 1.0f, -1.0f, 0.0f );
    }
    glEnd();
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0 );
    glDisable( GL_TEXTURE_RECTANGLE_ARB );

	glFlush();
	glDeleteTextures(1, &texture);
    
#else
	glDrawPixels((GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, glFormat, glType, m_pMemObjData );
	glErr = glGetError();
#endif
	((GLContext*)m_pContext)->glBindFramebufferEXT( GL_FRAMEBUFFER, currBuffer );
}
