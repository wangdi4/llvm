#include "stdafx.h"
#include "task_executor.h"
#include "wg_executor.h"
#include "vector_counter.h"

// LRB SDK XNTask library
#include <XN0Task_common.h>
#include <XN0Sys_common.h>
#include <limits.h>

using namespace Intel::OpenCL::CPUDevice;

// Local functions
ETERetCode ConvertXNError(XNERROR err)
{
	switch (err)
	{
	case XN_SUCCESS:
		return TE_SUCCESS;

	case XN_ALREADY_INITIALIZED:
		return TE_ALREADY_INITILIZED;

	case XN_OUT_OF_RANGE:
		return TE_OUT_OF_RANGE;

	case XN_OUT_OF_MEMORY:
		return TE_OUT_OF_MEMORY;

	case XN_RESOURCE_EXHAUSTED:
		return TE_RESOURCE_EXHAUSTED;
	}

	return TE_ERROR;
}

// Class Construtor/Destructor/Intstance
TaskExecutor::TaskExecutor() : 
	m_uiNumWorkingThreads(0), m_uiHWThreads(0), m_haHandles(1, UINT_MAX)
{
}

TaskExecutor::~TaskExecutor()
{
}

//------------------------------------------------------------------------------------------------
// Initialization/Destruction functions

// Init task executor
ETERetCode	TaskExecutor::Init(unsigned int uiNumThreads)
{
	m_uiHWThreads = XN0SysGetHardwareThreadCount();

	if ( 0 == uiNumThreads )	// we should deside for number of threads
	{
		m_uiNumWorkingThreads = m_uiHWThreads;
	}
	else
	{
		m_uiNumWorkingThreads = uiNumThreads;
	}

	// Initilize XNTask library
	XNERROR err;
	err = XN0TaskInit(m_uiNumWorkingThreads, NULL, 0);
	if ( XN_SUCCESS != err )
	{
		return ConvertXNError(err);
	}

	// Allocate Work-Groups executirs entries as a number of working threads
	for(unsigned int i=0; i<m_uiNumWorkingThreads; ++i)
	{
		WGExecutor* pExecutor = new WGExecutor;
		if ( NULL == pExecutor)
		{
			XN0TaskShutdown();
			return TE_OUT_OF_MEMORY;
		}
		m_lWGExecutors.push_back(pExecutor);
	}

	return TE_SUCCESS;
}

// Wait for all task completion and then release Task Executor resourses
void TaskExecutor::Close()
{
	// Free allocated resources for WG executors
	if ( !m_lWGExecutors.empty() )
	{
		TWGExecutorList::iterator it;
		
		for(it = m_lWGExecutors.begin(); it!= m_lWGExecutors.end(); ++it)
		{
			WGExecutor* pExecutor = *it;
			delete pExecutor;
		}

		m_lWGExecutors.clear();
	}

	// Shutdwon Task System
	XN0TaskShutdown();
}

