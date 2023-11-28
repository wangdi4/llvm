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

#ifndef WORK_GROUP_STORAGE_H
#define WORK_GROUP_STORAGE_H

#include "IWorkItemBuiltins.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/DerivedTypes.h"
#include <vector>

namespace Validation {

/// helper class for storing work-group related stuff for passing between
/// Open CL Reference runner and Reference WorkGroup built-ins
class WorkGroupStorage : public OCLBuiltins::IWorkGroupBuiltins {
public:
  /// ctor
  WorkGroupStorage() : m_RefCounter(0), m_LockedBy(std::string()) {}

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins. specific function for work_group_all.
  /// work_group_any builtins
  ///@return reference to llvm::GenericValue
  virtual llvm::GenericValue &GetValueForWorkGroupAllAnyBuiltin() override {
    return m_Predicate;
  }

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins. specific function for work_group_broadcast
  /// builtin
  ///@return reference to llvm::GenericValue
  virtual llvm::GenericValue &GetValueForBroadcastBuiltin() override {
    return m_BroadcastedValue;
  }

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins. specific function for work_group_reduce builtin
  ///@return reference to llvm::GenericValue
  virtual llvm::GenericValue &GetValueForReduceBuiltin() override {
    return m_ReduceValue;
  }

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins. specific function for work_group_scan builtin
  ///@return reference to llvm::GenericValue
  virtual llvm::GenericValue &GetValueForScanBuiltin() override {
    return m_ScanValue;
  }

  /// Report that someone is referencing to instance of class
  ///@param [in] string lock  built-in who is referencing to instance
  ///@param [in] GenericValue init initialize with this value on very first run
  virtual void AddRef(const std::string &lock,
                      const llvm::GenericValue init) override {
    if (m_LockedBy != std::string() && m_RefCounter)
      assert(lock == m_LockedBy && "Two different work group builtins \
                                     are executing in the same time");
    if (!m_RefCounter)
      initializeWithValue(init);

    m_LockedBy = lock;
    ++m_RefCounter;
  }

  /// Report that reference to instance of supreclass is no longer needed.
  /// reinitializes object with zero referenced objects
  virtual void DecRef() override { --m_RefCounter; }

private:
  /// initialize accumulators with given value
  ///@param [in] GenericValue init
  void initializeWithValue(const llvm::GenericValue init) {
    m_Predicate = init;
    m_BroadcastedValue = init;
    m_ReduceValue = init;
    m_ScanValue = init;
  }

  /// accumulative variable stored within work group
  /// used by work_group_any(int) and work_group_all(int)
  llvm::GenericValue m_Predicate;
  /// accumulative variable stored within work group
  /// used by broadcast work group built-ins
  llvm::GenericValue m_BroadcastedValue;
  /// accumulative variable stored within work group
  /// used by reduce work group built-in
  llvm::GenericValue m_ReduceValue;
  /// accumulative variable stored within work group
  /// used by scan work group built-ins
  llvm::GenericValue m_ScanValue;
  /// reference counter
  /// how many work-items is executing work-group builtin in the same time
  int32_t m_RefCounter;
  /// name of built-in executing by work-items
  std::string m_LockedBy;
};

} // namespace Validation
#endif // WORK_GROUP_STORAGE_H
