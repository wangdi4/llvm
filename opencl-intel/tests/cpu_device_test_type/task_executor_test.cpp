// Copyright (c) 2006 Intel Corporation
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

#include "cl_device_api.h"
#include "cl_object_pool.h"
#include "cl_shared_ptr.hpp"
#include "cl_synch_objects.h"
#include "cpu_dev_test.h"
#include "task_executor.h"

#include <atomic>
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <sys/syscall.h>
#endif

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

static SharedPtr<ITEDevice> g_pRootDevice = NULL;

// #define EXTENDED_PRINT
//  OCL Kernel execution
class TestSet : public ITaskSet {
  int m_id;
  volatile int *m_pDone;
  std::atomic<long> m_attached{0};

public:
  PREPARE_SHARED_PTR(TestSet)

  static SharedPtr<TestSet> Allocate(int id, volatile int *pDone) {
    return SharedPtr<TestSet>(new TestSet(id, pDone));
  }

  bool SetAsSyncPoint() override { return false; }

  bool CompleteAndCheckSyncPoint() override { return false; }

  bool IsCompleted() const override { return false; }

  void Cancel() override {}

  Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return NULL;
  }

  // ITaskSet interface
  int Init(size_t region[], unsigned int &regCount,
           size_t numberOfThreads) override {
    *m_pDone = 0;
#ifdef EXTENDED_PRINT
    printf("TestSet::Init() - %d - Init on %d\n", m_id, GET_THREAD_ID);
#endif
    regCount = 1;
    region[0] = 1000;
    return 0;
  }
  void *AttachToThread(void *pWgContext, size_t uiNumberOfWorkGroups,
                       size_t firstWGID[], size_t lastWGID[]) override {
    m_attached++;
#ifdef EXTENDED_PRINT
    printf("TestSet::AttachToThread() - %d - %d was joined as %d, attached: "
           "%d\n",
           m_id, GET_THREAD_ID, uiWorkerId, long(m_attached));
#endif
    return (void *)1; // return non-NULL
  }
  void DetachFromThread(void *pWgContext) override {
    m_attached--;
#ifdef EXTENDED_PRINT
    printf("TestSet::DetachFromThread() - %d - %d left execution, attached: "
           "%d\n",
           m_id, uiWorkerId, long(m_attached));
#endif
    return;
  }
  bool ExecuteIteration(size_t x, size_t y, size_t z,
                        void *pWgContext) override {
    // printf("TestSet::ExecuteIteration() - %d executing\n", uiWorkerId);
    for (unsigned int i = 0; i < x; ++i) {
      SLEEP(0);
    }
    return true;
  }
  bool Finish(FINISH_REASON reason) override {
#ifdef EXTENDED_PRINT
    printf("TestSet::Finish() - %d - Finished on %d\n", m_id, GET_THREAD_ID);
#endif
    assert(0 == m_attached && "m_attached != 0");
    *m_pDone = 1;
    return true;
  }
  long Release() override {
    delete this;
    return 0;
  }
  // Optimize By
  TASK_PRIORITY GetPriority() const override { return TASK_PRIORITY_MEDIUM; }
  TASK_SET_OPTIMIZATION OptimizeBy() const override {
    return TASK_SET_OPTIMIZE_DEFAULT;
  }
  size_t PreferredSequentialItemsPerThread() const override { return 1; }
  bool PreferNumaNodes() const override { return false; }

private:
  TestSet(int id, volatile int *pDone) : m_id(id), m_pDone(pDone) {}
};

RETURN_TYPE_ENTRY_POINT STDCALL_ENTRY_POINT MasterThread(void *pParam) {
  SharedPtr<ITaskList> pList =
      g_pRootDevice->CreateTaskList(TE_CMD_LIST_IN_ORDER);

  volatile int done = 0;
  pList->Enqueue(SharedPtr<ITaskBase>(TestSet::Allocate(1, &done)));
  pList->Flush();
  te_wait_result res = pList->WaitForCompletion(NULL);
  while ((TE_WAIT_MASTER_THREAD_BLOCKING == res) && !done) {
    SLEEP(100);
    res = pList->WaitForCompletion(NULL);
  }

  return 0;
}

bool test_task_executor() {
  printf("test_task_executor - Start test\n");

  for (int i = 0; i < 100; ++i) {
    ITaskExecutor *pTaskExecutor = GetTaskExecutor();
    g_pRootDevice = pTaskExecutor->CreateRootDevice(RootDeviceCreationParam());
    SharedPtr<ITaskList> pList =
        g_pRootDevice->CreateTaskList(TE_CMD_LIST_IN_ORDER);
#if 0
#ifdef _WIN32
    HANDLE hMaster = (HANDLE)_beginthreadex(NULL, 0, &MasterThread, GetTaskExecutor(), 0, NULL);
#else
    pthread_t* hMaster = new pthread_t;
    int err = pthread_create(hMaster, NULL, &MasterThread, GetTaskExecutor());
#endif
#endif
    volatile int done = 0;

    pList->Enqueue(SharedPtr<ITaskBase>(TestSet::Allocate(0, &done)));
    pList->Flush();
    te_wait_result res = pList->WaitForCompletion(NULL);
    while ((TE_WAIT_MASTER_THREAD_BLOCKING == res) && !done) {
      SLEEP(100);
      res = pList->WaitForCompletion(NULL);
    }

#if 0
#ifdef _WIN32
    WaitForSingleObject(hMaster, INFINITE);
    CloseHandle(hMaster);
#else
    pthread_join(*hMaster, NULL);
    delete(hMaster);
#endif
#endif

    g_pRootDevice = NULL;
    printf(".");
    fflush(stdout);
    if (((i % 10) == 0) && (0 != i)) {
      printf("\nEnd iteration = %d, passed\n", i);
    }
  }
  printf("test_task_executor - End test\n");
  return true;
}
