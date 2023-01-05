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

#include "cl_config.h"
#include "cl_object_pool.h"
#include "cl_shared_ptr.hpp"
#include "task_executor.h"

using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::Utils::AtomicCounter;
using Intel::OpenCL::Utils::SharedPtr;

class WGContextPool : public IWGContextPool {
public:
  virtual WGContextBase *GetWGContext(bool bBelongsToMasterThread) {
    return m_pool.Malloc();
  }

  virtual void ReleaseWorkerWGContext(WGContextBase *wgContext) {
    m_pool.Free(wgContext);
  }

private:
  Intel::OpenCL::Utils::ObjectPool<WGContextBase> m_pool;
};

/**
 * This class represents a tester object for TaskExecutor module.
 * Currently just testing of TBBTaskExecutor is implemented.
 */
class TaskExecutorTester : public ITaskExecutorObserver {
public:
  TaskExecutorTester() {
    m_config = nullptr;
    if (NULL == m_pTaskExecutor) {
      m_pTaskExecutor = Intel::OpenCL::TaskExecutor::GetTaskExecutor();
      m_config = new Intel::OpenCL::Utils::BasicCLConfigWrapper();
      m_config->Initialize(GetConfigFilePath());
      size_t additionalStackSize = m_config->GetForcedLocalMemSize() +
                                   m_config->GetForcedPrivateMemSize();
      m_pTaskExecutor->Init(TE_AUTO_THREADS, NULL, additionalStackSize);
    }
  }

  ~TaskExecutorTester() {
    if (m_config)
      delete m_config;
  }

  ITaskExecutor *GetTaskExecutor() { return m_pTaskExecutor; }

  // ITaskExecutorObserver
  virtual void *OnThreadEntry(bool /*registerThread*/) {
    WGContextBase *pCtx =
        m_wgContextPool.GetWGContext(m_pTaskExecutor->IsMaster());
    pCtx->SetThreadId(m_pTaskExecutor->GetPosition());
    return pCtx;
  }

  virtual void OnThreadExit(void *currentThreadData) {
    WGContextBase *pCtx = static_cast<WGContextBase *>(currentThreadData);
    pCtx->SetThreadId(-1);
    m_wgContextPool.ReleaseWorkerWGContext(pCtx);
  }

  TE_BOOLEAN_ANSWER MayThreadLeaveDevice(void *currentThreadData) {
    return TE_USE_DEFAULT;
  }

private:
  Intel::OpenCL::Utils::BasicCLConfigWrapper *m_config;
  static ITaskExecutor *m_pTaskExecutor;
  WGContextPool m_wgContextPool;
};

class TesterTaskSet : public ITaskSet {
public:
  PREPARE_SHARED_PTR(TesterTaskSet)

  static SharedPtr<TesterTaskSet> Allocate(unsigned int uiNumDims,
                                           AtomicCounter *pUncompletedTasks) {
    return SharedPtr<TesterTaskSet>(
        new TesterTaskSet(uiNumDims, pUncompletedTasks));
  }

  // overriden methods:

  virtual bool CompleteAndCheckSyncPoint() override { return false; }

  virtual bool SetAsSyncPoint() override { return false; }

  virtual bool IsCompleted() const override { return m_bIsComplete; }

  virtual long Release() override { return 0; }

  virtual void Cancel() override {}

  virtual int Init(size_t region[], unsigned int &regCount,
                   size_t numberOfThreads) override {
    for (unsigned int i = 0; i < m_uiNumDims; i++) {
      region[i] = 1;
    }
    regCount = m_uiNumDims;
    return 0;
  }

  virtual void *AttachToThread(void *pWgContext, size_t uiNumberOfWorkGroups,
                               size_t firstWGID[], size_t lastWGID[]) override {
    return pWgContext;
  }

  virtual void DetachFromThread(void *pWgContext) override {}

  virtual bool ExecuteIteration(size_t x, size_t y, size_t z,
                                void *pWgContext = NULL) override {
    return true;
  }

  virtual bool Finish(FINISH_REASON reason) override {
    m_bIsComplete = true;
    if (NULL != m_pUncompletedTasks) {
      (*m_pUncompletedTasks)--;
    }
    return true;
  }

  virtual Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return NULL;
  }

  // Optimize By
  TASK_PRIORITY GetPriority() const override { return TASK_PRIORITY_MEDIUM; }
  TASK_SET_OPTIMIZATION OptimizeBy() const override {
    return TASK_SET_OPTIMIZE_DEFAULT;
  }
  size_t PreferredSequentialItemsPerThread() const override { return 1; }
  bool PreferNumaNodes() const override { return false; }

private:
  TesterTaskSet(unsigned int uiNumDims, AtomicCounter *pUncompletedTasks)
      : m_bIsComplete(false), m_uiNumDims(uiNumDims),
        m_pUncompletedTasks(pUncompletedTasks) {}

  volatile bool m_bIsComplete;
  const unsigned int m_uiNumDims;
  AtomicCounter *const m_pUncompletedTasks;
};
