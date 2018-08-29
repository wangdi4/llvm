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

GLTexture3D::GLTexture3D(SharedPtr<Context> pContext, cl_gl_object_type clglObjType):
		  GLTexture2D(pContext, clglObjType)
{
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE3D;
	m_uiNumDim = 3;
}

cl_err_code GLTexture3D::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	return  GLTexture::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr, creation_flags);
}

GLint GLTexture3D::CalculateTextureDimensions()
{
	GLint realWidth, realHeight, realDepth, glErr = 0;
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_WIDTH, &realWidth );
	glErr |= glGetError();
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_HEIGHT, &realHeight );
	glErr |= glGetError();
	glGetTexLevelParameteriv( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, GL_TEXTURE_DEPTH, &realDepth );
	glErr |= glGetError();

	if ( 0 == glErr)
	{
		m_stDimensions[0] = realWidth;
		m_stDimensions[1] = realHeight;
		m_stDimensions[2] = realDepth;

		m_stPitches[0] = m_stDimensions[0] * m_stElementSize;
		m_stPitches[1] = m_stPitches[0]*m_stDimensions[1];

		// create buffer for image data
		m_stMemObjSize = m_stPitches[1]*m_stDimensions[2];
	}

	return 	glErr;
}

cl_err_code GLTexture3D::GetDimensionSizes(size_t* pszRegion) const
{
    assert(pszRegion);
    if (nullptr == pszRegion)
    {
        return CL_INVALID_VALUE;
    }
    GLTexture::GetDimensionSizes(pszRegion);
    pszRegion[2] = m_stDimensions[2];
    return CL_SUCCESS;
}

cl_err_code GLTexture3D::AcquireGLObject()
{
    // Since there is no efficien mechanism to access 3D texture we need to allacte real memory object
    // Now we need to create child object
    SharedPtr<MemoryObject> pChild;
    cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, m_clMemObjectType, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
    if (CL_FAILED(res))
    {
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return res;
    }

    res = pChild->Initialize(m_clFlags, &m_clFormat.clType, m_uiNumDim, m_stDimensions, nullptr, nullptr, 0);
    if (CL_FAILED(res))
    {
        pChild->Release();
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_OUT_OF_RESOURCES;
    }

    m_pMemObjData = pChild->GetBackingStoreData(nullptr);

    // Now read image data
    GLint currTexture;
    GLenum targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);
    GLint glErr = 0;
    glGetIntegerv(targetBinding, &currTexture);

    GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
    glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

    glGetTexImage( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, m_glReadBackFormat, m_glReadBackType, m_pMemObjData );

    glBindTexture(glBaseTarget, currTexture);

    m_itCurrentAcquriedObject->second = pChild;

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLTexture3D::ReleaseGLObject()
{
	// Now write back image data if required
	if ( (m_clFlags & CL_MEM_READ_WRITE) || (m_clFlags & CL_MEM_WRITE_ONLY) )
	{
		GLint	currTexture;
		GLenum	targetBinding = GetTargetBinding(m_txtDescriptor.glTextureTarget);
		GLint glErr = 0;
		glGetIntegerv(targetBinding, &currTexture);

		GLenum glBaseTarget = GetBaseTarget(m_txtDescriptor.glTextureTarget);
		glBindTexture(glBaseTarget, m_txtDescriptor.glTexture);

		FillTextureWithData();

		glBindTexture(glBaseTarget, currTexture);
	}

	m_pMemObjData = nullptr;

    // block until all GL operations are completed
    glFinish();

	return CL_SUCCESS;
}

cl_err_code GLTexture3D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > m_stDimensions[0] ||
        pszOrigin[1] + pszRegion[1] > m_stDimensions[1] ||
        pszOrigin[2] + pszRegion[2] > m_stDimensions[2])
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

void GLTexture3D::FillTextureWithData()
{
	m_pGLContext->glTexImage3D( m_txtDescriptor.glTextureTarget, m_txtDescriptor.glMipLevel, m_glInternalFormat,
		(GLsizei)m_stDimensions[0], (GLsizei)m_stDimensions[1], (GLsizei)m_stDimensions[2], m_glBorder, m_glReadBackFormat, m_glReadBackType, m_pMemObjData );
}
