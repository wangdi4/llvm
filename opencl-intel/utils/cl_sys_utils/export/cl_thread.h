// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __CL_THREAD_H__
#define __CL_THREAD_H__

#include "cl_synch_objects.h"
#include "cl_sys_defines.h"

#include <atomic>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define STDCALL_ENTRY_POINT __stdcall
#define RETURN_TYPE_ENTRY_POINT unsigned int
#define THREAD_HANDLE HANDLE
#else
#include <pthread.h>
#define STDCALL_ENTRY_POINT
#define RETURN_TYPE_ENTRY_POINT void *
#define THREAD_HANDLE pthread_t
#endif

namespace Intel {
namespace OpenCL {
namespace Utils {

/************************************************************************
 * OclThread is an abstract thread class.
 * To implement a thread in the system, inherit this class and implement the
 * Run function.
 * This class provides basic thread managing methods such as: start, join,
 *terminate Set bAutoDelete to true only for dynamicaly created objects and
 *expected to be terminated by themself
 ************************************************************************/

class OclThread {
public:
  OclThread(std::string name = "", bool bAutoDelete = false);
  virtual ~OclThread();

  OclThread(const OclThread &) = delete;
  OclThread &operator=(const OclThread &) = delete;

  virtual int Start();
  virtual int Join();
  virtual int WaitForCompletion();
  void Terminate(RETURN_TYPE_ENTRY_POINT exitCode);
  int SetAffinity(unsigned char ucAffinity);
  bool IsRunning() const { return m_running; }
  unsigned int GetThreadId() const { return m_threadId; }
  THREAD_HANDLE GetThreadHandle() const;
  void Clean();

  enum eThreadResult { THREAD_RESULT_SUCCESS = 0, THREAD_RESULT_FAIL = -1 };

  static bool IsOsThreadRunning(THREAD_HANDLE handle);
  static void WaitForOsThreadCompletion(THREAD_HANDLE handle);

protected:
  virtual RETURN_TYPE_ENTRY_POINT Run() = 0; // The actual thread running loop.
  void Exit(RETURN_TYPE_ENTRY_POINT exitCode);
  static void SelfTerminate(RETURN_TYPE_ENTRY_POINT exitCode);
  bool isSelf();

  static RETURN_TYPE_ENTRY_POINT STDCALL_ENTRY_POINT
  ThreadEntryPoint(void *threadObject);

  void *m_threadHandle;
  unsigned int m_threadId;
  bool m_running;
  std::atomic<long> m_join{0};
  std::atomic<long> m_numWaiters{0};
  bool m_bAutoDelete;
  std::string m_Name;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
#endif // __CL_THREAD_H__