// Execute a Task comprosed of kernels
ETERetCode	TaskExecutor::ExecuteTask(STaskDescriptor* pTaskDesc,
					TTaskNotifier* pfnNotify, void* pData,
					TTaskHandle* pDepList, unsigned int uiDepListCount,
					TTaskHandle* pTask)
{
	int		viNumGroups[MAX_WORK_DIM];
	unsigned int uiGroupCount = 1;

	// Calculate total number of tasks to execute
	for(unsigned int i=0; i<pTaskDesc->sWorkingDim.iWorkDim; ++i)
	{
		unsigned int tmp = pTaskDesc->sWorkingDim.viGlobalSize[i]/pTaskDesc->sWorkingDim.viLocalSize[i];
		viNumGroups[i] = tmp;
		uiGroupCount *= tmp;
	}
	if ( 0 == uiGroupCount )
	{
		return TE_OUT_OF_RANGE;
	}

	// Setup new task record
	unsigned int	uiNewTaskId;
	bool			rc;
	
	rc = m_haHandles.AllocateHandle(&uiNewTaskId);
	if ( false == rc )
	{
		return TE_RESOURCE_EXHAUSTED;
	}

	SRunningTask*	pNewTask = new SRunningTask;
	if ( NULL == pNewTask )
	{
		m_haHandles.FreeHandle(uiNewTaskId);
		return TE_OUT_OF_MEMORY;
	}

	// Setup task information
	pNewTask->psTaskDesc = pTaskDesc;

	// Allocate data for holding all working elements in the set
	pNewTask->psWGInfo = new SWGinfo[uiGroupCount];
	if ( NULL == pNewTask->psWGInfo )
	{
		delete pNewTask;
		m_haHandles.FreeHandle(uiNewTaskId);
		return TE_OUT_OF_MEMORY;
	}

	if ( NULL != pfnNotify )
	{
		// Allocate memory for task notification function
		pNewTask->pScratch = malloc(XN_SYNC_OBJECT_SCRATCH_SIZE);
		if ( NULL == pNewTask->pScratch)
		{
			delete []pNewTask->psWGInfo;
			delete pNewTask;
			m_haHandles.FreeHandle(uiNewTaskId);
			return TE_OUT_OF_MEMORY;
		}
	}
	else
	{
		pNewTask->pScratch = NULL;
	}

	pNewTask->uiTaskId = uiNewTaskId;
	pNewTask->pTE = this;

	// Setup WG information parameters
	int viInitVal[MAX_WORK_DIM] = {0};

	VectorCounter<int> vcGroupId(pTaskDesc->sWorkingDim.iWorkDim, viInitVal, viNumGroups);
	for(unsigned int i=0; i<uiGroupCount; ++i)
	{
		pNewTask->psWGInfo[i].pWorkingDim = &(pNewTask->psTaskDesc->sWorkingDim);
		memcpy(pNewTask->psWGInfo[i].viNumGroups, viNumGroups, pTaskDesc->sWorkingDim.iWorkDim*sizeof(int));
		memcpy(pNewTask->psWGInfo[i].viGroupId, vcGroupId.GetValue(), pTaskDesc->sWorkingDim.iWorkDim*sizeof(int));
		vcGroupId.Inc();
	}

	XNTASK xnNewTask;
	XNERROR xnRC;
	xnRC = XN0TaskCreateSet(uiGroupCount, XN_TASK_PRIORITY_MEDIUM, TaskExecutionRoutine, pNewTask,
		(XNTASK*)pDepList, uiDepListCount, &xnNewTask);
	if ( XN_SUCCESS != xnRC )
	{
		if ( NULL != pNewTask->pScratch )
		{
			free(pNewTask->pScratch);
		}
		delete []pNewTask->psWGInfo;
		delete pNewTask;
		m_haHandles.FreeHandle(uiNewTaskId);
		return ConvertXNError(xnRC);
	}

	if ( NULL == pfnNotify )
	{
		return TE_SUCCESS;
	}

	// Register callback function for  Task completion
	pNewTask->pfnNotify = pfnNotify;
	pNewTask->pData = pData;
	XN0SyncObjectAddWaiter(xnNewTask, pNewTask->pScratch, (XN_SYNC_OBJECT_CALLBACK)TaskCompleted, pNewTask);

	return TE_SUCCESS;
}

// Execute single Kernel/Function
ETERetCode	TaskExecutor::ExecuteFunction(const void* pfnFunction, void* pParams, size_t stSize,
					TFunctionNotifier* pfnNotify, void* pData,
					TTaskHandle* pDepList, unsigned int uiDepListCount,
					TTaskHandle* pTask)
{
	unsigned int	uiNewTaskId;
	bool bRC = m_haHandles.AllocateHandle(&uiNewTaskId);
	if ( false == bRC )
	{
		return TE_RESOURCE_EXHAUSTED;
	}

	SRunningFunction*	pNewFunc = new SRunningFunction;
	if ( NULL == pNewFunc )
	{
		m_haHandles.FreeHandle(uiNewTaskId);
		return TE_OUT_OF_MEMORY;
	}

	if ( NULL != pfnNotify )
	{
		// Allocate memory for task notification function
		pNewFunc->pScratch = malloc(XN_SYNC_OBJECT_SCRATCH_SIZE);
		if ( NULL == pNewFunc->pScratch)
		{
			delete pNewFunc;
			m_haHandles.FreeHandle(uiNewTaskId);
			return TE_OUT_OF_MEMORY;
		}
	}
	else
	{
		pNewFunc->pScratch = NULL;
	}

	// Fill function informatio
	pNewFunc->uiTaskId = uiNewTaskId;
	pNewFunc->pTE = this;
	pNewFunc->pParams = pParams;
	pNewFunc->stSize = stSize;

	XNTASK xnNewTask;
	XNERROR xnRC;
	xnRC = XN0TaskCreate(XN_TASK_PRIORITY_MEDIUM, (XNTaskFunction)pfnFunction, pParams,
		(XNTASK*)pDepList, uiDepListCount, &xnNewTask);
	if ( XN_SUCCESS != xnRC )
	{
		if ( NULL != pNewFunc->pScratch )
		{
			free(pNewFunc->pScratch);
		}
		delete pNewFunc;
		m_haHandles.FreeHandle(uiNewTaskId);
		return ConvertXNError(xnRC);
	}

	if ( NULL == pfnNotify )
	{
		return TE_SUCCESS;
	}

	// Register callback function for  Task completion
	pNewFunc->pfnNotify = pfnNotify;
	pNewFunc->pData = pData;
	XN0SyncObjectAddWaiter(xnNewTask, pNewFunc->pScratch, (XN_SYNC_OBJECT_CALLBACK)FunctionCompleted, pNewFunc);

	return TE_SUCCESS;
}

