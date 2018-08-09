// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __REF_COUNT_H__
#define __REF_COUNT_H__

#include <assert.h>

namespace intel {

template <typename T> class RefCount {
public:
  // Forge
  RefCount() : m_refCount(0), m_ptr(0) {}

  RefCount(T *ptr) : m_ptr(ptr) { m_refCount = new int(1); }

  RefCount(const RefCount<T> &other) { cpy(other); }

  ~RefCount() {
    if (m_refCount)
      dispose();
  }

  RefCount &operator=(const RefCount<T> &other) {
    if (this == &other)
      return *this;
    if (m_refCount)
      dispose();
    cpy(other);
    return *this;
  }

  void init(T *ptr) {
    assert(!m_ptr && "overrunning non NULL pointer");
    assert(!m_refCount && "overrunning non NULL pointer");
    m_refCount = new int(1);
    m_ptr = ptr;
  }

  bool isNull() const { return (!m_ptr); }

  // Pointer access
  const T &operator*() const {
    sanity();
    return *m_ptr;
  }

  T &operator*() {
    sanity();
    return *m_ptr;
  }

  operator T *() { return m_ptr; }

  operator const T *() const { return m_ptr; }

  T *operator->() { return m_ptr; }

  const T *operator->() const { return m_ptr; }

private:
  void sanity() const {
    assert(m_ptr && "NULL pointer");
    assert(m_refCount && "NULL ref counter");
    assert(*m_refCount && "zero ref counter");
  }

  void cpy(const RefCount<T> &other) {
    m_refCount = other.m_refCount;
    m_ptr = other.m_ptr;
    if (m_refCount)
      ++*m_refCount;
  }

  void dispose() {
    sanity();
    if (0 == --*m_refCount) {
      delete m_refCount;
      delete m_ptr;
      m_ptr = 0;
      m_refCount = 0;
    }
  }

  int *m_refCount;
  T *m_ptr;
}; // End RefCount

} // end namepace

#endif//__REF_COUNT_H__
