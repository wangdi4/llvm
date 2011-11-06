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

cl_err_code GLBuffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
					   const size_t* dimension, const size_t* pitches, void* pHostPtr)
{
	GLContext* pGLContext = static_cast<GLContext*>(m_pContext);

	m_glObjHandle = (GLuint)pHostPtr;
	// Retrieve open GL buffer size
	GLint	currBuff;
	GLint	buffSize;
	GLint	glErr = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuff);
	glErr |= glGetError();
	pGLContext->glBindBuffer(GL_ARRAY_BUFFER, m_glObjHandle);
	glErr |= glGetError();
	pGLContext->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffSize);
	glErr |= glGetError();
	pGLContext->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
	glErr |= glGetError();
	if ( 0 != glErr )
	{
		return CL_INVALID_GL_OBJECT;
	}

	m_clMemObjectType = CL_MEM_OBJECT_BUFFER;

	m_stMemObjSize = buffSize;

	m_clFlags = clMemFlags;

    m_uiNumDim = 1;
	SetGLMemFlags();

	return CL_SUCCESS;
}

cl_err_code GLBuffer::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer)
{
	if (NULL == m_pChildObject)
	{
		return CL_INVALID_GL_OBJECT;
	}

	return m_pChildObject->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
}

cl_err_code GLBuffer::AcquireGLObject()
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);

	if (NULL != m_pChildObject && CL_SUCCEEDED(GetAcquireState()))
	{
		// We have already acquired object
		return CL_SUCCESS;
	}

	m_muAcquireRelease.Lock();

	GLint	currBuff;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuff);
	((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, m_glObjHandle);
	void *pBuffer = ((GLContext*)m_pContext)->glMapBuffer(GL_ARRAY_BUFFER, m_glMemFlags);
	if ( NULL == pBuffer )
	{
		((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
		SetAcquireState(CL_INVALID_OPERATION);
		return CL_INVALID_OPERATION;
	}

	// Now we need to create child object
	MemoryObject* pChild;
	cl_err_code res = MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU, CL_MEM_OBJECT_BUFFER, CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
	if (CL_FAILED(res))
	{
		((GLContext*)m_pContext)->glUnmapBuffer(GL_ARRAY_BUFFER);
		((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
		SetAcquireState(res);
		return res;
	}

	res = pChild->Initialize(m_clFlags, NULL, 1, &m_stMemObjSize, NULL, pBuffer);
	if (CL_FAILED(res))
	{
		((GLContext*)m_pContext)->glUnmapBuffer(GL_ARRAY_BUFFER);
		((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);
		SetAcquireState(CL_OUT_OF_RESOURCES);
		return CL_OUT_OF_RESOURCES;
	}

	m_pChildObject.exchange(pChild);

	((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);

	return CL_SUCCESS;
}

cl_err_code GLBuffer::ReleaseGLObject()
{
	MemoryObject* pChild = m_pChildObject.exchange(NULL);

	if ( NULL == pChild )
	{
		return CL_INVALID_OPERATION;
	}

	pChild->Release();

	GLint	currBuff;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuff);
	((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, m_glObjHandle);

	((GLContext*)m_pContext)->glUnmapBuffer(GL_ARRAY_BUFFER);
	((GLContext*)m_pContext)->glBindBuffer(GL_ARRAY_BUFFER, currBuff);

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
