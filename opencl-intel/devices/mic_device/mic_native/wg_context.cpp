// Copyright (c) 2006-2008 Intel Corporation
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
* Implemenation of WGExecutor class, class that manages execution of single Work-Group
*
*/

#include "wg_context.h"
#include "cl_device_api.h"
#include "mic_dev_limits.h"
#include "cl_sys_defines.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <malloc.h>

using namespace Intel::OpenCL::MICDevice;

WGContext::WGContext(): m_pContext(NULL), m_cmdId(0), m_stPrivMemAllocSize(MIC_DEFAULT_WG_SIZE*MIC_DEV_MIN_WI_PRIVATE_SIZE)
{
	// Create local memory
	m_pLocalMem = (char*)ALIGNED_MALLOC(MIC_DEV_LCL_MEM_SIZE, MIC_DEV_MAXIMUM_ALIGN);
	m_pPrivateMem = ALIGNED_MALLOC(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN);

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

cl_dev_err_code WGContext::CreateContext(cl_dev_cmd_id cmdId, ICLDevBackendBinary_* pBinary, size_t* pBuffSizes, size_t count)
{
	if ( (NULL == m_pLocalMem) || (NULL == m_pPrivateMem))
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	if ( NULL != m_pContext )
	{
		m_pContext->Release();
		m_pContext = NULL;
		m_cmdId = 0;
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
		ALIGNED_FREE(m_pPrivateMem);
		m_stPrivMemAllocSize = pBuffSizes[count];
		m_pPrivateMem = ALIGNED_MALLOC(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN);
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
	m_cmdId = cmdId;
	return CL_DEV_SUCCESS;
}
