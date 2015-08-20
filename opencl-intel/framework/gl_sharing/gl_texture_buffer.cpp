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
// GLTextureBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE_BUFFER, 0, GLTextureBuffer)
GLTextureBuffer::GLTextureBuffer(SharedPtr<Context> pContext,cl_mem_object_type clObjType) :
    GLMemoryObject(pContext, clObjType)
{
    m_uiNumDim = 1;
    m_glTextureTarget = GL_TEXTURE_BUFFER;
    m_glTextureTargetBinding = GetTargetBinding(m_glTextureTarget);
    m_clMemObjectType = CL_MEM_OBJECT_IMAGE1D_BUFFER;

    // texture buffers are not mipmapped
    m_glMipLevel = 0;
}

cl_err_code GLTextureBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
            const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags creation_flags )
{
    GLint currTexture = 0;
    GLint currArrayBuffer = 0;
    GLint buffSize = 0;
    GLint glErr = GL_NO_ERROR;

    GLTextureDescriptor* pTxtDescriptor = (GLTextureDescriptor*)pHostPtr;

    if (pTxtDescriptor->glTextureTarget != m_glTextureTarget ||
        pTxtDescriptor->glMipLevel != m_glMipLevel)
    {
        return CL_INVALID_GL_OBJECT;
    }

    m_glObjHandle = (GLuint)pTxtDescriptor->glTexture;

    glGetIntegerv(m_glTextureTargetBinding, &currTexture);
    glErr |= glGetError();

    glBindTexture(m_glTextureTarget, m_glObjHandle);
    glErr |= glGetError();

    // retrieve OpenGL internal format
    glGetTexLevelParameteriv(m_glTextureTarget, m_glMipLevel, GL_TEXTURE_INTERNAL_FORMAT, &m_glInternalFormat);
    glErr |= glGetError();

    // retrieve the attached buffer for this texture
    m_glAttachedBuffer = 0;
    glGetTexLevelParameteriv(m_glTextureTarget, m_glMipLevel, GL_TEXTURE_BUFFER_DATA_STORE_BINDING, &m_glAttachedBuffer);
    glErr |= glGetError();

    // bind the attached buffer to get its size
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currArrayBuffer);
    glErr |= glGetError();

    m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, m_glAttachedBuffer);
    glErr |= glGetError();

    m_pGLContext->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffSize);
    glErr |= glGetError();

    m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, currArrayBuffer);
    glErr |= glGetError();

    glBindTexture(m_glTextureTarget, currTexture);
    glErr |= glGetError();

    if (GL_NO_ERROR != glErr || 0 == m_glAttachedBuffer)
    {
        return CL_INVALID_GL_OBJECT;
    }
    m_clFormat = ImageFrmtConvertGL2CL(m_glInternalFormat);
    if ( 0 == m_clFormat.clType.image_channel_order)
    {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }

    m_stMemObjSize = buffSize;

    m_clFlags = clMemFlags;
    SetGLMemFlags();

    m_clImageFormat = m_clFormat.clType;
    return CL_SUCCESS;
}

cl_err_code GLTextureBuffer::AcquireGLObject()
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

    GLint glErr = GL_NO_ERROR;
    GLint curArrayBuf = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &curArrayBuf);
    glErr |= glGetError();

    m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, m_glAttachedBuffer);
    glErr |= glGetError();

    void* pData = m_pGLContext->glMapBuffer(GL_ARRAY_BUFFER, m_glMemFlags);
    glErr |= glGetError();

    if (GL_NO_ERROR != glErr || NULL == pData)
    {
        m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, curArrayBuf);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_OUT_OF_RESOURCES;
    }

    // Now we need to create child object
    SharedPtr<MemoryObject> pChild;
    cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, m_clMemObjectType, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
    if (CL_FAILED(res))
    {
        m_pGLContext->glUnmapBuffer(GL_ARRAY_BUFFER);
        m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, curArrayBuf);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return res;
    }

    const cl_image_format* pCLFormat = NULL;
    if ( CL_GL_OBJECT_TEXTURE_BUFFER == m_clglObjectType )
    {
        pCLFormat = &m_clImageFormat;
    }

    res = pChild->Initialize(m_clFlags, pCLFormat, 1, &m_stMemObjSize, NULL, pData, CL_RT_MEMOBJ_FORCE_BS);
    if (CL_FAILED(res))
    {
        m_pGLContext->glUnmapBuffer(GL_ARRAY_BUFFER);
        m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, curArrayBuf);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_OUT_OF_RESOURCES;
    }

    m_itCurrentAcquriedObject->second = pChild;

    m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, curArrayBuf);

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLTextureBuffer::ReleaseGLObject()
{
    GLint curArrayBuf = 0;
    GLint glErr = GL_NO_ERROR;

    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &curArrayBuf);
    glErr |= glGetError();

    m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, m_glAttachedBuffer);
    glErr |= glGetError();

    const GLboolean ret = m_pGLContext->glUnmapBuffer(GL_ARRAY_BUFFER);

    m_pGLContext->glBindBuffer(GL_ARRAY_BUFFER, curArrayBuf);
    glErr |= glGetError();

    if (GL_NO_ERROR != glErr || GL_TRUE != ret)
    {
        return CL_OUT_OF_RESOURCES;
    }

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLTextureBuffer::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
            const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
    Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
    if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
    {
        return CL_INVALID_GL_OBJECT;
    }

    return m_itCurrentAcquriedObject->second->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
}

cl_err_code GLTextureBuffer::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (*pszOrigin + *pszRegion <= m_stMemObjSize)
    {
        return CL_SUCCESS;
    }
    return CL_INVALID_VALUE;
}

cl_err_code GLTextureBuffer::GetDimensionSizes(size_t* pszRegion) const
{
    assert(pszRegion);
    if (NULL == pszRegion)
    {
        return CL_INVALID_VALUE;
    }
    pszRegion[0] = m_stMemObjSize;
    return CL_SUCCESS;
}

void GLTextureBuffer::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
{
    if (NULL != dimensions)
    {
        GetDimensionSizes(dimensions);
    }
}

cl_err_code GLTextureBuffer::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
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

cl_err_code GLTextureBuffer::GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize)
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
