// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef __I_MEMORY_OBJECT_DESC_H__
#define __I_MEMORY_OBJECT_DESC_H__

namespace Validation {
class IMemoryObjectDesc {
public:
  virtual ~IMemoryObjectDesc() {
  } // This interface is passed to a template which call it's destructor. Need
    // virtual destructor in place.
  /// @brief is NEAT object
  virtual bool IsNEAT() const = 0;
  /// @brief Set NEAT property
  virtual void SetNeat(const bool in_IsNeat) = 0;
  /// @brief create clone object
  virtual IMemoryObjectDesc *Clone() const = 0;
  /// @brief get Name of class
  virtual std::string GetName() const = 0;
  /// @brief computes object size.
  virtual size_t GetSizeInBytes() const = 0;
};

// helper class to track IMemoryObjectDesc * objects
template <class T> class IMemoryObjectDescWrapper {
  /// pointer to object with IMemoryObjectDesc interface
  T *m_p;

public:
  IMemoryObjectDescWrapper() : m_p(NULL) {}
  IMemoryObjectDescWrapper(const IMemoryObjectDescWrapper &r) {
    m_p = (r.m_p) ? r.m_p->Clone() : NULL;
  }
  IMemoryObjectDescWrapper(const T &pr) { m_p = pr.Clone(); }
  virtual ~IMemoryObjectDescWrapper() {
    if (m_p)
      delete m_p;
  }
  IMemoryObjectDescWrapper &operator=(const IMemoryObjectDescWrapper &r) {
    if (m_p)
      delete m_p;
    m_p = (r.m_p) ? r.m_p->Clone() : NULL;
    return *this;
  }
  IMemoryObjectDescWrapper &operator=(const T *pr) {
    if (m_p)
      delete m_p;
    m_p = (pr) ? pr->Clone() : NULL;
    return *this;
  }
  T *operator->() { return m_p; }

  T *get() const { // return wrapped pointer
    return (m_p);
  }

  bool operator<(const IMemoryObjectDescWrapper &foo1) const {
    return m_p < foo1.m_p;
  }

  bool operator>(const IMemoryObjectDescWrapper &foo1) const {
    return m_p > foo1.m_p;
  }
};

typedef IMemoryObjectDescWrapper<IMemoryObjectDesc> IMemoryObjectDescPtr;

} // namespace Validation
#endif // __I_MEMORY_OBJECT_DESC_H__
