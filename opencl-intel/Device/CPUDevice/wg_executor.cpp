#include "stdafx.h"
#include "wg_executor.h"
#include "cl_device_api.h"
#include "vector_counter.h"
#include <windows.h>

// Prototype of external assembly function for kernel execution

extern "C" void	CALLBACK _KernelExecute(void* execParams);
//extern "C" void	CALLBACK _KernelFiberExecute(void* execParams);

using namespace Intel::OpenCL::CPUDevice;

// Static members initialization
void* WGExecutor::ms_pWGFiber = NULL;

WGExecutor::WGExecutor():
	m_psWGInfo(NULL), m_psKernelInfo(NULL), m_psWIinfo(NULL), m_iWIcount(0), m_uiTaskId(-1)
{
}

WGExecutor::~WGExecutor()
{
	Destroy();
}

int WGExecutor::Initialize(unsigned int uiTaskId, const SKernelInfo* pKernelInfo, const SWGinfo* pWGInfo)
{
	SWorkDim*	psWorkDim = pWGInfo->pWorkingDim;
	int			iDim = psWorkDim->iWorkDim;
	int			viGlbInitVal[MAX_DIMENSION] = {0};
	int			viGlbMaxVal[MAX_DIMENSION] = {0};
	int			viLclInitVal[MAX_DIMENSION] = {0};

	// Copy WG & Kernel information
	m_psKernelInfo = pKernelInfo;
	m_psWGInfo = pWGInfo;

	m_iWIcount = 1;

	m_lReady.clear();
	// Remove previously allocated memory
	if ( NULL != m_psWIinfo )
	{
		delete []m_psWIinfo;
		m_psWIinfo = NULL;
	}

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

	VectorCounter<int> vcLocalId(iDim, viLclInitVal, m_psWGInfo->pWorkingDim->viLocalSize);
	VectorCounter<int> vcGlobalId(iDim, viGlbInitVal, viGlbMaxVal);
	
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

int	WGExecutor::UpdateGroupId(const int pGroupId[])
{
	int	iDim = m_psWGInfo->pWorkingDim->iWorkDim;		
	int	viGlbInitVal[MAX_DIMENSION] = {0};
	int	viGlbMaxVal[MAX_DIMENSION] = {0};

	// Calculate initial and maximum global WI offset
	for(int i=0; i<iDim; ++i)
	{
		viGlbInitVal[i] = m_psWGInfo->pWorkingDim->viLocalSize[i]*pGroupId[i];
		viGlbMaxVal[i] = viGlbInitVal[i] + m_psWGInfo->pWorkingDim->viLocalSize[i];
	}

	VectorCounter<int> vcGlobalId(iDim, viGlbInitVal, viGlbMaxVal);

	// Upadate WI objects
	for(unsigned int i=0; i<m_iWIcount; ++i)
	{
		memcpy(m_psWIinfo[i].sWIinfo.viGlobalId, vcGlobalId.GetValue(), iDim * sizeof(int));
		vcGlobalId.Inc();
	}

	return CL_DEV_SUCCESS;
}

int WGExecutor::Execute()
{
#ifdef __USING_FIBERS__
	//ms_pMyFiber = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
	ms_pWGFiber = ConvertThreadToFiber(NULL);
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
	for(unsigned int i=0; i<m_iWIcount; ++i)
	{
		DeleteFiber(m_psWIinfo[i].pFiber);
		m_psWIinfo[i].pFiber = NULL;
	}

	m_lReady.clear();
	if ( NULL != m_psWIinfo )
	{
		delete []m_psWIinfo;
	}

}

void WGExecutor::ExecuteWI(void* param)
{
	while (1)
	{
		// Execute single WI
		_KernelExecute(param);
		// Switch back to main context
		SwitchToFiber(ms_pWGFiber);
	}
}

