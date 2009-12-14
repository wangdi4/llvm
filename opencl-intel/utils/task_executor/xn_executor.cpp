// Copyright (c) 2006-2009 Intel Corporation
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
* File xn_executor.cpp
*		Implementnation of Task system on XNTask
*
*/
#include "stdafx.h"

#if 0 //!defined (__WIN_XP__)
#include "xn_executor.h"
#include <windows.h>
#include <limits.h>
#include <assert.h>

using namespace Intel::OpenCL::TaskExecutor;

// LRB SDK XNTask library
#include <common\XN0Task_common.h>
#include <common\XN0Sys_common.h>

#ifdef _DEBUG
#pragma comment (lib, "xn_host32d.lib")
#else
#pragma comment (lib, "xn_host32r.lib")
#endif

//---------------------------------------------
static volatile int			g_iHWThreads = 0;	// number of actual working threads in the system
#ifdef _DEBUG
volatile long g_TaskCount = 0;
#endif
//-------------------------------------------

XNTaskImpl::~XNTaskImpl()
{
#ifdef _DEBUG
	InterlockedDecrement(&g_TaskCount);
#endif
	m_pTask->Release();
}

void XNTaskImpl::Release()
{
	uint32_t newRefCnt = 0;

	newRefCnt = XN0SyncObjectReferences(&m_hTask, 1);
	XN0SyncObjectRelease(&m_hTask, 1);
	--newRefCnt;

	// Destroy myself, if reference count == 1,
	// The object is hold by the XN system
	if ( newRefCnt > 1 )
		return;

	delete this;
}

int XNTaskImpl::SpawnTask( XNTaskImpl* pDepent )
{
	// Start task
	XNERROR xnRC;
	xnRC = XN0TaskCreate((XN_TASK_PRIORITY)m_pTask->GetPriority(), m_fExecStub, this, &(pDepent->m_hTask), pDepent != NULL, &m_hTask);
	if ( XN_SUCCESS != xnRC )
	{
		return -1;
	}

	// We need another reference of the task
	XN0SyncObjectAddRef(&m_hTask, 1);
	return 0;
}

void XNTaskImpl::TaskExecuteStub(void* _pTaskImpl)
{
	ITask* pTask = (ITask*)(((XNTaskImpl*)_pTaskImpl)->m_pTask);

	pTask->Execute();

	((XNTaskImpl*)_pTaskImpl)->Release();
}

//---------------------------------------------
//---------------------------------------------
// XNTaskSetImpl base functions implementation
void XNTaskSetImpl::TaskSetExecuteStub(void *pTaskSetImpl)
{
 	XNTaskSetImpl*	_this = (XNTaskSetImpl*)pTaskSetImpl;
	ITaskSet *pTaskSet = (ITaskSet*)_this->m_pTask;
	XN_TASK_PRIORITY prio = (XN_TASK_PRIORITY)pTaskSet->GetPriority();

	// Execute initialization function
	int res = pTaskSet->Init(_this->m_region);
	if ( res )
	{
		pTaskSet->Finish(FINISH_INIT_FAILED);
		return;
	}

	unsigned __int64 n = 1;
	for(int i =0; i<3; ++i)
	{
		n *= _this->m_region[i];
	}
	if ( n > ULONG_MAX)
	{
		// Too many instances to execute
		pTaskSet->Finish(FINISH_EXECUTION_FAILED);
		return;
	}
	// if not failed continue execution
	// Create XNTaskSet
	XNTASK xnTaskSet;
	XNERROR xnRC;
	xnRC = XN0TaskCreateSet((unsigned int)n, prio, SetExecuteStub, pTaskSetImpl, NULL, 0, &xnTaskSet);
	if ( XN_SUCCESS != xnRC )
	{
		// TODO: Report an error
		pTaskSet->Finish(FINISH_INIT_FAILED);
		return;
	}
	//XN0TaskExtendCompletion(xnTaskSet);
	XNTASK xnFinishTask;
	// Start Finish() as additional task
	xnRC = XN0TaskCreate(prio, FinishExecutionStub, _this, &xnTaskSet, 1, &xnFinishTask);

	XN0SyncObjectRelease(&xnTaskSet, 1);
	if ( XN_SUCCESS != xnRC )
	{
		// TODO: Report an error
		return;
	}
	XN0TaskExtendCompletion(xnFinishTask);

	XN0SyncObjectRelease(&xnFinishTask, 1);
}

void XNTaskSetImpl::SetExecuteStub(void* pTaskSetImpl, unsigned int uiIndex, unsigned int uiSetSize)
{
 	XNTaskSetImpl*	_this = (XNTaskSetImpl*)pTaskSetImpl;
	ITaskSet *pTaskSet = (ITaskSet*)_this->m_pTask;

	unsigned int uiWorkerId = XN0SysGetHardwareThreadIndex();
	pTaskSet->AttachToThread(uiWorkerId);
	size_t x = uiIndex % _this->m_region[0];
	uiIndex /= _this->m_region[0];
	size_t y = uiIndex % _this->m_region[1];
	uiIndex /= _this->m_region[1];
	size_t z = uiIndex % _this->m_region[2];

	pTaskSet->ExecuteIteration(x, y, z, uiWorkerId);
}

