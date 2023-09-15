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

#include "cl_shared_ptr.hpp"

#include <stdio.h>

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace Utils {

#ifdef _DEBUG
ManagedStatic<std::mutex> AllocatedObjectsMapMutex;
ManagedStatic<std::map<std::string, AllocatedObjectsMapTy>> AllocatedObjectsMap;
#endif

void ReferenceCountedObject::IncZombieCnt() const {
  std::lock_guard<std::mutex> lock(m_zombieLock);
  ++m_zombieLevelCnt;
  m_bCheckZombie = true;
}

long ReferenceCountedObject::DriveEnterZombieState() const {
  long new_val;
  bool trigger_action = false;

  // do everything inside a lock
  // in the most common case each DecRefCount operation for objects with
  // lifetime control will require 3 lock operations:
  //  1. Acquire m_zombieLock
  //  2. Decrement atomic counter
  //  3. Release m_zombieLock
  {
    std::lock_guard<std::mutex> lock(m_zombieLock);
    new_val =
        --m_refCnt; // this cannot result in object deletion as done inside lock

    if ((new_val == m_zombieLevelCnt) && (ZOMBIE != m_state)) {
      ++m_refCnt; // ensure object will not disapper during callbacks
      m_state = ZOMBIE;
      trigger_action = true;
    }
  }

  if (true == trigger_action) {
    // call callbacks autside of lonk to avoid deadlocks
    const_cast<ReferenceCountedObject *>(this)->EnterZombieState(
        TOP_LEVEL_CALL);
#ifdef _DEBUG
    assert((true == m_bEnterZombieStateCalled) &&
           "EnterZombieState() must be propagated up to the parent");
#endif
    // now decrement counter inside the same lock as above to avoid object
    // deappearence inside the above critical section in another thread
    std::lock_guard<std::mutex> lock(m_zombieLock);
    new_val = --m_refCnt;
  }

  return new_val;
}

#ifdef _DEBUG

static void DumpSharedPtsHeader(const char *map_title) {
  printf("\nBEGIN SHARED POINTERS MAP: %s\n",
         (nullptr != map_title) ? map_title : "");
}

static void DumpSharedPtsFooter(const char *map_title) {
  printf("\nEND SHARED POINTERS MAP: %s\n",
         (nullptr != map_title) ? map_title : "");
  fflush(0);
}

void DumpSharedPts(const char *map_title, bool if_non_empty) {
  bool header_printed = false;

  std::lock_guard<std::mutex> guard(*AllocatedObjectsMapMutex);

  if (!if_non_empty) {
    header_printed = true;
    DumpSharedPtsHeader(map_title);
  }

  for (auto name_it = AllocatedObjectsMap->begin(),
            name_it_end = AllocatedObjectsMap->end();
       name_it != name_it_end; ++name_it) {
    AllocatedObjectsMapTy &internal_map = name_it->second;
    if (internal_map.size() > 0) {
      if (!header_printed) {
        header_printed = true;
        DumpSharedPtsHeader(map_title);
      }
      printf("\n%s:\n", name_it->first.c_str());
      for (auto it = internal_map.begin(), e = internal_map.end(); it != e;
           ++it) {
        printf("\t%p  %ld\n", it->first, it->second);
      }
    }
  }

  if (header_printed) {
    DumpSharedPtsFooter(map_title);
  }
}

#endif

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
