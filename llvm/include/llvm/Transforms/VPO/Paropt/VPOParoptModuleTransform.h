#if INTEL_COLLAB
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
//===--- VPOParoptModuleTranform.h - Paropt Module Transforms ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

#include <functional>

namespace llvm {

class Module;
class Function;
class Constant;
class Instruction;

namespace vpo {

extern std::unordered_map<std::string, std::string> OCLBuiltin;

/// Address space value for offload entries on SPIR-V target.
extern unsigned SpirvOffloadEntryAddSpace;

/// Provide all functionalities to perform paropt threadization such as
/// outlining, privatization, loop partitioning, multithreaded code
/// generation for a Module.
class VPOParoptModuleTransform {

public:
  /// ParoptModuleTransform object constructor
  VPOParoptModuleTransform(Module &M, int Mode, unsigned OptLevel = 2,
                           bool DisableOffload = false)
      : M(M), C(M.getContext()), Mode(Mode),
#if INTEL_CUSTOMIZATION
        ORVerbosity(OptReportVerbosity::Low),
#endif  // INTEL_CUSTOMIZATION
        OptLevel(OptLevel),
        DisableOffload(DisableOffload), TgtOffloadEntryTy(nullptr),
        TgDeviceImageTy(nullptr), TgBinaryDescriptorTy(nullptr),
        DsoHandle(nullptr), PrintfDecl(nullptr), OCLPrintfDecl(nullptr) {}

  ~VPOParoptModuleTransform() {
    for (auto E : OffloadEntries)
      delete E;
    OffloadEntries.clear();
  }

  /// Perform paropt transformation on a module.
  bool doParoptTransforms(
      std::function<vpo::WRegionInfo &(Function &F, bool *Changed)>
          WRegionInfoGetter);

private:
  friend class VPOParoptTransform;

  /// Register outlined target region. This call returns region ID.
  Constant* registerTargetRegion(WRegionNode *W, Constant *Func);

  /// Returns offload device triples.
  const SmallVectorImpl<Triple>& getDeviceTriples() const {
    return TgtDeviceTriples;
  }

private:
  /// Module which is being transformed.
  Module &M;

  /// Module's context (shortcut for M.getContext()).
  LLVMContext &C;

  /// Paropt compilation mode.
  int Mode;

#if INTEL_CUSTOMIZATION
  /// Verbosity level for generating remarks using Loop Opt Report
  /// framework (under -qopt-report).
  OptReportVerbosity::Level ORVerbosity;
#endif  // INTEL_CUSTOMIZATION

  /// Optimization level.
  unsigned OptLevel;

  /// Ignore TARGET constructs
  bool DisableOffload;

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
  StructType *TgtOffloadEntryTy;

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

  /// Original declaration of printf() from clang:
  ///   declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...)
  /// This is populated during OpenCL offload compilation if printf() is used
  /// in the module
  Function *PrintfDecl;
  Function *getPrintfDecl() { return PrintfDecl; }

  /// Declaration of OCL's printf() created for OpenCL offload kernel code:
  ///   declare dso_local spir_func i32
  ///     @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)*, ...)
  Function *OCLPrintfDecl;
  Function *getOCLPrintfDecl() { return OCLPrintfDecl; }

  /// Routine to populate PrintfDecl and OCLPrintfDecl
  void createOCLPrintfDecl(Function *F);

  /// Replaces calls to sincos/sincosf with _Z6sincosdPd/_Z6sincosfPf
  void replaceSincosWithOCLBuiltin(Function *F, bool IsDouble);

  /// Clones functions that are both "declare target" and contain "target"
  /// region(s). If F is the original function, then the method clones it
  /// into NewF and does the following:
  ///   1. Removes all target directives from F.
  ///     (including calling \p DummyBranchDeleter on F to delete of branches
  ///      from begin to end directives, added by addBranchToEndDirective()).
  ///   2. Resets "contains-openmp-target" attrbiute for F.
  ///   3. Resets "openmp-target-declare" attribute for NewF.
  bool cloneDeclareTargetFunctions(
      std::function<bool(Function *F)> DummyBranchDeleter) const;

  /// Offload entries.
  SmallVector<OffloadEntry *, 8> OffloadEntries;

  /// Return true if the program is compiled at the offload mode.
  bool hasOffloadCompilation() const {
    return ((Mode & OmpOffload) || VPOParoptUtils::isForcedTargetCompilation());
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

  /// Generate offload entry table. It should be done after completing all
  /// transformations.  Returns true, if the table was generated,
  /// false - otherwise.
  bool genOffloadEntries();

  /// Return/Create the struct type __tgt_offload_entry.
  StructType *getTgtOffloadEntryTy();
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H
#endif // INTEL_COLLAB
