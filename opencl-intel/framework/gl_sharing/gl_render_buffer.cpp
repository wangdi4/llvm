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

#include "gl_render_buffer.h"
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLRenderBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_RENDERBUFFER, GLRenderBuffer)

GLRenderBuffer::GLRenderBuffer(SharedPtr<Context> pContext, cl_gl_object_type clglObjType):
	GLTexture2D(pContext, clglObjType)
{
	m_uiNumDim = 2;

	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D;
}

cl_err_code GLRenderBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	GLint	currBuffer;
	GLint glErr = 0;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &currBuffer);
	glErr |= glGetError();

	m_glObjHandle = (GLuint)pHostPtr;

	m_pGLContext->glBindRenderbufferEXT(GL_RENDERBUFFER, m_glObjHandle);

	GLint realWidth, realHeight;
	m_pGLContext->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &realWidth);
	glErr |= glGetError();
	m_pGLContext->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &realHeight);
	glErr |= glGetError();
	m_pGLContext->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT_EXT, &m_glInternalFormat);
	glErr |= glGetError();
    if (0 == realWidth || 0 == realHeight)
    {
        return CL_INVALID_GL_OBJECT;
    }

	m_pGLContext->glBindRenderbufferEXT(GL_RENDERBUFFER, currBuffer);
	glErr |= glGetError();
	if ( 0 != glErr )
	{
		return CL_INVALID_GL_OBJECT;
	}

	// Now we need to create PBO
	m_pGLContext->glGenBuffers(1, &m_glPBO);
	if ( 0 != glGetError() )
	{
		return CL_OUT_OF_RESOURCES;
	}

	m_clFlags = clMemFlags;
	SetGLMemFlags();

	// Now we need to create a frame buffer
	m_pGLContext->glGenFramebuffersEXT( 1, &m_glFramebuffer );
	if ( 0 != glGetError() )
	{
		return CL_INVALID_GL_OBJECT;
	}

	if (0 == m_glInternalFormat)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	// Setup internal parameters
	m_stDimensions[0] = realWidth;
	m_stDimensions[1] = realHeight;

	m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_clFormat.clType.image_channel_order)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	m_glReadBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
	m_glReadBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

	m_stElementSize = clGetPixelBytesCount(&m_clFormat.clType);
	m_stPitches[0] = m_stDimensions[0] * m_stElementSize;

	// create buffer for image data
	m_stMemObjSize = m_stPitches[0] * m_stDimensions[1];

	return CL_SUCCESS;
}

GLRenderBuffer::~GLRenderBuffer()
{
}

cl_err_code GLRenderBuffer::AcquireGLObject()
{
    Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);

    if ( m_lstAcquiredObjectDescriptors.end() != m_itCurrentAcquriedObject && 
        ( (CL_GFX_OBJECT_NOT_ACQUIRED != m_itCurrentAcquriedObject->second) &&
        (CL_GFX_OBJECT_NOT_READY != m_itCurrentAcquriedObject->second) &&
        (CL_GFX_OBJECT_FAIL_IN_ACQUIRE != m_itCurrentAcquriedObject->second) )
        )
    {
        // We have already acquired object
        return CL_SUCCESS;
    }

    m_muAcquireRelease.Lock();

    // First of all bind Render buffer to FBO
    GLint currentFBO;
    GLint glErr = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    glErr |= glGetError();

    // Create and bind a frame buffer to render with
    m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
    if( glGetError() != GL_NO_ERROR )
    {
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    // Attach to the framebuffer
    m_pGLContext->glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER, m_glObjHandle );
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currentFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }
    if( m_pGLContext->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currentFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    GLint currentPBO;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &currentPBO);

    // read pixels from framebuffer to PBO
    m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }
    GLenum glUsage = m_clFlags & CL_MEM_READ_ONLY ? GL_STREAM_READ_ARB : GL_STREAM_COPY_ARB;
    m_pGLContext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, m_stMemObjSize, nullptr, glUsage );
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    // glReadPixels() should return immediately, the transfer is in background by DMA
    glReadPixels(0, 0, (GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], m_glReadBackFormat, m_glReadBackType, 0);
    if( glGetError() != GL_NO_ERROR )
    {
        m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
        m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_INVALID_OPERATION;
    }

    m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
    m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );

    m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_NOT_READY;

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLRenderBuffer::ReleaseGLObject()
{
	if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);

		// bind PBO to update texture source
		m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);

		m_pGLContext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
		m_pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
	} else if ( m_clFlags & CL_MEM_WRITE_ONLY )
	{
		// Update texture context
		GLenum readBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
		GLenum readBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

		GLenum pboBinding = m_clFlags & CL_MEM_WRITE_ONLY ? GL_PIXEL_UNPACK_BUFFER_BINDING_ARB : GL_PIXEL_PACK_BUFFER_BINDING_ARB;
		GLenum pboTarget = m_clFlags & CL_MEM_WRITE_ONLY ? GL_PIXEL_UNPACK_BUFFER_ARB : GL_PIXEL_PACK_BUFFER_ARB;
		// Unmap PBO
		GLint currentPBO;
		glGetIntegerv(pboBinding, &currentPBO);

		// bind PBO to update texture source
		m_pGLContext->glBindBuffer(pboTarget, m_glPBO);
		m_pGLContext->glUnmapBuffer(pboTarget);

		// Create and fill a texture with updated data
		GLint currTex;
		glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &currTex);

		GLuint texture;
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, texture );
		glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0/*Level*/, m_glInternalFormat, (GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], 0/*Border*/, readBackFormat, readBackType, nullptr);

		m_pGLContext->glBindBuffer(pboTarget, currentPBO);

		// Need bing again the FBO
		GLint	currBuffer;
		GLint glErr = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currBuffer);
		glErr |= glGetError();

		glEnable( GL_TEXTURE_RECTANGLE_ARB );

		// Create and bind a frame buffer to render with
		m_pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
		if( glGetError() != GL_NO_ERROR )
		{
			return CL_INVALID_OPERATION;
		}

		// Now need render back to FBO
		// Render fullscreen textured quad 
		glDisable( GL_LIGHTING );
		glViewport(0, 0, (GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1]);
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
			glTexCoord2f( 0.0f, (float)m_stDimensions[1] );
			glVertex3f( -1.0f, 1.0f, 0.0f );
			glTexCoord2f( (float)m_stDimensions[0], (float)m_stDimensions[1] );
			glVertex3f( 1.0f, 1.0f, 0.0f );
			glTexCoord2f( (float)m_stDimensions[0], 0.0f );
			glVertex3f( 1.0f, -1.0f, 0.0f );
		}
		glEnd();
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0 );
		glDisable( GL_TEXTURE_RECTANGLE_ARB );

		glFlush();
		glDeleteTextures(1, &texture);
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, currTex );
	}

    // block until all GL operations are completed
    glFinish();

	return CL_SUCCESS;
}
