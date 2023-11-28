// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef __REF_COUNT_THREAD_SAFE_H__
#define __REF_COUNT_THREAD_SAFE_H__

#include "Atomics.h"
#include <assert.h>

namespace intel {
template <typename T>

/// Threadsafe Refernce counter pointer
/// Uses atomic instruction
/// Must LINK with Atomics.cpp object file
class RefCountThreadSafe {
public:
  // Forge
  RefCountThreadSafe() : m_refCount(0), m_ptr(0) {}

  explicit RefCountThreadSafe(T *ptr) : m_refCount(0), m_ptr(ptr) {
    if (ptr)
      m_refCount = new atomics::atomic_type(1);
  }

  RefCountThreadSafe(const RefCountThreadSafe<T> &other) { cpy(other); }

  ~RefCountThreadSafe() { dispose(); }

  RefCountThreadSafe &operator=(const RefCountThreadSafe<T> &other) {
    if (this == &other)
      return *this;
    dispose();
    cpy(other);
    return *this;
  }

  bool isNull() const { return (!m_ptr); }

  const T &operator*() const {
    sanity();
    return *m_ptr;
  }

  T &operator*() {
    sanity();
    return *m_ptr;
  }

  T *operator->() {
    sanity();
    return m_ptr;
  }

  const T *operator->() const {
    sanity();
    return m_ptr;
  }

  const T *get() const { return m_ptr; }

  T *get() { return m_ptr; }

private:
  void sanity() const {
    assert(m_ptr && "NULL pointer");
    assert(m_refCount && "NULL ref counter");
    assert(*m_refCount && "zero ref counter");
  }

  void cpy(const RefCountThreadSafe<T> &other) {
    m_refCount = other.m_refCount;
    m_ptr = other.m_ptr;
    if (m_refCount)
      atomics::AtomicIncrement(m_refCount);
  }

  void dispose() {
    if (!m_refCount)
      return;
    sanity();
    if (0 == atomics::AtomicDecrement(m_refCount)) {
      delete m_refCount;
      delete m_ptr;
      m_ptr = 0;
      m_refCount = 0;
    }
  }

  atomics::atomic_type *m_refCount;
  T *m_ptr;
}; // End RefCount

} // namespace intel

#endif //__REF_COUNT_THREAD_SAFE_H__
