#include "WorkerThread.h"
#include <iostream>

#if !defined(_WIN32)
#include <pthread.h>

WorkerThread::WorkerThread() : m_thread(NULL) {}

WorkerThread::~WorkerThread() { stop(); }

bool WorkerThread::start(WorkerThreadFunc threadFunction, void *threadData) {
  pthread_t thread;
  int error = pthread_create(&thread, NULL, threadFunction, threadData);

  if (error) {
    return false;
  }

  m_thread = (void *)thread;
  return true;
}

void WorkerThread::stop() {
  if (m_thread) {
    pthread_join((pthread_t)m_thread, NULL);
    m_thread = NULL;
  }
}
#else
#include <Windows.h>
#include <process.h>

WorkerThread::WorkerThread() : m_thread(NULL) {}

WorkerThread::~WorkerThread() { stop(); }

bool WorkerThread::start(WorkerThreadFunc threadFunction, void *threadData) {
  m_thread = (void *)_beginthread(threadFunction, 0, threadData);
  if (!m_thread) {
    return false;
  }

  return true;
}

void WorkerThread::stop() {
  if (m_thread) {
    WaitForSingleObject(m_thread, INFINITE);
    m_thread = NULL;
  }
}
#endif