void XNTaskSetImpl::FinishExecutionStub(void* _pTaskSetImpl)
{
	XNTaskSetImpl*	_this = (XNTaskSetImpl*)_pTaskSetImpl;

	((ITaskSet*)(_this->m_pTask))->Finish(FINISH_COMPLETED);

	// Decrease reference count
	_this->Release();
}

//----------------------------------------------------
//----------------------------------------------------
// XNTaskListOrderedImpl - ordered list implementation
unsigned int XNTaskListOrderedImpl::Enqueue(ITaskBase* pTask)
{
	XNTaskImpl*	pTaskImpl;
	// Create XNTask specific
	if ( pTask->IsTaskSet() )
	{
		pTaskImpl = new XNTaskSetImpl(pTask);
	}
	else
	{
		pTaskImpl = new XNTaskImpl(pTask);
	}
	int res = pTaskImpl->SpawnTask(m_pLastTask);
	if ( 0 != res )
		return res;
#ifdef _DEBUG
	InterlockedIncrement(&g_TaskCount);
#endif
	if ( NULL != m_pLastTask )
	{
		m_pLastTask->Release();
	}
	m_pLastTask = pTaskImpl;
	return 0;
}

void XNTaskListOrderedImpl::Release()
{
	if ( NULL != m_pLastTask )
	{
		m_pLastTask->Release();
	}

	delete this;
}

//----------------------------------------------------
//----------------------------------------------------
// XNTaskListOrderedImpl - ordered list implementation
unsigned int XNTaskListUnorderedImpl::Enqueue(ITaskBase* pTask)
{
	XNTaskImpl*	pTaskImpl;
	// Create XNTask specific
	if ( pTask->IsTaskSet() )
	{
		pTaskImpl = new XNTaskSetImpl(pTask);
	}
	else
	{
		pTaskImpl = new XNTaskImpl(pTask);
	}
	int ret =  pTaskImpl->SpawnTask(NULL);
	if ( 0 == ret )
	{
		pTaskImpl->Release();
#ifdef _DEBUG
		InterlockedIncrement(&g_TaskCount);
#endif
	}
	return ret;

}

void XNTaskListUnorderedImpl::Release()
{
	delete this;
}

//---------------------------------------------
//---------------------------------------------
// XNTaskExecutorImpl - task executor implementation
int	XNTaskExecutor::Init(unsigned int uiNumThreads)
{
	unsigned long ulNewVal = InterlockedIncrement(&m_lRefCount);
	if ( ulNewVal > 1 )
	{
		while ( 0 == g_iHWThreads );	// Speen Loop until initialization completed
		return g_iHWThreads;
	}

	unsigned int uiHWThreads = XN0SysGetHardwareThreadCount();

	if ( 0 != uiNumThreads )	// we should deside for number of threads
	{
		uiHWThreads = uiNumThreads;
	}

	// Initilize XNTask library
	XNERROR err;
	err = XN0TaskInit(uiHWThreads, NULL, 0);
	if ( XN_SUCCESS != err )
	{
		g_iHWThreads = -1;
		return -1;
	}

	g_iHWThreads = uiHWThreads;
	return g_iHWThreads;
}

unsigned int XNTaskExecutor::GetNumWorkingThreads() const
{
	return g_iHWThreads;
}

void XNTaskExecutor::Close(bool bCancel)
{
	unsigned long ulNewVal = InterlockedDecrement(&m_lRefCount);
	if ( ulNewVal > 0)
	{
		return;
	}

	assert(g_TaskCount == 0);
	// Shutdwon Task System
	XN0TaskShutdown();
}

ITaskList* XNTaskExecutor::CreateTaskList(bool OOO)
{
	if ( OOO )
	{
		return new XNTaskListUnorderedImpl();
	}

	return new XNTaskListOrderedImpl();
}

unsigned int XNTaskExecutor::Execute(ITaskBase* pTask)
{
	XNTaskImpl*	pTaskImpl;
	// Create XNTask specific
	if ( pTask->IsTaskSet() )
	{
		pTaskImpl = new XNTaskSetImpl(pTask);
	}
	else
	{
		pTaskImpl = new XNTaskImpl(pTask);
	}
	int ret =  pTaskImpl->SpawnTask(NULL);
	if ( 0 == ret )
	{
		pTaskImpl->Release();
#ifdef _DEBUG
		InterlockedIncrement(&g_TaskCount);
#endif
	}
	return ret;
}
#endif