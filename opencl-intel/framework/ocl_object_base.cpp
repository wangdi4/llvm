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

#include "ocl_object_base.h"
#include <cassert>

namespace Intel {
namespace OpenCL {
namespace Framework {

#ifdef _DEBUG
// #define DEBUG_DEPENDENCY
#endif

void OCLObjectBase::PrintDependencyGraphRecursive(std::ostream &os,
                                                  unsigned int uiIndent) const {
  for (std::multiset<const OCLObjectBase *>::const_iterator iter =
           m_dependencySet.begin();
       iter != m_dependencySet.end(); iter++) {
    for (unsigned int i = 0; i < uiIndent; i++) {
      os << '\t';
    }
    const OCLObjectBase *const pObj = *iter;
    os << pObj->m_typename << " " << *iter << std::endl;
    pObj->PrintDependencyGraphRecursive(os, uiIndent + 1);
  }
}

void OCLObjectBase::PrintDependencyGraph(std::ostream &os) {
  m_muAcquireRelease.lock();
  // Save ostream flags
  std::ios::fmtflags f(os.flags());
  os << std::hex;
  os << "Dependency graph for " << m_typename << " " << this << ":"
     << std::endl;
  PrintDependencyGraphRecursive(os, 1);
  // Restore ostream flags
  os.flags(f);
  m_muAcquireRelease.unlock();
}

void OCLObjectBase::InsertToDependencySet(OCLObjectBase * /*pObj*/) {
#if defined DEBUG_DEPENDENCY
  m_muAcquireRelease.lock();
  m_dependencySet.insert(pObj);
  pObj->m_reverseDependencySet.insert(this);
  m_muAcquireRelease.unlock();
#endif
}

void OCLObjectBase::EraseFromDependecySet(OCLObjectBase * /*pObj*/) {
#if defined DEBUG_DEPENDENCY
  m_muAcquireRelease.lock();
  m_dependencySet.erase(
      m_dependencySet.find(pObj)); // remove only one occurrence
  pObj->m_reverseDependencySet.erase(pObj->m_reverseDependencySet.find(this));
  m_muAcquireRelease.unlock();
#endif
}

OCLObjectBase::~OCLObjectBase() {
#if defined DEBUG_DEPENDENCY
  assert(0 == m_dependencySet.size());
  assert(0 == m_reverseDependencySet.size());
#endif
}

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
