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


#include "gl_texture_buffer.h"

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
GLTextureBuffer::GLTextureBuffer(SharedPtr<Context> pContext,cl_mem_object_type clObjType) :
	GLBuffer(pContext, clObjType)
{
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE1D_BUFFER;
}

cl_err_code GLTextureBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	cl_err_code err;

	GLTextureDescriptor* pTxtDescriptor = (GLTextureDescriptor*)pHostPtr;

	err = GLBuffer::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, (void*)pTxtDescriptor->glTexture, creation_flags);
	if ( CL_FAILED(err) )
	{
		return err;
	}

	// Retrieve texture_buffer format
	GLint	currTexture;
	GLint glErr = 0;
	glGetIntegerv(m_glBufferTargetBinding, &currTexture);
	glErr |= glGetError();

	glBindTexture(m_glBufferTarget, pTxtDescriptor->glTexture);
	glErr |= glGetError();
	
	glGetTexLevelParameteriv( pTxtDescriptor->glTextureTarget, pTxtDescriptor->glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat );
	glErr |= glGetError();

	glBindTexture(m_glBufferTarget, currTexture);
    glErr |= glGetError();

	if ( 0 != glErr )
	{
		return CL_INVALID_GL_OBJECT;
	}
	m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
	if ( 0 == m_clFormat.clType.image_channel_order)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	m_clImageFormat = m_clFormat.clType;
	m_glMipLevel = pTxtDescriptor->glMipLevel;
	return CL_SUCCESS;
}

cl_err_code	GLTextureBuffer::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
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

cl_err_code GLTextureBuffer::GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize)
{
	void* pIntVal;
	size_t intSize;

	switch (glTextInfo)
	{
	case CL_GL_TEXTURE_TARGET:
		pIntVal = &m_glBufferTarget;
		intSize = sizeof(m_glBufferTarget);
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
