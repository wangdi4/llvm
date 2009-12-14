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
* File xn_executor.h
*		Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#include "task_executor.h"

// LRB SDK XNTask library
#include <common\XN0Task_common.h>

namespace Intel { namespace OpenCL { namespace TaskExecutor {
/////////////////////////////////////////////////////////////////////////////
// XNTaskImpl - implements basic functions that use XNTask library
class XNTaskImpl
{
public:
	XNTaskImpl(ITaskBase* pTask) :
	  m_hTask(NULL), m_pTask(pTask), m_fExecStub(TaskExecuteStub) {}

	XNTaskImpl(ITaskBase* pTask, XNTaskFunction stub) :
	  m_hTask(NULL), m_pTask(pTask), m_fExecStub(stub) {}

	// ITask interface
	void			Release();

protected:
	friend class	XNTaskListOrderedImpl;
	friend class	XNTaskListUnorderedImpl;
	friend class	XNTaskExecutor;

	virtual			~XNTaskImpl();

	inline int		SpawnTask( XNTaskImpl *pDepend );
	
	ITaskBase*			m_pTask;
	XNTASK				m_hTask;			// Handle to the task instance
	XNTaskFunction		m_fExecStub;

	// The entry point for task execution
	static	void TaskExecuteStub(void* _pTask);
};

/////////////////////////////////////////////////////////////////////////////
// XNTaskSetImpl - implements basic functions that use XNTask library
class XNTaskSetImpl : public XNTaskImpl
{
public:
	XNTaskSetImpl(ITaskBase *pTask) :
	  XNTaskImpl(pTask, TaskSetExecuteStub) {}

	// ITaskDescriptor
  	void	Execute();

protected:
	// The entry point for task execution
	static	void TaskSetExecuteStub(void* _pTaskSetImpl);
	// The entry point for task set execution
	static	void SetExecuteStub(void* _pTaskSet, unsigned int uiIndex, unsigned int uiSetSize);
	// The entry point for Finish() function execution
	static	void FinishExecutionStub(void* _pTaskSetImpl);
	// Working Dimension
	unsigned int	m_region[3];
};

/////////////////////////////////////////////////////////////////////////////
// ITaskList interface - defines a function set for task list handling
class XNTaskListOrderedImpl : public ITaskList
{
public:
	XNTaskListOrderedImpl() :
	  m_pLastTask(NULL) {}

	// ITaskList interface
	unsigned int Enqueue(ITaskBase* pTaskBase);
	void Flush() {}
	void	Release();

protected:
	virtual ~XNTaskListOrderedImpl() {};

	XNTaskImpl*		m_pLastTask;
};

/////////////////////////////////////////////////////////////////////////////
// ITaskList interface - defines a function set for task list handling
class XNTaskListUnorderedImpl : public ITaskList
{
public:
	// ITaskList interface
	unsigned int Enqueue(ITaskBase* pTaskBase);
	void Flush() {}
	void	Release();

protected:
	virtual ~XNTaskListUnorderedImpl() {};
};

class XNTaskExecutor : public ITaskExecutor
{
public:
	XNTaskExecutor() : m_lRefCount(0) {}
	// ITaskExecutor interface
	int	Init(unsigned int uiNumThreads);
	unsigned int GetNumWorkingThreads() const;
	ITaskList* CreateTaskList(bool OOO = false);
	unsigned int	Execute(ITaskBase * pTask);
	void Close(bool bCancel);

protected:
		long		m_lRefCount;
};

}}}