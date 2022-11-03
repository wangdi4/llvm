// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include <stack>

using Intel::OpenCL::Utils::OclAutoMutex;

namespace Intel {
namespace OpenCL {
namespace Utils {

/**
 * This class is responsible for managing a pool of objects that can quickly be
 * allocated and freed.
 * @param ElemType type of the objects managed by the pool
 */
template <typename ElemType> class ObjectPool {
public:
  /**
   * Destructor
   */
  ~ObjectPool() { Clear(); }

  /**
   * Allocate an object from the pool. If the pool is not empty, the complexity
   * of this operation is O(1).
   * @return a pointer to the allocated object
   */
  ElemType *Malloc();

  /**
   * Free an object back to the pool. The complexity of this operation is O(1).
   * @param pObj the object to be freed
   */
  void Free(ElemType *pObj);

  /**
   * Clear and delete all the objects in the pool
   */
  void Clear();

private:
  std::stack<ElemType *> m_stack;
  Intel::OpenCL::Utils::OclSpinMutex m_mutex;
};

template <typename ElemType> void ObjectPool<ElemType>::Clear() {
  while (!m_stack.empty()) {
    delete m_stack.top();
    m_stack.pop();
  }
}

template <typename ElemType> ElemType *ObjectPool<ElemType>::Malloc() {
  m_mutex.Lock();
  if (m_stack.empty()) {
    m_mutex
        .Unlock(); // let the heavy new operator be outside the critical section
    return new ElemType();
  } else {
    ElemType *const pElem = m_stack.top();
    assert(nullptr != pElem);
    m_stack.pop();
    m_mutex.Unlock();
    return pElem;
  }
}

template <typename ElemType> void ObjectPool<ElemType>::Free(ElemType *pObj) {
  assert(nullptr != pObj);
  OclAutoMutex autoMutex(&m_mutex);
  m_stack.push(pObj);
}

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
