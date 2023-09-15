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
#include <iostream>
#include <mutex>
#include <set>
#include <string>

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This represents a base class for all objects in the OpenCL runtime that can
 * add dependency to another object. This super-class holds two multi-sets: a
 * dependency set and a reverse dependency set. This enables to maintains a
 * dependency graph for each object, which can be printed at any moment. In the
 * destructor OCLObjectBase asserts that both the dependency and the reverse
 * dependency sets are empty, otherwise it means that there is a memory leak or
 * a possible wild pointer.
 */

class OCLObjectBase {
  const std::string m_typename;
  std::mutex m_muAcquireRelease;
  std::multiset<const OCLObjectBase *> m_dependencySet;
  std::multiset<const OCLObjectBase *> m_reverseDependencySet;

  void PrintDependencyGraphRecursive(std::ostream &,
                                     unsigned int uiIndent) const;

protected:
  /**
   * Constructor
   *
   * @param typeName the name of the concrete object's type
   */
  explicit OCLObjectBase(const std::string &typeName) : m_typename(typeName) {}

  /**
   * Insert an object to the dependency set - this object is dependent on the
   * parameter object.
   *
   * @param pObj an object to insert to the dependency set
   */
  void InsertToDependencySet(OCLObjectBase *pObj);

  /**
   * Erase an object from the dependency set.
   *
   * @param pObj an object to erase from the dependency set
   */
  void EraseFromDependecySet(OCLObjectBase *pObj);

  /**
   * @return the type name of this OCLObjectBase
   */
  const std::string &GetTypeName() const { return m_typename; }

public:
  /**
   * destructor
   */
  virtual ~OCLObjectBase();

  /**
   * Print the objects dependency graph
   *
   * @param os an ostream to print the graph on
   */
  void PrintDependencyGraph(std::ostream &os);
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
