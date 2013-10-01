// Copyright (c) 2006-2013 Intel Corporation
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

/*
*
* File wg_executor.cpp
* Implementation of WGExecutor class, class that manages execution of single Work-Group
*
*/

#include "stdafx.h"
#include "wg_context.h"
#include "cl_device_api.h"
#include "cpu_dev_limits.h"
#include "cl_sys_defines.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <malloc.h>
#include <assert.h>

using namespace Intel::OpenCL::DeviceBackend;

using namespace Intel::OpenCL::CPUDevice;

WGContext::WGContext() :
    m_pContext(NULL), m_lNDRangeId(-1), m_stPrivMemAllocSize(CPU_DEV_MAX_WG_TOTAL_SIZE),
    m_pLocalMem(NULL), m_pPrivateMem(NULL), m_pCurrentNDRange(NULL)
{
}

WGContext::~WGContext()
{
    if ( NULL != m_pContext )
    {
        m_pContext->Release();
    }
    if ( NULL != m_pLocalMem )
    {
        ALIGNED_FREE(m_pLocalMem);
    }
    if ( NULL != m_pPrivateMem )
    {
        ALIGNED_FREE(m_pPrivateMem);
    }
}

cl_dev_err_code	WGContext::Init()
{
    if (NULL != m_pLocalMem && NULL != m_pPrivateMem)
    {
        return CL_DEV_SUCCESS; // already initialized
    }

    // Create local memory
    m_pLocalMem = (char*)ALIGNED_MALLOC(CPU_DEV_LCL_MEM_SIZE, CPU_DEV_MAXIMUM_ALIGN);
    if ( NULL == m_pLocalMem )
    {
      assert(0 && "Local memory allocation failed");
      return CL_DEV_OUT_OF_MEMORY;
    }

    m_pPrivateMem = ALIGNED_MALLOC(m_stPrivMemAllocSize, CPU_DEV_MAXIMUM_ALIGN);
    if ( NULL == m_pPrivateMem )
    {
        ALIGNED_FREE(m_pLocalMem);
        m_pLocalMem = NULL;
        assert(0 && "Private memory allocation failed");
        return CL_DEV_OUT_OF_MEMORY;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code WGContext::CreateContext(long ndrCmdId, ICLDevBackendBinary_* pBinary, size_t* pBuffSizes, size_t count)
{
    assert( (count<=CPU_MAX_LOCAL_ARGS ) && "Unexpected number of local buffers");

    if ( (NULL == m_pLocalMem) || (NULL == m_pPrivateMem))
    {
        assert(0 && "Execution context memory is not initialized");
        return CL_DEV_OUT_OF_MEMORY;
    }

    InvalidateContext();

    void*	pBuffPtr[CPU_MAX_LOCAL_ARGS+1]; // Additional one for private memory

    // Allocate local memories
    char*	pCurrPtr = m_pLocalMem;
    // The last buffer is private memory (stack) size
    --count;
    for(size_t i=0;i<count;++i)
    {
        pBuffPtr[i] = pCurrPtr;
        pCurrPtr += pBuffSizes[i];
    }

    // Check allocated size of the private memory
    if ( m_stPrivMemAllocSize < pBuffSizes[count] )
    {
        assert(0 && "Reqired private size is greater than allowed one");
        return CL_DEV_OUT_OF_MEMORY;
    }

    pBuffPtr[count] = m_pPrivateMem;

    cl_dev_err_code rc = pBinary->CreateExecutable(pBuffPtr, count+1, &m_pContext);
    if (CL_DEV_FAILED(rc))
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_lNDRangeId = ndrCmdId;
    return CL_DEV_SUCCESS;
}

void WGContext::InvalidateContext()
{
    if ( NULL != m_pContext )
    {
        m_pContext->Release();
        m_pContext = NULL;
    }
    m_lNDRangeId = -1;
}