#if INTEL_COLLAB
//===--- VPOParoptModuleTranform.h - Paropt Module Transforms ---*- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the ParOpt interface to perform module transformations
/// for OpenMP and Auto-parallelization.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H
#define LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"

#include <functional>

namespace llvm {

class Module;
class Function;
class Constant;
class Instruction;

namespace vpo {

/// Provide all functionalities to perform paropt threadization such as
/// outlining, privatization, loop partitioning, multithreaded code
/// generation for a Module.
class VPOParoptModuleTransform {

public:
  /// ParoptModuleTransform object constructor
  VPOParoptModuleTransform(Module &M, int Mode, unsigned OptLevel = 2,
                           bool SwitchToOffload = false)
    : M(M), C(M.getContext()), Mode(Mode), OptLevel(OptLevel),
      SwitchToOffload(SwitchToOffload),
      TgOffloadEntryTy(nullptr), TgDeviceImageTy(nullptr),
      TgBinaryDescriptorTy(nullptr), DsoHandle(nullptr)
  {}

  /// Perform paropt transformation on a module.
  bool doParoptTransforms(
      std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter);

private:
  friend class VPOParoptTransform;

  /// Register outlined target region. This call returns region ID.
  Constant* registerTargetRegion(WRegionNode *W, Constant *Func);

private:
  /// Module which is being transformed.
  Module &M;

  /// Module's context (shortcut for M.getContext()).
  LLVMContext &C;

  /// Paropt compilation mode.
  int Mode;

  /// Optimization level.
  unsigned OptLevel;

  /// Offload compilation mode.
  bool SwitchToOffload;

  /// A list of device triples for offload compilation.
  SmallVector<Triple, 16> TgtDeviceTriples;

  /// Hold the struct type as follows.
  ///    struct __tgt_offload_entry {
  ///      void      *addr;       // The address of a global variable
  ///                             // or entry point in the host.
  ///      char      *name;       // Name of the symbol referring to the
  ///                             // global variable or entry point.
  ///      size_t     size;       // Size in bytes of the global variable or
  ///                             // zero if it is entry point.
  ///      int32_t    flags;      // Flags of the entry.
  ///      int32_t    reserved;   // Reserved by the runtime library.
  /// };
  StructType *TgOffloadEntryTy;

  /// Hold the struct type as follows.
  /// struct __tgt_device_image{
  ///   void   *ImageStart;       // The address of the beginning of the
  ///                             // target code.
  ///   void   *ImageEnd;         // The address of the end of the target
  ///                             // code.
  ///   __tgt_offload_entry  *EntriesBegin;  // The first element of an array
  ///                                        // containing the globals and
  ///                                        // target entry points.
  ///   __tgt_offload_entry  *EntriesEnd;    // The last element of an array
  ///                                        // containing the globals and
  ///                                        // target entry points.
  /// };
  StructType *TgDeviceImageTy;

  /// Hold the struct type as follows.
  /// struct __tgt_bin_desc{
  ///   uint32_t              NumDevices;     // Number of device types i
  ///                                         // supported.
  ///   __tgt_device_image   *DeviceImages;   // A pointer to an array of
  ///                                         // NumDevices elements.
  ///   __tgt_offload_entry  *EntriesBegin;   // The first element of an array
  ///                                         // containing the globals and
  ///                                         // target entry points.
  ///   __tgt_offload_entry  *EntriesEnd;     // The last element of an array
  ///                                         // containing the globals and
  ///                                         // target entry points.
  /// };
  StructType *TgBinaryDescriptorTy;

  /// Create a variable that binds the atexit to this shared object.
  GlobalVariable *DsoHandle;

  /// Information about outlined target regions.
  SmallVector<std::pair<Constant*, Constant*>, 8u> TargetRegions;

  /// Return true if the program is compiled at the offload mode.
  bool hasOffloadCompilation() const {
    return ((Mode & OmpOffload) || SwitchToOffload);
  }

  /// Remove routines and global variables which has no target declare
  /// attribute.
  void removeTargetUndeclaredGlobals();

  /// Transform the use of the tid global into __kmpc_global_thread_num
  /// or the the use of the first argument of the OMP outlined function. The use
  /// of bid global is transformed accordingly.
  void fixTidAndBidGlobals();

  /// The utility to transform the tid/bid global variable.
  void processUsesOfGlobals(Constant *PtrHolder,
                            SmallVectorImpl<Instruction *> &RewriteIns,
                            bool IsTid);

  /// Collect the uses of the given global variable.
  void collectUsesOfGlobals(Constant *PtrHolder,
                            SmallVectorImpl<Instruction *> &RewriteIns);

  /// Process the device information into the triples.
  void processDeviceTriples();

  /// Generate offload entry table.
  void genOffloadEntries();

  /// Register the offloading binary descriptors.
  void genOffloadingBinaryDescriptorRegistration();

  /// Return/Create the struct type __tgt_offload_entry.
  StructType *getTgOffloadEntryTy();

  /// Return/Create the struct type __tgt_device_image.
  StructType *getTgDeviceImageTy();

  /// Return/Create the struct type __tgt_bin_desc.
  StructType *getTgBinaryDescriptorTy();

  /// Return/Create a variable that binds the atexit to this shared
  /// object.
  GlobalVariable *getDsoHandle();

  /// Create the function .omp_offloading.descriptor_reg
  Function *createTgDescRegisterLib(Function *TgDescUnregFn,
                                    GlobalVariable *Desc);

  /// Create the function .omp_offloading.descriptor_unreg
  Function *createTgDescUnregisterLib(GlobalVariable *Desc);
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H
#endif // INTEL_COLLAB
