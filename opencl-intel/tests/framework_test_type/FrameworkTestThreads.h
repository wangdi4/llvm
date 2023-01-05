#pragma once

#include "FrameworkTest.h"

typedef RETURN_TYPE_ENTRY_POINT(STDCALL *ThreadEntryPoint)(void *);

class Thread {
public:
  Thread() {}
  virtual ~Thread() {}

  bool Run(ThreadEntryPoint func);
  virtual void WaitForCompletion();

  virtual void ThreadRoutine() = 0;

protected:
#ifdef _WIN32
  void *m_handle;
#else
  pthread_t m_pthread;
#endif
};

class SynchronizedThread : public Thread {
public:
  SynchronizedThread() {}
  virtual ~SynchronizedThread() {}

  bool Run();

  static void Signal() { m_canStart = true; }

protected:
  static RETURN_TYPE_ENTRY_POINT STDCALL SynchronizedThreadEntryPoint(void *p);
  static volatile bool m_canStart;
};

class SynchronizedThreadPool {
public:
  SynchronizedThreadPool() : m_init(false), m_threadStarted(NULL) {}
  virtual ~SynchronizedThreadPool() {
    if (NULL != m_threadStarted)
      delete[] m_threadStarted;
  }

  bool Init(SynchronizedThread **pThreads, size_t numThreads);

  void StartAll();
  void WaitAll();

protected:
  bool m_init;
  SynchronizedThread **m_threads;
  bool *m_threadStarted;
  size_t m_numThreads;
};