ETERetCode TaskExecutor::FreeTaskHandle(TTaskHandle pTask)
{
	XNERROR xnRC;

	xnRC = XN0TaskDecRef((XNTASK)pTask, NULL);

	return ConvertXNError(xnRC);
}

/************************************************************************************************************
*	Implementation of Internal commands
*************************************************************************************************************/
// Task execution function
void TaskExecutor::TaskExecutionRoutine(void* _Task, unsigned int uiIndex, unsigned int uiSize)
{
	SRunningTask* pTask = (SRunningTask*)_Task;
	TaskExecutor* _this = pTask->pTE;
	const SWGinfo*	pWGInfo = &(pTask->psWGInfo[uiIndex]);
	WGExecutor* pWGExec = NULL;

	// Get available WG executer
	_this->m_muWGExecList.Lock();
	if ( 0 != _this->m_lWGExecutors.size() )
	{
		pWGExec = _this->m_lWGExecutors.front();
		_this->m_lWGExecutors.pop_front();
	}
	_this->m_muWGExecList.Unlock();

	// No available WG executors -> allocate new one
	if ( NULL == pWGExec )
	{
		pWGExec = new WGExecutor;
		if ( NULL == pWGExec)
		{
			throw;
		}
	}

	// Check if WG executer was configured to execute current task
	if ( pWGExec->GetCurrentTaskid() != pTask->uiTaskId )
	{
		// Configured for other task, configure for this one
		pWGExec->Initialize(pTask->uiTaskId, &(pTask->psTaskDesc->sKernelParam), pWGInfo);
	}
	else
	{
		// Running this task -> update global invormation
		pWGExec->UpdateGroupId(pWGInfo->viGroupId);
	}

	// Execute WG
	pWGExec->Execute();

	// Return WG executer to global pool
	_this->m_muWGExecList.Lock();
	_this->m_lWGExecutors.push_back(pWGExec);
	_this->m_muWGExecList.Unlock();
}

// Completion notification functions
void TaskExecutor::TaskCompleted(TTaskHandle hTask, void* _Task)
{
	SRunningTask* pTask = (SRunningTask*)_Task;

	XNERROR xnER;
	
//	xnER = XN0SyncObjectRemoveWaiter((XNTASK)hTask, pTask->pScratch);
//	assert(XN_SUCCESS == xnER);

	// Free task ID
	pTask->pTE->m_haHandles.FreeHandle(pTask->uiTaskId);

	// Call user callback
	pTask->pfnNotify(hTask, pTask->psTaskDesc, pTask->pData);

	// Relese object
	delete pTask;
}

void TaskExecutor::FunctionCompleted(TTaskHandle hTask, void* _Function)
{
	SRunningFunction*	pFunc = (SRunningFunction*)_Function;

	XNERROR xnER;
	
//	xnER = XN0SyncObjectRemoveWaiter((XNTASK)hTask, pFunc->pScratch);
//	assert(XN_SUCCESS == xnER);

	// Free task ID
	pFunc->pTE->m_haHandles.FreeHandle(pFunc->uiTaskId);

	// Call user callback
	pFunc->pfnNotify(hTask, pFunc->pParams, pFunc->stSize, pFunc->pData);

	// Relese object
	delete pFunc;
}