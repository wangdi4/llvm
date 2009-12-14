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

WGContext::WGContext(): m_pContext(NULL), m_cmdId(0)
{
	// Create local memory
	m_pLocalMem = (char*)_aligned_malloc(CPU_DEV_LCL_MEM_SIZE, 64);
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
}

int WGContext::CreateContext(cl_dev_cmd_id cmdId, ICLDevBackendBinary* pExec, size_t* pBuffSizes, size_t count)
{
	if ( NULL != m_pContext )
	{
		m_pContext->Release();
		m_pContext = NULL;
		m_cmdId = 0;
	}

	// TODO: consider use constant array
	void*	*pBuffPtr = new void*[count];
	if ( NULL == pBuffPtr )
		return CL_DEV_OUT_OF_MEMORY;

	char*	pCurrPtr = m_pLocalMem;
	for(size_t i=0;i<count;++i)
	{
		pBuffPtr[i] = pCurrPtr;
		pCurrPtr += pBuffSizes[i];
	}

	int rc = pExec->CreateExecutable(pBuffPtr, count, &m_pContext);

	delete[] pBuffPtr;
	if (CL_DEV_FAILED(rc))
	{
		return CL_DEV_ERROR_FAIL;
	}
	m_cmdId = cmdId;
	return CL_DEV_SUCCESS;
}
