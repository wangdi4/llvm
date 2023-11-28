#include "FrameworkTestThreads.h"

volatile bool SynchronizedThread::m_canStart = false;

RETURN_TYPE_ENTRY_POINT STDCALL
SynchronizedThread::SynchronizedThreadEntryPoint(void *p) {
  SynchronizedThread *pThread = reinterpret_cast<SynchronizedThread *>(p);
  while (!m_canStart)
    ; // busy-wait
  pThread->ThreadRoutine();
  return 0;
}

bool SynchronizedThread::Run() {
  return Thread::Run(SynchronizedThreadEntryPoint);
}

bool SynchronizedThreadPool::Init(SynchronizedThread **pThreads,
                                  size_t numThreads) {
  m_numThreads = numThreads;
  m_threads = pThreads;
  if (m_numThreads > 0) {
    m_threadStarted = new bool[m_numThreads];
    memset(m_threadStarted, false, sizeof(bool) * m_numThreads);
  }
  m_init =
      ((m_threads != 0) && (m_numThreads > 0) && (m_threadStarted != NULL));

  return m_init;
}

void SynchronizedThreadPool::StartAll() {
  if (!m_init) {
    return;
  }
  for (size_t t = 0; t < m_numThreads; ++t) {
    m_threadStarted[t] = m_threads[t]->Run();
  }
  // Start all threads
  SynchronizedThread::Signal();
}

void SynchronizedThreadPool::WaitAll() {
  if (!m_init) {
    return;
  }
  for (size_t t = 0; t < m_numThreads; ++t) {
    if (m_threadStarted[t]) {
      m_threads[t]->WaitForCompletion();
    }
  }
}
