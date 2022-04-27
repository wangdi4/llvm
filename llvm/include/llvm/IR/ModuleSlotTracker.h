//===-- llvm/IR/ModuleSlotTracker.h -----------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_MODULESLOTTRACKER_H
#define LLVM_IR_MODULESLOTTRACKER_H

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace llvm {

class Module;
class Function;
class SlotTracker;
class Value;
class MDNode;

/// Abstract interface of slot tracker storage.
class AbstractSlotTrackerStorage {
public:
  virtual ~AbstractSlotTrackerStorage();

  virtual unsigned getNextMetadataSlot() = 0;

  virtual void createMetadataSlot(const MDNode *) = 0;
  virtual int getMetadataSlot(const MDNode *) = 0;
};

/// Manage lifetime of a slot tracker for printing IR.
///
/// Wrapper around the \a SlotTracker used internally by \a AsmWriter.  This
/// class allows callers to share the cost of incorporating the metadata in a
/// module or a function.
///
/// If the IR changes from underneath \a ModuleSlotTracker, strings like
/// "<badref>" will be printed, or, worse, the wrong slots entirely.
class ModuleSlotTracker {
  /// Storage for a slot tracker.
  std::unique_ptr<SlotTracker> MachineStorage;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
  bool ShouldCreateStorage = false;
  bool ShouldInitializeAllMetadata = false;
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

  const Module *M = nullptr;
  const Function *F = nullptr;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
  SlotTracker *Machine = nullptr;
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

  std::function<void(AbstractSlotTrackerStorage *, const Module *, bool)>
      ProcessModuleHookFn;
  std::function<void(AbstractSlotTrackerStorage *, const Function *, bool)>
      ProcessFunctionHookFn;

public:
  /// Wrap a preinitialized SlotTracker.
  ModuleSlotTracker(SlotTracker &Machine, const Module *M,
                    const Function *F = nullptr);

  /// Construct a slot tracker from a module.
  ///
  /// If \a M is \c nullptr, uses a null slot tracker.  Otherwise, initializes
  /// a slot tracker, and initializes all metadata slots.  \c
  /// ShouldInitializeAllMetadata defaults to true because this is expected to
  /// be shared between multiple callers, and otherwise MDNode references will
  /// not match up.
  explicit ModuleSlotTracker(const Module *M,
                             bool ShouldInitializeAllMetadata = true);

  /// Destructor to clean up storage.
  virtual ~ModuleSlotTracker();

  /// Lazily creates a slot tracker.
  SlotTracker *getMachine();

  const Module *getModule() const { return M; }
  const Function *getCurrentFunction() const { return F; }

  /// Incorporate the given function.
  ///
  /// Purge the currently incorporated function and incorporate \c F.  If \c F
  /// is currently incorporated, this is a no-op.
  void incorporateFunction(const Function &F);

  /// Return the slot number of the specified local value.
  ///
  /// A function that defines this value should be incorporated prior to calling
  /// this method.
  /// Return -1 if the value is not in the function's SlotTracker.
  int getLocalSlot(const Value *V);

  void setProcessHook(
      std::function<void(AbstractSlotTrackerStorage *, const Module *, bool)>);
  void setProcessHook(std::function<void(AbstractSlotTrackerStorage *,
                                         const Function *, bool)>);

  using MachineMDNodeListType =
      std::vector<std::pair<unsigned, const MDNode *>>;

  void collectMDNodes(MachineMDNodeListType &L, unsigned LB, unsigned UB) const;
};

} // end namespace llvm

#endif
