/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  WGContext.cpp

\*****************************************************************************/

#include "WGContext.h"
#include "cl_device_api.h"
#include "mic_dev_limits.h"
//#include "cl_sys_defines.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <malloc.h>

WGContext::WGContext(): m_pContext(NULL), m_stPrivMemAllocSize(MIC_DEFAULT_WG_SIZE*MIC_DEV_MIN_WI_PRIVATE_SIZE)
{
    // Create local memory
    m_pLocalMem = (char*)_mm_malloc(MIC_DEV_LCL_MEM_SIZE, MIC_DEV_MAXIMUM_ALIGN);
    m_pPrivateMem = _mm_malloc(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN);
}

WGContext::~WGContext()
{
    if ( NULL != m_pContext )
    {
        m_pContext->Release();
    }

    if ( NULL != m_pLocalMem )
    {
        _mm_free(m_pLocalMem);
    }

    if ( NULL != m_pPrivateMem )
    {
        _mm_free(m_pPrivateMem);
    }
}

cl_dev_err_code WGContext::CreateContext(ICLDevBackendBinary_* pBinary, size_t* pBuffSizes, size_t count)
{
    if ( (NULL == m_pLocalMem) || (NULL == m_pPrivateMem))
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    if ( NULL != m_pContext )
    {
        m_pContext->Release();
        m_pContext = NULL;
    }

    void*	pBuffPtr[MIC_MAX_LOCAL_ARGS+2]; // Additional two for implicit and private

    // Allocate local memories
    char*	pCurrPtr = m_pLocalMem;
    // The last buffer is private memory (stack) size
    --count;
    for(size_t i=0;i<count;++i)
    {
        pBuffPtr[i] = pCurrPtr;
        pCurrPtr += pBuffSizes[i];
    }

    // Check allocated size of the private memory, and allocate new if nessesary.
    if ( m_stPrivMemAllocSize < pBuffSizes[count] )
    {
        _mm_free(m_pPrivateMem);
        m_stPrivMemAllocSize = pBuffSizes[count];
        m_pPrivateMem = _mm_malloc(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN);
        if (NULL == m_pPrivateMem)
        {
            return CL_DEV_OUT_OF_MEMORY;
        }
    }

    pBuffPtr[count] = m_pPrivateMem;

    cl_dev_err_code rc = pBinary->CreateExecutable(pBuffPtr, count+1, &m_pContext);
    if (CL_DEV_FAILED(rc))
    {
        return CL_DEV_ERROR_FAIL;
    }
    return CL_DEV_SUCCESS;
}
