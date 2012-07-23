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


#include "gl_buffer.h"
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// GLBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_BUFFER, GLBuffer)

GLBuffer::GLBuffer(SharedPtr<Context> pContext, cl_gl_object_type clglObjType) :
	GLMemoryObject(pContext, clglObjType)
{
    m_uiNumDim = 1;
	m_glBufferTarget = CL_GL_OBJECT_BUFFER == clglObjType ? GL_ARRAY_BUFFER : GL_TEXTURE_BUFFER;
	m_glBufferTargetBinding = GetTargetBinding(m_glBufferTarget);
	m_clMemObjectType = CL_MEM_OBJECT_BUFFER;
}

cl_err_code GLBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
					   const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags )
{
	SharedPtr<GLContext> pGLContext = m_pContext.DynamicCast<GLContext>();
	m_glObjHandle = (GLuint)pHostPtr;

	// Retrieve open GL buffer size
	GLint	currBuff;
	GLint	buffSize;
	GLint	glErr = 0;

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

cl_err_code GLBuffer::GetDimensionSizes(size_t* pszRegion) const
{
    assert(pszRegion);
    if (NULL == pszRegion)
    {
        return CL_INVALID_VALUE;
    }
    pszRegion[0] = m_stMemObjSize;
    return CL_SUCCESS;
}

void GLBuffer::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
{
    if (NULL != dimensions)
    {
        GetDimensionSizes(dimensions);
    }
}

cl_err_code GLBuffer::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
	if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
	{
		return CL_INVALID_GL_OBJECT;
	}

	return m_itCurrentAcquriedObject->second->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
}

cl_err_code GLBuffer::AcquireGLObject()
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

	GLint	currBuff;
	glGetIntegerv(m_glBufferTargetBinding, &currBuff);
    assert(!glGetError());
    m_pContext.DynamicCast<GLContext>()->glBindBuffer(m_glBufferTarget, m_glObjHandle);
    assert(!glGetError());
	void *pBuffer = m_pGLContext->glMapBuffer(m_glBufferTarget, m_glMemFlags);
	if ( NULL == pBuffer )
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
        m_pContext.DynamicCast<GLContext>()->glUnmapBuffer(m_glBufferTarget);
		m_pContext.DynamicCast<GLContext>()->glBindBuffer(m_glBufferTarget, currBuff);
		m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
		return res;
	}

	const cl_image_format* pCLFormat = NULL;
	if ( CL_GL_OBJECT_TEXTURE_BUFFER == m_clglObjectType )
	{
		pCLFormat = &m_clImageFormat;
	}

	res = pChild->Initialize(m_clFlags, pCLFormat, 1, &m_stMemObjSize, NULL, pBuffer, CL_RT_MEMOBJ_FORCE_BS);
	if (CL_FAILED(res))
	{
		m_pContext.DynamicCast<GLContext>()->glUnmapBuffer(m_glBufferTarget);
		m_pContext.DynamicCast<GLContext>()->glBindBuffer(m_glBufferTarget, currBuff);
		m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
		return CL_OUT_OF_RESOURCES;
	}

    m_itCurrentAcquriedObject->second = pChild;

	m_pGLContext->glBindBuffer(m_glBufferTarget, currBuff);

	return CL_SUCCESS;
}

cl_err_code GLBuffer::ReleaseGLObject()
{
	GLint	currBuff;
	glGetIntegerv(m_glBufferTargetBinding, &currBuff);
    assert(!glGetError());
	m_pContext.DynamicCast<GLContext>()->glBindBuffer(m_glBufferTarget, m_glObjHandle);
    assert(!glGetError());
	const GLboolean ret = m_pContext.DynamicCast<GLContext>()->glUnmapBuffer(m_glBufferTarget);
    assert(GL_TRUE == ret);
	m_pContext.DynamicCast<GLContext>()->glBindBuffer(m_glBufferTarget, currBuff);
    assert(!glGetError());
	return CL_SUCCESS;
}

cl_err_code GLBuffer::CreateChildObject()
{
	assert(0);
	return CL_INVALID_OPERATION;
}

cl_err_code GLBuffer::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (*pszOrigin + *pszRegion <= m_stMemObjSize)
    {
        return CL_SUCCESS;
    }
    return CL_INVALID_VALUE;
}
