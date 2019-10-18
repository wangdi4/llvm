#if INTEL_COLLAB
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
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
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

/// Provide all functionalities to perform paropt threadization such as
/// outlining, privatization, loop partitioning, multithreaded code
/// generation for a Module.
class VPOParoptModuleTransform {

public:
  /// ParoptModuleTransform object constructor
  VPOParoptModuleTransform(Module &M, int Mode, unsigned OptLevel = 2,
                           bool SwitchToOffload = false,
                           bool DisableOffload = false)
      : M(M), C(M.getContext()), Mode(Mode),
#if INTEL_CUSTOMIZATION
        ORVerbosity(OptReportVerbosity::Low),
#endif  // INTEL_CUSTOMIZATION
        OptLevel(OptLevel), SwitchToOffload(SwitchToOffload),
        DisableOffload(DisableOffload), TgOffloadEntryTy(nullptr),
        TgDeviceImageTy(nullptr), TgBinaryDescriptorTy(nullptr),
        DsoHandle(nullptr), PrintfDecl(nullptr), OCLPrintfDecl(nullptr) {}

  ~VPOParoptModuleTransform() {
    DeleteContainerPointers(OffloadEntries);
  }

  /// Perform paropt transformation on a module.
  bool doParoptTransforms(
      std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter,
      std::function<TargetLibraryInfo &(Function &F)> TLIGetter);

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

  /// Offload compilation mode.
  bool SwitchToOffload;

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

  /// Original declaration of printf() from clang:
  ///   declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...)
  /// This is populated during OpenCL offload compilation if printf() is used
  /// in the module
  Function *PrintfDecl;
  Function *getPrintfDecl() { return PrintfDecl; }

  /// Declaration of OCL's printf() created for OpenCL offload kernel code:
  ///   declare dso_local spir_func i32
  ///     @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(1)*, ...)
  Function *OCLPrintfDecl;
  Function *getOCLPrintfDecl() { return OCLPrintfDecl; }

  /// Routine to populate PrintfDecl and OCLPrintfDecl
  void createOCLPrintfDecl(Function *F);

  /// Routine to identify Functions that may use "omp critical"
  /// either directly or down the call stack.
  void collectMayHaveOMPCriticalFunctions(
      std::function<TargetLibraryInfo &(Function &F)> TLIGetter);

  /// A set of Functions identified by collectMayHaveOMPCriticalFunctions()
  /// to potentially "invoke" "omp critical".
  SmallPtrSet<Function *, 32> MayHaveOMPCritical;

  /// Returns true for Functions marked by collectMayHaveOMPCriticalFunctions().
  bool mayHaveOMPCritical(const Function *F) const;

  /// Base class for offload entries. It is not supposed to be instantiated.
  class OffloadEntry {
  public:
    enum EntryKind : unsigned {
      RegionKind = 0,
      VarKind    = 1u
    };

  protected:
    explicit OffloadEntry(EntryKind Kind, StringRef Name, uint32_t Flags)
      : Kind(Kind), Name(Name), Flags(Flags)
    {}

  public:
    virtual ~OffloadEntry() = default;

    EntryKind getKind() const { return Kind; }

    StringRef getName() const { return Name; }

    Constant *getAddress() const { return Addr; }

    void setAddress(Constant *NewAddr) {
      assert(!Addr && "Address has been set before!");
      Addr = NewAddr;
    }

    uint32_t getFlags() const { return Flags; }
    void setFlags(uint32_t NewFlags) { Flags = NewFlags; }

    virtual size_t getSize() const = 0;

  private:
    EntryKind Kind;
    SmallString<64u> Name;
    Constant* Addr = nullptr;
    uint32_t Flags = 0;
  };

  /// Target region entry.
  class RegionEntry final : public OffloadEntry {
  public:
    enum : uint32_t {
      Region = 0x00,
      Ctor   = 0x02,
      Dtor   = 0x04
    };

  public:
    RegionEntry(StringRef Name, uint32_t Flags)
      : OffloadEntry(RegionKind, Name, Flags) {}

    RegionEntry(GlobalValue *GV, uint32_t Flags)
      : OffloadEntry(RegionKind, GV->getName(), Flags) {
      setAddress(GV);
    }

    size_t getSize() const override { return 0; }

    static bool classof(const OffloadEntry *E) {
      return E->getKind() == RegionKind;
    }
  };

  /// Global variable entry.
  class VarEntry final : public OffloadEntry {
  public:
    enum : uint32_t {
      DeclareTargetTo = 0x00,
      DeclareTargetLink = 0x01
    };

  public:
    explicit VarEntry(GlobalVariable *Var, uint32_t Flags)
      : OffloadEntry(VarKind, Var->getName(), Flags) {
      setAddress(Var);
    }

    size_t getSize() const override {
      const auto *Var = cast<GlobalVariable>(getAddress());
      auto *VarType = Var->getType()->getPointerElementType();
      // Global variables have pointer type always.
      // For the purpose of the size calculation, we have to
      // get the pointee's type.
      return Var->getParent()->getDataLayout().getTypeAllocSize(VarType);
    }

    bool isDeclaration() const {
      const auto *Var = cast<GlobalVariable>(getAddress());
      return Var->isDeclaration();
    }

    static bool classof(const OffloadEntry *E) {
      return E->getKind() == VarKind;
    }
  };

  /// Offload entries.
  SmallVector<OffloadEntry*, 8u> OffloadEntries;

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

  /// Load offload metadata from the module and create offload entries that
  /// need to be emitted after lowering all target constructs.
  void loadOffloadMetadata();

  /// Generate offload entry table. It should be done after completing all
  /// transformations.  Returns true, if the table was generated,
  /// false - otherwise.
  bool genOffloadEntries();

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
