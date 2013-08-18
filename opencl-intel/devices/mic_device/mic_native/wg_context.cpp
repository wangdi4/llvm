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
* Implemenation of WGExecutor class, class that manages execution of single Work-Group
*
*/

#include "wg_context.h"
#include "cl_device_api.h"
#include "mic_dev_limits.h"
#include "cl_sys_defines.h"
#include "native_program_service.h"
#include <malloc.h>
#include <assert.h>
#include "native_globals.h"

using namespace Intel::OpenCL::MICDeviceNative;

WGContext::WGContext(): m_pContext(NULL), m_cmdId((cl_dev_cmd_id)-1), m_stPrivMemAllocSize(MIC_DEV_MAX_WG_TOTAL_SIZE), m_pPrintHandle(NULL)
{
	// Create local memory
	m_pLocalMem = (char*)ALIGNED_MALLOC(MIC_DEV_LCL_MEM_SIZE, MIC_DEV_MAXIMUM_ALIGN);
	m_pPrivateMem = ALIGNED_MALLOC(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN);
	
	ProgramService::getInstance().create_executable(&m_pContext);
}

WGContext::~WGContext()
{
    if ( NULL != m_pLocalMem )
    {
        ALIGNED_FREE(m_pLocalMem);
    }

    if ( NULL != m_pPrivateMem )
    {
        ALIGNED_FREE(m_pPrivateMem);
    }
	
    if ( NULL != m_pContext )
    {
        m_pContext->Release();
        m_pContext = NULL;
    }
}

cl_dev_err_code WGContext::UpdateContext(cl_dev_cmd_id cmdId, ICLDevBackendKernel_* pKernel, ICLDevBackendBinary_* pBinary, size_t* pBuffSizes, size_t count, PrintfHandle* pPrintHandle)
{
	 assert( (count<=MIC_MAX_LOCAL_ARGS ) && "Unexpected number of local buffers");
	// Nullify m_pContext in order to be able to use InvalidateContext() in case of failure
	if ( (NULL == m_pLocalMem) || (NULL == m_pPrivateMem))
	{
		assert(0 && "Memory regions are not allocated");
		return CL_DEV_OUT_OF_MEMORY;
	}

	void*	pBuffPtr[MIC_MAX_LOCAL_ARGS+1]; // Additional one for the private memory

	// Allocate local memories
	char*	pCurrPtr = m_pLocalMem;
	// The last buffer is private memory (stack) size
	--count;
	for(size_t i=0;i<count;++i)
	{
		pBuffPtr[i] = pCurrPtr;
		pCurrPtr += pBuffSizes[i];
	}

	if ( m_stPrivMemAllocSize < pBuffSizes[count] )
	{
      assert(0 && "Private Memory is not enough");
      return CL_DEV_OUT_OF_MEMORY;
	}

	pBuffPtr[count] = m_pPrivateMem;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("WGContext::UpdateContext()->ICLDevBackendExecutable()::Init()");
      }
      __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif
	cl_dev_err_code rc = m_pContext->Init(pBuffPtr, m_pPrivateMem, pBinary);
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    // Monitor only IN-ORDER queue
    if ( gMicGPAData.bUseGPA )
    {
      __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif

	if (CL_DEV_FAILED(rc))
	{
		assert(0 && "Context initialization failed");
		return CL_DEV_ERROR_FAIL;
	}

	// Set PrintHandle
	m_pPrintHandle = pPrintHandle;

	m_hw_wrapper.SetKernel(pKernel);

	m_cmdId = cmdId;
	return CL_DEV_SUCCESS;
}

void WGContext::InvalidateContext()
{
	  m_pPrintHandle = NULL;
    m_cmdId = (cl_dev_cmd_id)-1;
}

