// Copyright (c) 2006-2012 Intel Corporation
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

#pragma once

#include "task_executor.h"
#include "cl_shared_ptr.hpp"
#include "cl_object_pool.h"

using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::AtomicCounter;

class WGContextPool : public IWGContextPool
{
public:
    virtual WGContextBase* GetWGContext(bool bBelongsToMasterThread) { return m_pool.Malloc(); }

    virtual void ReleaseWorkerWGContext(WGContextBase* wgContext) { m_pool.Free(wgContext); }

private:

    Intel::OpenCL::Utils::ObjectPool<WGContextBase> m_pool;
};

/**
 * This class represents a tester object for TaskExecutor module.
 * Currently just testing of TBBTaskExecutor is implemented.
 */
class TaskExecutorTester : public ITaskExecutorObserver
{
public:

    TaskExecutorTester()
    {
        if (NULL == m_pTaskExecutor)
        {
            m_pTaskExecutor = Intel::OpenCL::TaskExecutor::GetTaskExecutor();
            m_pTaskExecutor->Init(NULL, TE_AUTO_THREADS, NULL);            
        }
    }

    ~TaskExecutorTester()
    {
    }

    ITaskExecutor* GetTaskExecutor() { return m_pTaskExecutor; }

    // ITaskExecutorObserver
    virtual void*  OnThreadEntry() 
    {
        WGContextBase* pCtx = m_wgContextPool.GetWGContext( m_pTaskExecutor->IsMaster() );
        pCtx->SetThreadId( m_pTaskExecutor->GetPosition() );
        return pCtx;
    }

    virtual void   OnThreadExit( void* currentThreadData )
    {
        WGContextBase* pCtx = static_cast<WGContextBase*>(currentThreadData);
        pCtx->SetThreadId( -1 );
        m_wgContextPool.ReleaseWorkerWGContext( pCtx );
    }

    TE_BOOLEAN_ANSWER  MayThreadLeaveDevice( void* currentThreadData ) 
    { 
        return TE_USE_DEFAULT; 
    }

private:

    static ITaskExecutor* m_pTaskExecutor;
    WGContextPool m_wgContextPool;

};

class TesterTaskSet : public ITaskSet
{
public:

    PREPARE_SHARED_PTR(TesterTaskSet)

    static SharedPtr<TesterTaskSet> Allocate(unsigned int uiNumDims, AtomicCounter* pUncompletedTasks)
    {
    	return SharedPtr<TesterTaskSet>(new TesterTaskSet(uiNumDims, pUncompletedTasks));
    }

    // overriden methods:

    virtual bool CompleteAndCheckSyncPoint() { return false; }
	
    virtual bool SetAsSyncPoint() { return false; }

    virtual bool IsCompleted() const { return m_bIsComplete; }

    virtual long Release() { return 0; }

    virtual void Cancel() {}

    virtual int  Init(size_t region[], unsigned int& regCount, size_t numberOfThreads)
    {
        for (unsigned int i = 0; i < m_uiNumDims; i++)
        {
            region[i] = 1;
        }
        regCount = m_uiNumDims;
        return 0;
    }

    virtual void* AttachToThread(void* pWgContext, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) { return pWgContext; }

    virtual void DetachFromThread(void* pWgContext) {}

    virtual bool ExecuteIteration(size_t x, size_t y, size_t z, void* pWgContext = NULL) { return true; }

	virtual bool Finish(FINISH_REASON reason)
    {
        m_bIsComplete = true;
        if (NULL != m_pUncompletedTasks)
        {
        	(*m_pUncompletedTasks)--;
        }
        return true;
    }

	virtual Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup* GetNDRangeChildrenTaskGroup() { return NULL; }

    
    // Optimize By
    TASK_PRIORITY         GetPriority() const { return TASK_PRIORITY_MEDIUM;}
    TASK_SET_OPTIMIZATION OptimizeBy() const { return TASK_SET_OPTIMIZE_DEFAULT; }
    unsigned int          PreferredSequentialItemsPerThread() const { return 1; }

private:

    TesterTaskSet(unsigned int uiNumDims, AtomicCounter* pUncompletedTasks) : m_bIsComplete(false), m_uiNumDims(uiNumDims),
    	m_pUncompletedTasks(pUncompletedTasks) { }

    volatile bool m_bIsComplete;
    const unsigned int m_uiNumDims;
    AtomicCounter* const m_pUncompletedTasks;

};

