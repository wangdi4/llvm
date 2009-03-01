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
#include "wg_executor.h"
#include "cl_device_api.h"
#include "vector_counter.h"
#include "cpu_dev_limits.h"

#include <windows.h>

// Prototype of external assembly function for kernel execution

extern "C" void	CALLBACK _KernelExecute(void* execParams);
//extern "C" void	CALLBACK _KernelFiberExecute(void* execParams);

using namespace Intel::OpenCL::CPUDevice;

WGExecutor::WGExecutor():
	m_psWGInfo(NULL), m_psKernelInfo(NULL), m_psWIinfo(NULL), m_iWIcount(0), m_uiTaskId(-1), m_fbWGFiber(NULL)
{
	m_pLocalMem = malloc(CPU_DEV_LCL_MEM_SIZE);
}

WGExecutor::~WGExecutor()
{
	Destroy();

	if ( NULL != m_pLocalMem )
	{
		free(m_pLocalMem);
	}
}

int WGExecutor::Initialize(unsigned int uiTaskId, const SKernelInfo* pKernelInfo, const SWGinfo* pWGInfo)
{
	SWorkDim*		psWorkDim = pWGInfo->pWorkingDim;
	int				iDim = psWorkDim->iWorkDim;
	unsigned int	viGlbInitVal[MAX_WORK_DIM] = {0};
	unsigned int	viGlbMaxVal[MAX_WORK_DIM] = {0};
	unsigned int	viLclInitVal[MAX_WORK_DIM] = {0};

	if ( NULL == m_pLocalMem)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Destroy previoulsy allocated data
	Destroy();

	// Copy WG & Kernel information
	m_psKernelInfo = pKernelInfo;
	m_psWGInfo = pWGInfo;

	m_iWIcount = 1;

	// Tototal items cound is a mutliply of all working dimensions
	for(int i=0; i<iDim; ++i)
	{
		m_iWIcount *= psWorkDim->viLocalSize[i];
		// Calculate initial and maximum global WI offset
		viGlbInitVal[i] = psWorkDim->viLocalSize[i]*pWGInfo->viGroupId[i];
		viGlbMaxVal[i] = viGlbInitVal[i] + psWorkDim->viLocalSize[i];
	}

	if ( 0 == m_iWIcount )
	{
		return CL_DEV_INVALID_WRK_ITEM_SIZE;
	}

	// Allocate structure for all work items
	m_psWIinfo = new SWIExecItem[m_iWIcount];
	if ( NULL == m_psWIinfo )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Initilize local memory buffers
	cl_char*	pCurrLocalPtr = (cl_char*)m_pLocalMem;
	for(unsigned int i=0; i<pKernelInfo->uiNumLocal; ++i)
	{
		size_t locSize = (size_t)pKernelInfo->pLocalPtr[i];	// Retrieve buffer size
		pKernelInfo->pLocalPtr[i] = pCurrLocalPtr;			// Set current local pinter
		pCurrLocalPtr += locSize;							// Advance current local pointer
	}

	VectorCounter<unsigned int> vcLocalId(iDim, viLclInitVal, m_psWGInfo->pWorkingDim->viLocalSize);
	VectorCounter<unsigned int> vcGlobalId(iDim, viGlbInitVal, viGlbMaxVal);
	
	// Initilize WI objects
	for(unsigned int i=0; i<m_iWIcount; ++i)
	{
		// Set WI information
		memcpy(m_psWIinfo[i].sWIinfo.viLocalId, vcLocalId.GetValue(), iDim * sizeof(int));
		memcpy(m_psWIinfo[i].sWIinfo.viGlobalId, vcGlobalId.GetValue(), iDim * sizeof(int));
		vcLocalId.Inc();
		vcGlobalId.Inc();
		// Set Execution parameters
		m_psWIinfo[i].sWIExecParam.psWGExecParam = pKernelInfo;
		m_psWIinfo[i].sWIExecParam.psWGInfo = pWGInfo;
		m_psWIinfo[i].sWIExecParam.psWIInfo = &m_psWIinfo[i].sWIinfo;
		m_psWIinfo[i].sWIExecParam.pWGFiber = &m_fbWGFiber;

		// Create Fiber for Work-Item
#ifdef __USING_FIBERS__
		//m_psWIinfo[i].pFiber = CreateFiberEx(1024, 8096, 0,/*FIBER_FLAG_FLOAT_SWITCH,*/ ExecuteWI, &m_psWIinfo[i].sWIExecParam);
		m_psWIinfo[i].pFiber = CreateFiber(8096, ExecuteWI, &m_psWIinfo[i].sWIExecParam);
#else
		m_psWIinfo[i].pFiber = NULL;
#endif
		m_lReady.push_back(&m_psWIinfo[i]);
	}

	// Update last task id
	m_uiTaskId = uiTaskId;

	return CL_DEV_SUCCESS;
}

int	WGExecutor::UpdateGroup(const SWGinfo* pWGInfo)
{
	int	iDim = m_psWGInfo->pWorkingDim->iWorkDim;		
	int	viGlbInitVal[MAX_WORK_DIM] = {0};
	int	viGlbMaxVal[MAX_WORK_DIM] = {0};

	m_psWGInfo = pWGInfo;

	// Calculate initial and maximum global WI offset
	for(int i=0; i<iDim; ++i)
	{
		viGlbInitVal[i] = m_psWGInfo->pWorkingDim->viLocalSize[i]*m_psWGInfo->viGroupId[i];
		viGlbMaxVal[i] = viGlbInitVal[i] + m_psWGInfo->pWorkingDim->viLocalSize[i];
	}

	VectorCounter<int> vcGlobalId(iDim, viGlbInitVal, viGlbMaxVal);

	// Upadate WI objects
	for(unsigned int i=0; i<m_iWIcount; ++i)
	{
		m_psWIinfo[i].sWIExecParam.psWGInfo = pWGInfo;
		memcpy(m_psWIinfo[i].sWIinfo.viGlobalId, vcGlobalId.GetValue(), iDim * sizeof(int));
		vcGlobalId.Inc();
	}

	return CL_DEV_SUCCESS;
}

int WGExecutor::Execute()
{
#ifdef __USING_FIBERS__
	//ms_pMyFiber = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
	m_fbWGFiber = ConvertThreadToFiber(NULL);
#endif
	while ( !m_lReady.empty() )
	{
		SWIExecItem* top = m_lReady.front();
		m_lReady.pop_front();
#ifdef __USING_FIBERS__
		SwitchToFiber(top->pFiber);
#else
		_KernelExecute(&top->sWIExecParam);
#endif
	}
#ifdef __USING_FIBERS__
	ConvertFiberToThread();
#endif

	// Place fibers again to the ready list
	for(unsigned int i=0; i<m_iWIcount; ++i)
	{
		m_lReady.push_back(&m_psWIinfo[i]);
	}

	return CL_DEV_SUCCESS;
}

void WGExecutor::Destroy()
{
	m_lReady.clear();

	if ( NULL != m_psWIinfo )
	{
		for(unsigned int i=0; i<m_iWIcount; ++i)
		{
			DeleteFiber(m_psWIinfo[i].pFiber);
			m_psWIinfo[i].pFiber = NULL;
		}
		delete []m_psWIinfo;
		m_psWGInfo = NULL;
	}
}

void WGExecutor::ExecuteWI(void* param)
{
	SWIExecutionParam* pWIExecParam = (SWIExecutionParam*)param;

	while (1)
	{
		// Execute single WI
		_KernelExecute(param);
		// Switch back to main context
		SwitchToFiber(*pWIExecParam->pWGFiber);
	}
}
