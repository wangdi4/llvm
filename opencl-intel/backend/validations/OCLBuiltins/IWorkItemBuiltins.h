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

#ifndef I_WORK_ITEM_BUILTINS
#define I_WORK_ITEM_BUILTINS

#include "llvm/IR/DerivedTypes.h"
#include <cassert>

namespace Validation {
namespace OCLBuiltins {

/// Interface for setting/getting work-group built-in variables
class IWorkGroupBuiltins {
public:
  /// Report that someone is referencing to instance of class
  /// if object locked by someone for first time - initialize it
  ///@param [in] string lock  built-in who is referencing to instance
  ///@param [in] GenericValue init initialize with this value on very first run
  virtual void AddRef(const std::string &lock,
                      const llvm::GenericValue init) = 0;

  /// Report that reference to instance of supreclass is no longer needed.
  virtual void DecRef() = 0;

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins work_group_any, work_group_all
  ///@return reference to llvm::GenericValue accumulative value for
  /// work_group_all and work_group_all built-ins
  virtual llvm::GenericValue &GetValueForWorkGroupAllAnyBuiltin() = 0;

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins work_group_broadcast
  ///@return reference to llvm::GenericValue accumulative value for
  /// work_group_broadcast builtins
  virtual llvm::GenericValue &GetValueForBroadcastBuiltin() = 0;

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins work_group_reduce_<op>
  ///@return reference to llvm::GenericValue accumulative value for
  /// work_group_reduce_<op> built-ins
  virtual llvm::GenericValue &GetValueForReduceBuiltin() = 0;

  /// Get reference to accumulative value. Needed to collect data
  /// for work-group built-ins work_group_scan_<inclusive/exclusive>_<op>
  ///@return reference to llvm::GenericValue accumulative value for
  /// work_group_scan built-ins
  virtual llvm::GenericValue &GetValueForScanBuiltin() = 0;
  virtual ~IWorkGroupBuiltins() {}
};

/// Interface for setting/getting work-item built-in variables
/// This interface is intended to pass work-item variables between OpenCL
/// ReferenceRunner layer and work-item built-in implementation in OpenCL
/// Reference. OpenCL 1.1 sec 6.11.1
class IWorkItemBuiltins {
public:
  /// Get number of dimensions in use for get_work_dim() built-in
  /// @return uint32_t type
  virtual uint32_t GetWorkDim() = 0;

  /// Get global size for get_global_size() built-in
  /// @param [in]     dimindx dimension number
  virtual uint64_t GetGlobalSize(const uint32_t dimindx) = 0;

  /// Get local size for get_local_size() built-in
  /// @param [in]     dimindx dimension number
  virtual uint64_t GetLocalSize(const uint32_t dimindx) = 0;

  /// Get global ID without global offset (!!!)  for get_global_id() built-in
  /// global offset should be added inside built-in implementation
  /// @param [in]     dimindx dimension number
  virtual uint64_t GetGlobalIdNoOffset(const uint32_t dimindx) = 0;

  /// Get local ID for get_local_id() built-in
  /// @param [in]     dimindx dimension number
  virtual uint64_t GetLocalId(const uint32_t dimindx) = 0;

  /// Get enqueued local ID for get_enqueued_local_size() built-in
  /// @param [in]     dimindx dimension number
  virtual uint64_t getEnqueuedLocalSize(const uint32_t dimindx) = 0;

  /// Get global offset for get_global_offset() built-in
  /// this offset will be added to global ID
  /// @param [in]     dimindx dimension number
  virtual uint64_t GetGlobalOffset(const uint32_t dimindx) = 0;

  virtual ~IWorkItemBuiltins() {}
};

/// singleton class to set interface for workitem built-ins to obtain workgroup
/// IDs
class WorkItemInterfaceSetter {
public:
  WorkItemInterfaceSetter(WorkItemInterfaceSetter const &) = delete;
  WorkItemInterfaceSetter &operator=(WorkItemInterfaceSetter const &) = delete;

  /// obtain pointer to object
  static WorkItemInterfaceSetter *inst() {
    if (!m_pInst)
      m_pInst = new WorkItemInterfaceSetter;
    return m_pInst;
  }

  /// Set interface to work-items
  void SetWorkItemInterface(IWorkItemBuiltins *p) {
    assert(p);
    m_pWorkItemBuiltins = p;
  }

  /// Set interface to work-group
  void SetWorkGroupInterface(IWorkGroupBuiltins *p) {
    assert(p);
    m_pWorkGroupBuiltins = p;
  }

  /// Get interface to work-items
  IWorkItemBuiltins *GetWorkItemInterface() {
    assert(m_pWorkItemBuiltins);
    return m_pWorkItemBuiltins;
  }

  /// Get interface to work-group
  IWorkGroupBuiltins *GetWorkGroupInterface() {
    assert(m_pWorkGroupBuiltins);
    return m_pWorkGroupBuiltins;
  }

protected:
  /// hide ctor and dtor
  WorkItemInterfaceSetter()
      : m_pWorkItemBuiltins(NULL), m_pWorkGroupBuiltins(NULL){};
  ~WorkItemInterfaceSetter(){};

private:
  /// static pointer to instance
  static WorkItemInterfaceSetter *m_pInst;
  /// workgroup interface
  IWorkItemBuiltins *m_pWorkItemBuiltins;
  IWorkGroupBuiltins *m_pWorkGroupBuiltins;
};

} // namespace OCLBuiltins
} // namespace Validation

#endif // I_WORK_ITEM_BUILTINS
