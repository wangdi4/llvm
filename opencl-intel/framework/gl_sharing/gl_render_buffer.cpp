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

cl_err_code GLRenderBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr)
{
	GLint	currBuffer;
	GLint glErr = 0;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &currBuffer);
	glErr |= glGetError();

	m_glObjHandle = (GLuint)pHostPtr;

	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	pGLContext->glBindRenderbufferEXT(GL_RENDERBUFFER, m_glObjHandle);

	GLint realWidth, realHeight;
	pGLContext->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &realWidth);
	glErr |= glGetError();
	pGLContext->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &realHeight);
	glErr |= glGetError();
	pGLContext->glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT_EXT, &m_glInternalFormat);
	glErr |= glGetError();

	pGLContext->glBindRenderbufferEXT(GL_RENDERBUFFER, currBuffer);
	glErr |= glGetError();
	if ( 0 != glErr )
	{
		return CL_INVALID_GL_OBJECT;
	}

	// Now we need to create PBO
	pGLContext->glGenBuffers(1, &m_glPBO);
	if ( 0 != glGetError() )
	{
		return CL_OUT_OF_RESOURCES;
	}

	m_clFlags = clMemFlags;
	SetGLMemFlags();

	// Now we need to create a frame buffer
	pGLContext->glGenFramebuffersEXT( 1, &m_glFramebuffer );
	if ( 0 != glGetError() )
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

	m_uiNumDim = 2;

	m_szElementSize = m_pContext->GetPixelBytesCount(&m_clFormat.clType);
	m_szImageRowPitch = m_szImageWidth * m_szElementSize;

	// create buffer for image data
	m_stMemObjSize = m_szImageRowPitch * m_szImageHeight;

	return CL_SUCCESS;
}

GLRenderBuffer::~GLRenderBuffer()
{
}

cl_err_code GLRenderBuffer::AcquireGLObject()
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);

	if (NULL != m_pChildObject && CL_SUCCEEDED(GetAcquireState()))
	{
		// We have already acquired object
		return CL_SUCCESS;
	}

	m_muAcquireRelease.Lock();

	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	// First of all bing Render buffer to FBO
	GLint	currentFBO;
	GLint glErr = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
	glErr |= glGetError();

	// Create and bind a frame buffer to render with
	pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
	if( glGetError() != GL_NO_ERROR )
	{
		SetAcquireState(CL_INVALID_OPERATION);
		return CL_INVALID_OPERATION;
	}

	// Attach to the framebuffer
	pGLContext->glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER, m_glObjHandle );
	if( glGetError() != GL_NO_ERROR )
	{
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currentFBO );
		SetAcquireState(CL_INVALID_OPERATION);
		return CL_INVALID_OPERATION;
	}
	if( pGLContext->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, currentFBO );
		SetAcquireState(CL_INVALID_OPERATION);
		return CL_INVALID_OPERATION;
	}

	// For read operations read data from the FBO
	if ( (m_clFlags & CL_MEM_READ_ONLY) || (m_clFlags & CL_MEM_READ_WRITE) )
	{
		GLint currentPBO;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &currentPBO);

		// read pixels from framebuffer to PBO
		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
			pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		GLenum glUsage = m_clFlags & CL_MEM_READ_ONLY ? GL_STREAM_READ_ARB : GL_STREAM_COPY_ARB;
		pGLContext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, m_stMemObjSize, NULL, glUsage );
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
			pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}
		GLenum readBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
		GLenum readBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

		// glReadPixels() should return immediately, the transfer is in background by DMA
		glReadPixels(0, 0, (GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, readBackFormat, readBackType, 0);
		if( glGetError() != GL_NO_ERROR )
		{
			pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
			pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );
			SetAcquireState(CL_INVALID_OPERATION);
			return CL_INVALID_OPERATION;
		}

		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, currentPBO);
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );

	}
	// when write only just allocate space  in PBO
	else if ( m_clFlags & CL_MEM_WRITE_ONLY )
	{
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currentFBO );

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

cl_err_code GLRenderBuffer::ReleaseGLObject()
{
	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);
	MemoryObject* pChild = m_pChildObject.exchange(NULL);

	if ( NULL == pChild )
	{
		return CL_INVALID_OPERATION;
	}

	pChild->Release();

	if ( m_clFlags & CL_MEM_READ_ONLY )
	{
		GLint pboBinding;
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &pboBinding);

		// bind PBO to update texture source
		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, m_glPBO);

		pGLContext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
		pGLContext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboBinding);
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
		pGLContext->glBindBuffer(pboTarget, m_glPBO);
		pGLContext->glUnmapBuffer(pboTarget);

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
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0/*Level*/, m_glInternalFormat, (GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, 0/*Border*/, readBackFormat, readBackType, NULL);

		pGLContext->glBindBuffer(pboTarget, currentPBO);

		// Need bing again the FBO
		GLint	currBuffer;
		GLint glErr = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currBuffer);
		glErr |= glGetError();

		glEnable( GL_TEXTURE_RECTANGLE_ARB );

		// Create and bind a frame buffer to render with
		pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
		if( glGetError() != GL_NO_ERROR )
		{
			return CL_INVALID_OPERATION;
		}

		// Now need render back to FBO
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
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, currTex );
	}
	return CL_SUCCESS;
}

cl_err_code GLRenderBuffer::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > m_szImageWidth ||
        pszOrigin[1] + pszRegion[1] > m_szImageHeight)
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

#if 0
void GLRenderBuffer::GetGLObjectData()
{

}
void GLRenderBuffer::SetGLObjectData()
{
	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	GLint	currBuffer;
	GLint glErr = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currBuffer);
	glErr |= glGetError();

	// Create and bind a frame buffer to render with
	pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, m_glFramebuffer );
	if( glGetError() != GL_NO_ERROR )
		return;

	// Attach to the framebuffer
	pGLContext->glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT,
		GL_RENDERBUFFER, m_glBufObj );
	if( glGetError() != GL_NO_ERROR )
		return;
	if( pGLContext->glCheckFramebufferStatusEXT( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		return;
	}
	
	fmt_cl_gl* pFmt = (fmt_cl_gl*)m_pclImageFormat;

	GLenum glFormat = GetGLFormat(pFmt->clType.image_channel_data_type, pFmt->isGLExt);
	GLenum glType = GetGLType(pFmt->clType.image_channel_data_type); 

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
	pGLContext->glBindFramebufferEXT( GL_FRAMEBUFFER, currBuffer );
}
#endif