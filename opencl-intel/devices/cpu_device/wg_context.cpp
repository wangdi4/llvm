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

#include "stdafx.h"
#include "wg_context.h"
#include "cl_device_api.h"
#include "cpu_dev_limits.h"

#include <windows.h>
#include <malloc.h>

using namespace Intel::OpenCL::CPUDevice;

WGContext::WGContext(): m_pContext(NULL), m_cmdId(0), m_stPrivMemAllocSize(2*CPU_DEV_MIN_WI_PRIVATE_SIZE)
{
	// Create local memory
	m_pLocalMem = (char*)_aligned_malloc(CPU_DEV_LCL_MEM_SIZE, CPU_DEV_DCU_LINE_SIZE);
	m_pPrivateMem = _aligned_malloc(m_stPrivMemAllocSize, CPU_DEV_DCU_LINE_SIZE);

}

WGContext::~WGContext()
{
	if ( NULL != m_pContext )
	{
		m_pContext->Release();
	}

	if ( NULL != m_pLocalMem )
	{
		_aligned_free(m_pLocalMem);
	}

	if ( NULL != m_pPrivateMem )
	{
		_aligned_free(m_pPrivateMem);
	}

}

int WGContext::CreateContext(cl_dev_cmd_id cmdId, ICLDevBackendBinary* pBinary, size_t* pBuffSizes, size_t count)
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

	void*	pBuffPtr[CPU_MAX_LOCAL_ARGS+2]; // Additional two for implicit and private

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
		_aligned_free(m_pPrivateMem);
		m_stPrivMemAllocSize = pBuffSizes[count];
		m_pPrivateMem = _aligned_malloc(m_stPrivMemAllocSize, CPU_DEV_DCU_LINE_SIZE);
	}

	pBuffPtr[count] = m_pPrivateMem;

	int rc = pBinary->CreateExecutable(pBuffPtr, count+1, &m_pContext);
	if (CL_DEV_FAILED(rc))
	{
		return CL_DEV_ERROR_FAIL;
	}
	m_cmdId = cmdId;
	return CL_DEV_SUCCESS;
}
