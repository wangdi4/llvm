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


#include "gl_texture3D.h"

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

cl_err_code GLTexture3D::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
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
	GLint realWidth, realHeight, realDepth;
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_DEPTH, &realDepth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_BORDER, &m_glBorder);

	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat );
	glErr |= glGetError();

	glBindTexture(glBaseTarget, currTexture);
	glErr |= glGetError();

	if ( 0 != glErr )
	{
		return CL_INVALID_GL_OBJECT;
	}

	if (0 == m_glInternalFormat)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}
	// Setup internal parameters
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE3D;

	m_szImageWidth = realWidth;
	m_szImageHeight = realHeight;
	m_szImageDepth = realDepth;

	m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_clFormat.clType.image_channel_order)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	m_uiNumDim = 3;
	m_szElementSize = m_pContext->GetPixelBytesCount(&m_clFormat.clType);
	m_szImageRowPitch = m_szImageWidth * m_szElementSize;
	m_szImageSlicePitch = m_szImageRowPitch*m_szImageHeight;

	// create buffer for image data
	m_stMemObjSize = m_szImageSlicePitch*m_szImageDepth;

	m_txtDescriptor = *pTxtDescriptor;
	m_glObjHandle = m_txtDescriptor.glTexture;
	m_clFlags = clMemFlags;
	SetGLMemFlags();

	return CL_SUCCESS;
}

cl_err_code GLTexture3D::AcquireGLObject()
{
	// Since there is no efficien mechanism to access 3D texture we need to allacte real memory object
	// Now we need to create child object
	MemoryObject* pChild;
	cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, CL_MEM_OBJECT_IMAGE3D, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
	if (CL_FAILED(res))
	{
		SetAcquireState(res);
		return res;
	}

	size_t dim[] = {m_szImageWidth, m_szImageHeight, m_szImageDepth};
//	size_t pitch[] = {m_szImageRowPitch, m_szImageSlicePitch};
	res = pChild->Initialize(m_clFlags, &m_clFormat.clType, 3, dim, NULL, NULL);
	if (CL_FAILED(res))
	{
		pChild->Release();
		SetAcquireState(CL_OUT_OF_RESOURCES);
		return CL_OUT_OF_RESOURCES;
	}

	m_pMemObjData = pChild->GetBackingStoreData(NULL);

	// Now read image data if requried
	if ( (m_clFlags & CL_MEM_READ_WRITE) || (m_clFlags & CL_MEM_READ_ONLY) )
	{
		GLenum readBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
		GLenum readBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

		GLint	currTexture;
		GLenum	targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);
		GLint glErr = 0;
		glGetIntegerv(targetBinding, &currTexture);

		GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
		glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

		glGetTexImage( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, readBackFormat, readBackType, m_pMemObjData );

		glBindTexture(glBaseTarget, currTexture);
	}

	SetAcquireState(CL_SUCCESS);
	m_pChildObject.exchange(pChild);

	return CL_SUCCESS;

}

cl_err_code GLTexture3D::ReleaseGLObject()
{
	MemoryObject* pChild = m_pChildObject.exchange(NULL);

	if ( NULL == pChild )
	{
		return CL_INVALID_OPERATION;
	}

	// Now write back image data if requried
	if ( (m_clFlags & CL_MEM_READ_WRITE) || (m_clFlags & CL_MEM_WRITE_ONLY) )
	{
		GLenum readBackFormat = GetGLFormat(m_clFormat.clType.image_channel_data_type, m_clFormat.isGLExt);
		GLenum readBackType = GetGLType(m_clFormat.clType.image_channel_data_type);

		GLint	currTexture;
		GLenum	targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);
		GLint glErr = 0;
		glGetIntegerv(targetBinding, &currTexture);

		GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
		glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

		((GLContext*)m_pContext)->glTexImage3D( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, m_glInternalFormat,
			(GLsizei)m_szImageWidth, (GLsizei)m_szImageHeight, (GLsizei)m_szImageDepth, m_glBorder, readBackFormat, readBackType, m_pMemObjData );

		glBindTexture(glBaseTarget, currTexture);
	}

	m_pMemObjData = NULL;

	pChild->Release();

	return CL_SUCCESS;

}

cl_err_code GLTexture3D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > m_szImageWidth ||
        pszOrigin[1] + pszRegion[1] > m_szImageHeight ||
        pszOrigin[2] + pszRegion[2] > m_szImageDepth)
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}
