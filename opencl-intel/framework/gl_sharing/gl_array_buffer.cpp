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

#include "gl_array_buffer.h"
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLArrayBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_BUFFER, GLArrayBuffer)

GLArrayBuffer::GLArrayBuffer(SharedPtr<Context> pContext, cl_gl_object_type clglObjType) :
    GLMemoryObject(pContext, clglObjType)
{
    m_uiNumDim = 1;
    m_glBufferTarget = CL_GL_OBJECT_BUFFER == clglObjType ? GL_ARRAY_BUFFER : GL_TEXTURE_BUFFER;
    m_glBufferTargetBinding = GetTargetBinding(m_glBufferTarget);
    m_clMemObjectType = CL_MEM_OBJECT_BUFFER;
}

cl_err_code GLArrayBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
                       const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags  creation_flags )
{
    SharedPtr<GLContext> pGLContext = m_pContext.DynamicCast<GLContext>();
    m_glObjHandle = (GLuint)pHostPtr;

    // Retrieve open GL buffer size
    GLint   currBuff;
    GLint   buffSize;
    GLint   glErr = 0;

    // Retrieve open GL buffer size
    glGetIntegerv(m_glBufferTargetBinding, &currBuff);
    glErr |= glGetError();

    m_pGLContext->glBindBuffer(m_glBufferTarget, m_glObjHandle);
    glErr |= glGetError();
    m_pGLContext->glGetBufferParameteriv(m_glBufferTarget, GL_BUFFER_SIZE, &buffSize);
    glErr |= glGetError();
    m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);
    glErr |= glGetError();
    if ( 0 != glErr )
    {
        return CL_INVALID_GL_OBJECT;
    }

    m_stMemObjSize = buffSize;

    m_clFlags = clMemFlags;

    SetGLMemFlags();

    return CL_SUCCESS;
}

cl_err_code GLArrayBuffer::GetDimensionSizes(size_t* pszRegion) const
{
    assert(pszRegion);
    if (nullptr == pszRegion)
    {
        return CL_INVALID_VALUE;
    }
    pszRegion[0] = m_stMemObjSize;
    return CL_SUCCESS;
}

void GLArrayBuffer::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
{
    if (nullptr != dimensions)
    {
        GetDimensionSizes(dimensions);
    }
}

cl_err_code GLArrayBuffer::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
            const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
    Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
    if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
    {
        return CL_INVALID_GL_OBJECT;
    }

    return m_itCurrentAcquriedObject->second->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
}

cl_err_code GLArrayBuffer::AcquireGLObject()
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

    GLint currBuff;
    glGetIntegerv(m_glBufferTargetBinding, &currBuff);
    assert(!glGetError());
    m_pGLContext->glBindBuffer(m_glBufferTarget, m_glObjHandle);
    assert(!glGetError());
    void *pBuffer = m_pGLContext->glMapBuffer(m_glBufferTarget, m_glMemFlags);
    if ( nullptr == pBuffer )
    {
        m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_OUT_OF_RESOURCES;
    }

    // Now we need to create child object
    SharedPtr<MemoryObject> pChild;
    cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, m_clMemObjectType, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
    if (CL_FAILED(res))
    {
        m_pGLContext->glUnmapBuffer(m_glBufferTarget);
        m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return res;
    }

    const cl_image_format* pCLFormat = nullptr;
    if ( CL_GL_OBJECT_TEXTURE_BUFFER == m_clglObjectType )
    {
        pCLFormat = &m_clImageFormat;
    }

    res = pChild->Initialize(m_clFlags, pCLFormat, 1, &m_stMemObjSize, nullptr, pBuffer, CL_RT_MEMOBJ_FORCE_BS);
    if (CL_FAILED(res))
    {
        m_pGLContext->glUnmapBuffer(m_glBufferTarget);
        m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return CL_OUT_OF_RESOURCES;
    }

    m_itCurrentAcquriedObject->second = pChild;

    m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLArrayBuffer::ReleaseGLObject()
{
    GLint   currBuff;
    glGetIntegerv(m_glBufferTargetBinding, &currBuff);
    assert(!glGetError());
    m_pGLContext->glBindBuffer(m_glBufferTarget, m_glObjHandle);
    assert(!glGetError());
    const GLboolean ret = m_pGLContext->glUnmapBuffer(m_glBufferTarget);
    assert(GL_TRUE == ret);
    m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);
    assert(!glGetError());

    // block until all GL operations are completed
    glFinish();

    return CL_SUCCESS;
}

cl_err_code GLArrayBuffer::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (*pszOrigin + *pszRegion <= m_stMemObjSize)
    {
        return CL_SUCCESS;
    }
    return CL_INVALID_VALUE;
}
