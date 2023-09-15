// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#pragma once

#include "cl_synch_objects.h"
#include "cl_thread.h"
#include <atomic>

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

class TEDevice;

template <class Data> class TBB_ThreadManager;

template <class Data> struct TBB_ThreadDescriptor {
public:
  Data m_data;

  friend class TBB_ThreadManager<Data>;
};

//
// Manage per-thread data for all threads as they enter any TEDevice.
// Implements simple single-linked free list with partial static preallocation
// for performance
//
template <class Data> class TBB_ThreadManager {
public:
  TBB_ThreadManager();
  ~TBB_ThreadManager();

  TBB_ThreadManager(const TBB_ThreadManager &o) = delete;
  TBB_ThreadManager &operator=(const TBB_ThreadManager &o) = delete;

  // return false on error
  bool Init(unsigned int uiNumberOfThreadsToOptimize);

  // allocate new thread entry
  Data *RegisterCurrentThread();
  void UnregisterCurrentThread();

  static Data *GetCurrentThreadDescriptor() {
    TBB_ThreadDescriptor<Data> *cached = m_CurrentThreadGlobalID;
    return (nullptr != cached) ? &(cached->m_data) : nullptr;
  }

  // register thread of still integistered
  Data *RegisterAndGetCurrentThreadDescriptor() {
    Data *d = GetCurrentThreadDescriptor();
    return (nullptr != d) ? d : RegisterCurrentThread();
  }

private:
  TBB_ThreadDescriptor<Data> *m_DescriptorsArray;

  static THREAD_LOCAL TBB_ThreadDescriptor<Data> *m_CurrentThreadGlobalID;

  unsigned int m_uiNumberOfStaticEntries;
  std::atomic<long> m_nextFreeEntry{0};
  volatile bool m_bOverflowed;
  static bool m_object_exists;
};

template <class Data>
THREAD_LOCAL TBB_ThreadDescriptor<Data>
    *TBB_ThreadManager<Data>::m_CurrentThreadGlobalID = nullptr;
template <class Data> bool TBB_ThreadManager<Data>::m_object_exists = false;

#include "tbb_thread_manager.hpp"

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
