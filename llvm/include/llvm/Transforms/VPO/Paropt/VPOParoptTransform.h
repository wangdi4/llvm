#if INTEL_COLLAB // -*- C++ -*-
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
//===-- VPO/Paropt/VPOParoptTranform.h - Paropt Transform Class -*- C++ -*-===//
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
/// This file contains the interface to outline a work region formed from
/// parallel loop/regions/tasks into a new function, replacing it with a
/// call to the threading runtime call by passing new function pointer to
/// the runtime for parallel execution.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORMS_H
#define LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORMS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Pass.h"

#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"

#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace llvm {

namespace vpo {

extern cl::opt<bool> UseMapperAPI;

/// Device architectures supported
enum DeviceArch : uint64_t {
  DeviceArch_None   = 0,
  DeviceArch_Gen9   = 0x0001,
  DeviceArch_XeLP   = 0x0002, // DG1
  DeviceArch_XeHP   = 0x0004, // ATS
  DeviceArch_x86_64 = 0x0100  // Internal use: OpenCL CPU offloading
};

enum TgtOffloadMappingFlags : uint64_t {
  TGT_MAP_TO = 0x01,
  // instructs the runtime to copy the host data to the device.
  TGT_MAP_FROM = 0x02,
  // instructs the runtime to copy the device data to the host.
  TGT_MAP_ALWAYS = 0x04,
  // forces the copying regardless of the reference
  // count associated with the map.
  TGT_MAP_DELETE = 0x08,
  // forces the unmapping of the object in a target data.
  TGT_MAP_PTR_AND_OBJ = 0x10,
  // forces the runtime to map the pointer variable as
  // well as the pointee variable.
  TGT_MAP_TARGET_PARAM = 0x20,
  // instructs the runtime that it is the first
  // occurrence of this mapped variable within this construct.
  TGT_MAP_RETURN_PARAM = 0x40,
  // instructs the runtime to return the base
  // device address of the mapped variable.
  TGT_MAP_PRIVATE = 0x80,
  // informs the runtime that the variable is a private variable.
  TGT_MAP_LITERAL = 0x100,
  // instructs the runtime to forward the value to target construct.
  TGT_MAP_IMPLICIT = 0x200,
  TGT_MAP_CLOSE = 0x400,
  // The close map-type-modifier is a hint to the runtime to
  // allocate memory close to the target device.
  TGT_MAP_ND_DESC = 0x800,
  // indicates that the parameter is loop descriptor struct.
  TGT_MAP_MEMBER_OF = 0xffff000000000000
};

typedef SmallVector<WRegionNode *, 32> WRegionListTy;
typedef std::unordered_map<const BasicBlock *, WRegionNode *> BBToWRNMapTy;
typedef std::pair<Type*, Value*> ElementTypeAndNumElements;

class VPOParoptModuleTransform;

/// Provide all functionalities to perform paropt threadization
/// such as outlining, privatization, loop partitioning, multithreaded
/// code generation.
class VPOParoptTransform {

public:
  /// ParoptTransform object constructor
  VPOParoptTransform(VPOParoptModuleTransform *MT,
                     Function *F, WRegionInfo *WI, DominatorTree *DT,
                     LoopInfo *LI, ScalarEvolution *SE,
                     const TargetTransformInfo *TTI, AssumptionCache *AC,
                     const TargetLibraryInfo *TLI, AAResults *AA, int Mode,
#if INTEL_CUSTOMIZATION
                     OptReportVerbosity::Level ORVerbosity,
#endif  // INTEL_CUSTOMIZATION
                     OptimizationRemarkEmitter &ORE,
                     unsigned OptLevel = 2,
                     bool DisableOffload = false)
      : MT(MT), F(F), WI(WI), DT(DT), LI(LI), SE(SE), TTI(TTI), AC(AC),
        TLI(TLI), AA(AA), Mode(Mode),
        TargetTriple(F->getParent()->getTargetTriple()),
#if INTEL_CUSTOMIZATION
        ORVerbosity(ORVerbosity),
#endif  // INTEL_CUSTOMIZATION
        ORE(ORE), OptLevel(OptLevel),
        DisableOffload(DisableOffload),
        IdentTy(nullptr), TidPtrHolder(nullptr), BidPtrHolder(nullptr),
        KmpcMicroTaskTy(nullptr), KmpRoutineEntryPtrTy(nullptr),
        KmpTaskTTy(nullptr), KmpTaskTRedTy(nullptr),
        KmpTaskDependInfoTy(nullptr) {

#if INTEL_CUSTOMIZATION
        // Set up Builder for generating remarks using Opt Report
        // framework (under -qopt-report).
        ORBuilder.setup(F->getContext(), ORVerbosity);
#endif  // INTEL_CUSTOMIZATION
      }

  /// Add a temporary branch from \p W's EntryBB to ExitBB. This is to prevent
  /// optimizations from deleting the end region directive of a WRegion if it
  /// is determined to be unreachable (for instance if there is a call to
  /// `exit()` within the WRegion.
  /// The generated code looks like:
  ///
  /// \code
  ///   %temp = alloca i1
  ///   %0 = llvm.region.entry()[... "QUAL.OMP.JUMP.TO.END.IF" (i1* %temp)
  ///   %temp.load = load volatile i1, i1* %temp
  ///   %cmp = icmp ne i1 %temp.load, false
  ///   br i1 %cmp, label %REGION.END, label %REGION.BODY
  ///
  ///   REGION.BODY:
  ///   ...
  ///
  ///   REGION.END:
  ///   llvm.region.exit(%0)
  ///
  /// \endcode
  ///
  /// This branch is added at the end of paropt-prepare pass, and later removed
  /// before the vpo-paropt transformation.
  /// \returns \b true if a branch was added, \b false otherwise.
  bool addBranchToEndDirective(WRegionNode *W);

  /// Top level interface for parallel and prepare transformation
  bool paroptTransforms();

  bool addNormUBsToParents(WRegionNode* W);

  bool isModeOmpNoFECollapse() { return Mode & vpo::OmpNoFECollapse; }
  bool isModeOmpSimt() { return Mode & vpo::OmpSimt; }

  PointerType *getDefaultPointerType() {
        assert(F && "Function cannot be null.");
        const Module *M = F->getParent();
        assert(M && "Function is not in a module?");
        unsigned AS = M ? WRegionUtils::getDefaultAS(M) : 0;
        return PointerType::get(M->getContext(), AS);
  }

#if INTEL_CUSTOMIZATION
  /// Interfaces for data sharing optimization.
  bool optimizeDataSharingForPrivateItems(
      BBToWRNMapTy &BBToWRNMap, int &NumOptimizedItems);
  bool optimizeDataSharingForReductionItems(
      BBToWRNMapTy &BBToWRNMap, int &NumOptimizedItems);
  /// Create a map between the BasicBlocks and the corresponding
  /// innermost WRegionNodes owning the blocks.
  void initializeBlocksToRegionsMap(BBToWRNMapTy &BBToWRNMap);
  /// Privatize shared items in the work region.
  bool privatizeSharedItems();
#endif  // INTEL_CUSTOMIZATION

private:
  /// A reference to the parent module transform object. It can be NULL if
  /// paropt transform is construted from a function pass.
  VPOParoptModuleTransform *MT;

  /// The W-regions in the function F are to be transformed
  Function *F;

  /// W-Region information holder
  WRegionInfo *WI;

  /// Get the Dominator Tree for code extractor
  DominatorTree *DT;

  /// Get the Loop information for loop candidates
  LoopInfo *LI;

  /// Get the Scalar Evolution information for loop candidates
  ScalarEvolution *SE;

  /// Get the Target Tranform information for loop candidates.
  const TargetTransformInfo *TTI;

  /// Get the assumption cache informtion for loop candidates.
  AssumptionCache *AC;

  /// Get the target library information for the loop candidates.
  const TargetLibraryInfo *TLI;

  AAResults *AA;

  /// Paropt compilation mode
  int Mode;

  /// Target triple that we are compiling for.
  Triple TargetTriple;

#if INTEL_CUSTOMIZATION
  /// Verbosity level for generating remarks using Opt Report framework (under
  /// -qopt-report).
  OptReportVerbosity::Level ORVerbosity;

  /// Builder for generating remarks using Opt Report framework (under
  /// -qopt-report).
  OptReportBuilder ORBuilder;
#endif  // INTEL_CUSTOMIZATION

  /// Optimization remark emitter.
  OptimizationRemarkEmitter &ORE;

  /// Optimization level.
  unsigned OptLevel;

  /// Ignore TARGET construct
  bool DisableOffload;

  /// Contain all parallel/sync/offload constructs to be transformed
  WRegionListTy WRegionList;

  /// Hold the LOC structure type which is need for KMP library
  StructType *IdentTy;

  /// Hold the pointer to Tid (thread id) Value
  Constant *TidPtrHolder;

  /// Hold the pointer to Bid (binding thread id) Value
  Constant *BidPtrHolder;

  /// Hold the function type for the function
  /// void (*kmpc_micro)(kmp_int32 *global_tid, kmp_int32 *bound_tid, ...)
  FunctionType *KmpcMicroTaskTy;

  /// Hold the function type for the taskloop outlined function in the
  /// form of void @RoutineEntry (i32 %tid, %struct.kmp_task_t_with_privates*
  /// %taskt.withprivates)
  PointerType *KmpRoutineEntryPtrTy;

  /// Hold the struct type in the form of %struct.kmp_task_t = type {
  /// i8*, i32 (i32, i8*)*, i32, %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t,
  /// i64, i64, i64, i32}
  StructType *KmpTaskTTy;

  /// Hold the struct type in the form of %struct.kmp_task_t_red_item =
  /// type { i8*, i64, i8*, i8*, i8*, i32 }
  StructType *KmpTaskTRedTy;

  /// Hold the struct type as follows.
  ///           struct kmp_depend_info {
  ///              void* arg_addr;
  ///              size_t arg_size;
  ///              char   depend_type;
  ///           };
  StructType *KmpTaskDependInfoTy;

  /// Launder Intrinsics inserted by genGlobalPrivatizationLaunderIntrin().
  SmallDenseMap<WRegionNode *, SmallPtrSet<Value *, 8>>
      LaunderIntrinsicsForRegion;

  struct OmpNumThreadsCallerInfo {
    bool Computed = false;
    bool AddressTaken = false;
    SmallPtrSet<Function *, 16> PotentialCallers;
  };

  /// Information about functions that may call omp_get_num_threads.
  OmpNumThreadsCallerInfo NumThreadsCallerInfo;

  /// Atomic-free reduction global buffers per reduction item.
  DenseMap<ReductionItem *, GlobalVariable *> AtomicFreeRedLocalBufs;
  DenseMap<ReductionItem *, GlobalVariable *> AtomicFreeRedGlobalBufs;

  struct LocalUpdateInfo {
    BasicBlock *UpdateBB = nullptr;
    BasicBlock *ExitBB = nullptr;
    PHINode *IVPhi = nullptr;
    Instruction *LocalId = nullptr;
    Instruction *EntryBarrier = nullptr;
  };
  struct GlobalUpdateInfo {
    BasicBlock *EntryBB = nullptr;
    BasicBlock *UpdateBB = nullptr;
    BasicBlock *ExitBB = nullptr;
    PHINode *IVPhi = nullptr;
    BasicBlock *ScalarUpdateBB = nullptr;
    BasicBlock *LatchBB = nullptr;
  };
  /// BBs that perform updates within the atomic-free reduction loops.
  DenseMap<WRegionNode *, LocalUpdateInfo> AtomicFreeRedLocalUpdateInfos;
  DenseMap<WRegionNode *, GlobalUpdateInfo> AtomicFreeRedGlobalUpdateInfos;
  DenseSet<WRNTargetNode *> UsedLocalTreeReduction;

  /// Struct that keeps all the information needed to pass to
  /// the runtime library.
  class TgDataInfo {
  public:
    /// The array of base pointers passed to the runtime library.
    Value *BaseDataPtrs = nullptr;
    Value *ResBaseDataPtrs;
    /// The array of data pointers passed to the runtime library.
    Value *DataPtrs = nullptr;
    Value *ResDataPtrs;
    /// The array of data sizes passed to the runtime library.
    Value *DataSizes = nullptr;
    Value *ResDataSizes;
    /// The array of data map types passed to the runtime library.
    Value *DataMapTypes = nullptr;
    Value *ResDataMapTypes;
    /// The array of mapper names passed to the runtime library.
    Value *Names = nullptr;
    Value *ResNames;
    /// The array of mapper pointers passed to the runtime library.
    Value *Mappers = nullptr;
    Value *ResMappers;
    bool FoundValidMapper = false;
    /// The number of pointers passed to the runtime library.
    unsigned NumberOfPtrs = 0u;
    explicit TgDataInfo() {}
    void clearArrayInfo() {
      BaseDataPtrs = nullptr;
      DataPtrs = nullptr;
      DataSizes = nullptr;
      DataMapTypes = nullptr;
      Names = nullptr;
      Mappers = nullptr;
      NumberOfPtrs = 0u;
    }
    bool isValid() {
      return BaseDataPtrs && DataPtrs && DataSizes && DataMapTypes &&
             (!UseMapperAPI || Mappers) && NumberOfPtrs;
    }
  };

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  /// Returns true if we are compiling for CSA target.
  bool isTargetCSA() const {
     return TargetTriple.getArch() == Triple::csa;
  }
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  /// Returns true if we are compiling for SPIRV target.
  bool isTargetSPIRV() const {
    return VPOAnalysisUtils::isTargetSPIRV(F->getParent());
  }

  /// Use the WRNVisitor class (in WRegionUtils.h) to walk the
  /// W-Region Graph in DFS order and perform outlining transformation.
  /// \param[out] NeedTID : 'true' if any W visited has W->needsTID()==true
  /// \param[out] NeedBID : 'true' if any W visited has W->needsBID()==true
  void gatherWRegionNodeList(bool &NeedTID, bool &NeedBID);

  enum FunctionKind : int {
    FK_Start = 0,
    FK_Ctor = FK_Start,
    FK_Dtor = 1,
    FK_CopyAssign = 2,
    FK_CopyCtor = 3,
    FK_End = FK_CopyCtor
  };

  /// Copy data from the source address \p To to the destination address
  /// \p From
  /// These must both be pointer types.
  /// A copy constructor or copy-assign function \p Cctor will be used if
  /// given.
  /// \p IsByRef will insert an extra load to dereference the \p From pointer.
  /// If \p NumElements is provided, then it is used as the number of elements.
  /// Otherwise, it will be obtained from \p I.
  void genCopyByAddr(Item *I, Value *To, Value *From, Instruction *InsertPt,
                     Function *Cctor = nullptr, bool IsByRef = false,
                     Value *NumElements = nullptr);

  /// For array constructor/destructor/copy assignment/copy constructor loop,
  /// get the base address of the array, number of elements, and element type.
  /// \param [in] ObjTy Either the array's type or the element type.
  /// \param [in] NumElements Number of elements of \p ObjTy type in the array.
  /// \param [in] BaseAddr Base address of the array.
  /// \param [in] Builder IRBuilder for any new Instructions.
  /// \returns a \b tuple of <ElementType, NumElements, BaseAddress>,
  /// where \p ElementType is the element type of the array,
  /// \p NumElements is the number of elements in the array, and
  /// \p BaseAddress is the base address of the array properly casted.
  ///
  /// \p ObjTy and \p NumElements represent the array configuration,
  /// and there are only two supported configurations:
  ///   1. \p ObjTy is an array type, and \p NumElements is either nullptr
  ///      or ConstantInt value 1.
  ///   2. \p ObjTy is a non-array type.
  std::tuple<Type *, Value *, Value *> genPrivAggregatePtrInfo(
      Type *ObjTy, Value *NumElements, Value *BaseAddr, IRBuilder<> &Builder);

  /// Generate the constructor/destructor/copy assignment/copy constructor call
  /// for a privatized array.
  /// \param [in] Fn Function that needs to be called.
  /// \param [in] FuncKind Kind of the action performed by \p Fn, which
  /// defines the call signature.
  /// \param [in] ObjTy Either the array's type or the element type.
  /// \param [in] NumElements Number of elements of \p ObjTy type in the array.
  /// \param [in] DestVal Base address of the destination array.
  /// \param [in] SrcVal Base address of the source array.
  /// \param [in] InsertPt Insert point for any Instructions to be inserted.
  /// \param [in] DT DominatorTree that needs to be updated, if not nullptr.
  ///
  /// \p ObjTy and \p NumElements represent the array configuration,
  /// and there are only two supported configurations:
  ///   1. \p ObjTy is an array type, and \p NumElements is either nullptr
  ///      or ConstantInt value 1.
  ///   2. \p ObjTy is a non-array type.
  void genPrivAggregateInitOrFini(Function *Fn, FunctionKind FuncKind,
                                  Type *ObjTy, Value *NumElements,
                                  Value *DestVal, Value *SrcVal,
                                  Instruction *InsertPt, DominatorTree *DT);

  /// Generate code for calling constructor/destructor/copy assignment/copy
  /// constructor for privatized variables including scalar and arrays.
  /// If \p NumElements is provided, then it is used as the number of elements.
  /// Otherwise, it will be obtained from \p I.
  void genPrivatizationInitOrFini(Item *I, Function *Fn, FunctionKind FuncKind,
                                  Value *DestVal, Value *SrcVal,
                                  Instruction *InsertPt, DominatorTree *DT,
                                  Value *NumElements = nullptr);

  /// If any item is a VLA or a variable length array section, a stacksave is
  /// inserted at the begining of the region and a restore is inserted at the
  /// end of the region. Note that currently this function is only called for
  /// SIMD constructs.
  bool insertStackSaveRestore(WRegionNode *W);

  /// Loops over every item of every clause. If any item is a VLA or a variable
  /// length array section, it creates an empty entry block and sets the VLA
  /// alloca insertPt to the terminator of this newly created entry block.
#if INTEL_CUSTOMIZATION
  /// If \p OnlyCountReductionF90DVsAsVLAs is \b true, F90_DVs in other clause
  /// items like private, firstprivate, etc., won't be considered for the
  /// purpose of this function.
  bool setInsertionPtForVlaAllocas(WRegionNode *W,
                                   bool OnlyCountReductionF90DVsAsVLAs = true);
#else
  bool setInsertionPtForVlaAllocas(WRegionNode *W);
#endif

  /// returns true if the input item is either a Vla or a Vla Section.
#if INTEL_CUSTOMIZATION
  /// If \p OnlyCountReductionF90DVsAsVLAs is \b true, F90_DVs in other clause
  /// items like private, firstprivate, etc., won't be considered for the
  /// purpose of this function.
  static bool getIsVlaOrVlaSection(Item *I,
                                   bool OnlyCountReductionF90DVsAsVLAs = true);
#else
  static bool getIsVlaOrVlaSection(Item *I);
#endif

  /// Generate code for private variables
  bool genPrivatizationCode(WRegionNode *W,
                            Instruction *CtorInsertPt = nullptr );

  /// Generate code for linear variables.
  ///
  /// The following needs to be done for handling a linear var:
  ///
  /// -# Create two local copies of the linear vars. One to capture the
  /// starting value. Another to be the local linear variable which replaces all
  /// uses of the original inside the region.
  /// -# Capture original value of linear vars before entering the loop.
  /// -# Use the captured value along with the specified step to initialize
  /// the local linear var in each iteration of the loop.
  /// -# At the end of the last loop iteration, copy the value of the local
  /// var back to the original linear var.
  /// If \p OMPLBForLinearClosedForm is provided, the linear closed-form
  /// expression is inserted after it, and it is used instead of loop index
  /// for the closed form computation of the local linear var. Otherwise, the
  /// closed-form computation of the linear var happens in each iteration of the
  /// loop, as part of the loop's body.
  bool genLinearCode(WRegionNode *W, BasicBlock *IfLastIterBB,
                     Instruction *OMPLBForLinearClosedForm = nullptr);

  /// Emit privatization and copyin/copyout code for linear/linear:iv clause
  /// operands on SIMD directives. Initial copyin is generated for "linear"
  /// operands but not for "linear:iv" operands. The final copyout is done for
  /// both "linear" and "linear:iv" operands.
  bool genLinearCodeForVecLoop(WRegionNode *W, BasicBlock *LinearFiniBB);

  /// Add Firstprivate clause for every normalized UB clause variable in a
  /// WRNTaskLoopNode WRegion
  bool addFirstprivateForNormalizedUB(WRegionNode *W);

  /// Generate code for firstprivate variables.
  /// If \p OnlyParoptGeneratedFPForNonPtrCaptures is true, then Only those
  /// Firtstprivate items are handled that were added to capture non-pointers to
  /// be passed into the region.
  /// \see captureAndAddCollectedNonPointerValuesToSharedClause() for details.
  bool genFirstPrivatizationCode(
      WRegionNode *W, bool OnlyParoptGeneratedFPForNonPtrCaptures = false);

  /// Generate code for lastprivate variables
  bool genLastPrivatizationCode(WRegionNode *W, BasicBlock *IfLastIterBB,
                                Instruction *OMPLBForChunk = nullptr,
                                Instruction *BranchToNextChunk = nullptr,
                                Instruction *OMPZtt = nullptr);

  /// Generate destructor calls for [first|last]private variables
  bool genDestructorCode(WRegionNode *W);

  /// Generate an optionally addrspacecast'ed pointer Value for the local copy
  /// of ClauseItem \I for various data-sharing clauses like private,
  /// firstprivate, lastprivate, reduction, linear.
  /// \p NameSuffix is appended at the end of the generated
  /// Instruction's name.
  /// If new instructions need to be generated, they will be inserted
  /// before \p InsertPt.
  /// \p AllocaAddrSpace specifies address space in which the memory
  /// for the privatized variable needs to be allocated. If it is
  /// llvm::None, then the address space matches the default alloca's
  /// address space, as specified by DataLayout. Note that some address
  /// spaces may require allocating the private version of the variable
  /// as a GlobalVariable, not as an AllocaInst.
  /// If \p PreserveAddressSpace is true, then the generated Value
  /// will be addrspacecast'ed to match the addrspace of the \p OrigValue,
  /// otherwise, the generated Value will have the addrspace, as specified
  /// by \p AllocaAddrSpace.
  //  FIXME: get rid of PreserveAddressSpace, when PromoteMemToReg
  //         supports AddrSpaceCastInst.
  Value *genPrivatizationAlloca(
      Item *I, Instruction *InsertPt,
      const Twine &NameSuffix = "",
      llvm::Optional<unsigned> AllocaAddrSpace = llvm::None,
      bool PreserveAddressSpace = true) const;

  /// Returns address space that should be used for privatizing variable
  /// referenced in the [FIRST]PRIVATE clause \p I of the given region \p W.
  /// If the return value is llvm::None, then the address space
  /// should be equal to default alloca address space, as defined
  /// by DataLayout.
  llvm::Optional<unsigned> getPrivatizationAllocaAddrSpace(
      const WRegionNode *W, const Item *I) const;

  /// Replace the variable with the privatized variable.
  /// If \p ExcludeEntryDirective is true, then uses in the entry
  /// directive are not replaced. Default is `false`.
  void genPrivatizationReplacement(WRegionNode *W, Value *PrivValue,
                                   Value *NewPrivInst,
                                   bool ExcludeEntryDirective = false);

  /// For array sections, generate a base + offset GEP corresponding to the
  /// section's starting address. \p Orig is the base of the array section
  /// coming from the frontend, \p ArrSecInfo is the data structure containg the
  /// starting offset, size and stride for various dimensions of the section.
  /// The generated GEP is inserted before \p InsertBefore.
  static Value *
  genBasePlusOffsetGEPForArraySection(Value *Orig,
                                      const ArraySectionInfo &ArrSecInfo,
                                      Instruction *InsertBefore);

  /// \name Reduction Specific Functions
  /// {@

  /// For array [section] reduction init loop, get the base address of
  /// the destination array, number of elements, and destination element type.
  /// \param [in] ReductionItem Reduction Item.
  /// \param [in] AI Local Value for the reduction operand.
  /// \param [in] OldV Original reduction operand Value.
  /// \param [in] InsertPt Insert point for any Instructions to be inserted.
  /// \param [in] Builder IRBuilder using InsertPt for any new Instructions.
  /// \param [out] NumElements Number of elements in the array [section].
  /// \param [out] DestArrayBegin Base address of the local reduction array.
  /// \param [out] DestElementTy Type of each element of the array [section].
  void genAggrReductionInitDstInfo(const ReductionItem &RedI, Value *AI,
                                   Instruction *InsertPt, IRBuilder<> &Builder,
                                   Value *&NumElements, Value *&DestArrayBegin,
                                   Type *&DestElementTy);

  /// For array [section] reduction init (for UDR with non-null initializer) or
  /// finalization loop, compute the base address of the source and destination
  /// arrays, number of elements, and the type of destination array elements.
  /// \param [in] ReductionItem Reduction Item.
  /// \param [in] SrcVal Source Value for the reduction operand.
  /// \param [in] DestVal Destination Value for the reduction operand.
  /// \param [in] InsertPt Insert point for any Instructions to be inserted.
  /// \param [in] Builder IRBuilder using InsertPt for any new Instructions.
  /// \param [out] NumElements Number of elements in the array [section].
  /// \param [out] SrcArrayBegin Base address of the local reduction array.
  /// \param [out] DestArrayBegin Starting address of the original reduction
  /// array [section].
  /// \param [out] DestElementTy Type of each element of the array [section].
  /// \param [in] NoNeedToOffsetOrDerefOldV If true, then that means that \p
  /// OldV has already been pre-processed to include any pointer
  /// dereference/offset, and can be used directly as the destination base
  /// pointer. (default = false)
  void genAggrReductionSrcDstInfo(const ReductionItem &RedI, Value *SrcVal,
                                  Value *DestVal, Instruction *InsertPt,
                                  IRBuilder<> &Builder, Value *&NumElements,
                                  Value *&SrcArrayBegin, Value *&DestArrayBegin,
                                  Type *&DestElementTy,
                                  bool NoNeedToOffsetOrDerefOldV = false);

  /// Initialize `Size`, `ElementType`, `Offset` and `BaseIsPointer` fields for
  /// ArraySectionInfo of the map/reduction item \p CI. It may need to emit some
  /// Instructions, which is done \b before \p InsertPt.
  void computeArraySectionTypeOffsetSize(WRegionNode *W, Item &CI,
                                         Instruction *InsertPt);

  /// Initialize `Size`, `ElementType`, `Offset` and `BaseIsPointer` fields for
  /// ArraySectionInfo \p ArrSecInfo. \p Orig is the base of the array section.
  /// The code emitted is inserted \b before \p InsertPt.
  void computeArraySectionTypeOffsetSize(WRegionNode *W, Value *Orig,
                                         ArraySectionInfo &ArrSecInfo,
                                         bool IsByRef, Instruction *InsertPt);

  /// For all use_device_ptr clauses in \p W, create a Map clause.
  /// If \p InsertBefore is not null, then any instructions genereated
  /// for the map clause are inserted before it, otherwise they are
  /// inserted before \p W's entry BasicBlock
  bool addMapForUseDevicePtr(WRegionNode *W,
                 Instruction *InsertBefore = nullptr);

  // Create maps for private/firstprivate VLAs in W, (if not already present).
  bool addMapForPrivateAndFPVLAs(WRNTargetNode *W);

  bool addFastGlobalRedBufMap(WRegionNode *W);

  // Convert 'IS_DEVICE_PTR' clauses in W to MAP, and 'IS_DEVICE_PTR:PTR_TO_PTR'
  // clauses to MAP + PRIVATE.
  bool addMapAndPrivateForIsDevicePtr(WRegionNode *W);

  /// Add private clauses to \p W for Values in the \p ToPrivatize list.
  /// If a Value in the list has associated ElementTypeAndNumElements
  /// information, a Typed clause will be added, otherwise an untyped clause
  /// will be added. The clauses are added to the region W's private clause list
  /// as well as its entry directive. Returns true if any clause is added, false
  /// otherwise.
  static bool addPrivateClausesToRegion(
      WRegionNode *W,
      ArrayRef<std::pair<Value *, llvm::Optional<ElementTypeAndNumElements>>>
          ToPrivatize);

  /// Update references of use_device_ptr operands in tgt data region to use the
  /// value updated by the tgt_data_init call.
  void useUpdatedUseDevicePtrsInTgtDataRegion(
      WRegionNode *W, Instruction *TgtDataOutlinedFunctionCall);

  /// Return the Value to replace the occurrences of the original clause
  /// operand inside the body of the associated WRegion. It may need to emit
  /// some Instructions, which is done \b before \p InsertPt.
  static Value *getClauseItemReplacementValue(const Item *I,
                                              Instruction *InsertPt);

  /// Return the Value to replace the occurrences of the original Array Section
  /// Reduction operand inside the body of the associated WRegion. It may need
  /// to emit some Instructions, which is done \b before \p InsertPt.
  static Value *
  getArrSecReductionItemReplacementValue(ReductionItem const &RedI,
                                         Instruction *InsertPt);

  /// Generate the reduction initialization code.
  void genReductionInit(WRegionNode *W, ReductionItem *RedI,
                        Instruction *InsertPt, DominatorTree *DT);

  /// Generate the reduction update code.
  /// Returns true iff critical section is required around the generated
  /// reduction update code. If \p NoNeedToOffsetOrDerefOldV is true, then that
  /// means that \p OldV has already been pre-processed to include any pointer
  /// dereference/offset, and can be used directly as the destination base
  /// pointer. (default = false) \p LocalRedVar is a WG-local storage for
  /// the reduction items, contains !nullptr iff atomic-free reduction
  /// uses SLM for the local computations to be able copy its contents to the
  /// global buffer. (default = nullptr)
  bool genReductionFini(WRegionNode *W, ReductionItem *RedI, Value *OldV,
                        Instruction *InsertPt, DominatorTree *DT,
                        bool NoNeedToOffsetOrDerefOldV = false,
                        Value *LocalRedVar = nullptr);

  /// Generate the reduction initialization code for Min/Max.
  Value *genReductionMinMaxInit(ReductionItem *RedI, Type *Ty, bool IsMax);

  /// Generate calling reduction initialization function for user-defined
  /// reduction.
  void genReductionUdrInit(ReductionItem *RedI, Value *ReductionVar,
                           Value *ReductionValueLoc, Type *ScalarTy,
                           IRBuilder<> &Builder);

  /// Generate the inscan reduction initialization.
  Value *genReductionInscanInit(Type *ScalarTy, Value *ReductionVar,
                                IRBuilder<> &Builder);

  /// Generate the reduction intialization instructions.
  Value *genReductionScalarInit(ReductionItem *RedI, Type *ScalarTy);

  /// Generate the reduction code for reduction clause.
  bool genReductionCode(WRegionNode *W);

  // Fast reduction mode: none, tree reduction and atomic.
  // Used for return value of checkFastReduction.
  enum FastReductionMode : int {
    FastReductionNoneMode = 0,
    FastReductionTreeOnlyMode = 1,
    FastReductionAtomicMode = 2
  };

  /// Check if it's array reduction (type of reduction variable is array).
  bool isArrayReduction(ReductionItem *I);

  /// Check if we want to generate fast reduction code (including tree-like
  /// reduction, atomic and etc).
  int checkFastReduction(WRegionNode *W);

  /// Generate the tree-like reduction callback routine
  RDECL genFastRedCallback(WRegionNode *W, StructType *FastRedStructTy);

  /// Create struct type and variable for fast reduction.
  std::pair<StructType *, Value *> genFastRedTyAndVar(WRegionNode *W,
                                                      int FastReduction);

  /// Generate the code to copy local reduction variable to local variable for
  /// fast reduction.
  void genFastRedCopy(ReductionItem *RedI, Value *Dst, Value *Src,
                      Instruction *InsertPt, DominatorTree *DT,
                      bool NoNeedToOffsetOrDerefOldV = false);

  /// Generate copy code for scalar type \p ElemTy.
  /// Copy from \p Src address to \p Dst address.
  /// New instructions are inserted using \p Builder.
  void genFastRedScalarCopy(Value *Dst, Value *Src, Type *ElemTy,
                            IRBuilder<> &Builder);

  /// Generate copy code for aggregate type.
  void genFastRedAggregateCopy(ReductionItem *RedI, Value *Src, Value *Dst,
                               Instruction *InsertPt, DominatorTree *DT,
                               bool NoNeedToOffsetOrDerefOldV = false);

  /// Generate private reduction variable for fast reduction.
  Value *genFastRedPrivateVariable(ReductionItem *RedI, unsigned ItemIndex,
                                   Type *FastRedStructTy, Value *FastRedInst,
                                   Instruction *InsertPt);

  /// Generate reduce blocks for tree and atomic reduction.
  void genFastReduceBB(WRegionNode *W, FastReductionMode Mode,
                       StructType *FastRedStructTy, Value *FastRedVar,
                       BasicBlock *EntryBB, BasicBlock *EndBB);

  /// Generate local update loop for atomic-free GPU reduction
  bool genAtomicFreeReductionLocalFini(WRegionNode *W, ReductionItem *RedI,
                                       LoadInst *Rhs1, LoadInst *Rhs2,
                                       StoreInst *RedStore,
                                       IRBuilder<> &Builder, DominatorTree *DT);

  /// Generate global update loop for atomic-free GPU reduction
  bool genAtomicFreeReductionGlobalFini(
      WRegionNode *W, ReductionItem *RedI, StoreInst *RedStore,
      Value *ReductionValueLoc, Instruction *RedValToLoad, PHINode *RedSumPhi,
      bool UseExistingUpdateLoop, IRBuilder<> &Builder, DominatorTree *DT);

  /// Generate code for the aligned clause.
  bool genAlignedCode(WRegionNode *W);

  /// Generate code for the nontemporal clause.
  bool genNontemporalCode(WRegionNode *W);

  /// For the given region \p W returns a BasicBlock, where
  /// new alloca instructions may be inserted.
  /// If the region itself or one of its ancestors will be outlined,
  /// then the returned block is an immediate successor of the region's
  /// entry directive, otherwise, it is the enclosing Function's entry block.
  /// New alloca instructions must be inserted at the beginning
  /// of the returned block.
  BasicBlock *createAllocaBB(WRegionNode *W) const;

  /// Prepare the empty basic block for the array
  /// reduction or firstprivate initialization.
  BasicBlock *createEmptyPrivInitBB(WRegionNode *W) const;

  /// Return the empty basic block for the array
  /// reduction or lastprivate update.
  /// If \p W is a loop region, and the loop has ZTT check,
  /// then the new block will be inserted at the exit block
  /// of the loop, unless \p HonorZTT is false.  Otherwise,
  /// the new block will be inserted at the region's exit
  /// block
  BasicBlock *createEmptyPrivFiniBB(WRegionNode *W,
                                    bool HonorZTT = true);

  /// Generate the reduction update instructions for min/max.
  Value* genReductionMinMaxFini(ReductionItem *RedI, Value *Rhs1, Value *Rhs2,
                             Type *ScalarTy, IRBuilder<> &Builder, bool IsMax);

  /// Generate calling reduction update function for user-defined reduction.
  bool genReductionUdrFini(ReductionItem *RedI, Value *ReductionVar,
                           Value *ReductionValueLoc, IRBuilder<> &Builder);

  /// Generate the reduction update instructions.
  /// Returns true iff critical section is required around the generated
  /// reduction update code. If \p NoNeedToOffsetOrDerefOldV is true, then that
  /// means that \p RedI has no extra by-ref related loads that may require
  /// special hoisting when atomic-free reduction is used. (default = false)
  bool genReductionScalarFini(WRegionNode *W, ReductionItem *RedI,
                              Value *ReductionVar, Value *ReductionValueLoc,
                              Type *ScalarTy, IRBuilder<> &Builder,
                              DominatorTree *DT,
                              bool NoNeedToOffsetOrDerefOldV = false);

  /// Generate the reduction operator with the given arguments \p Rhs1
  /// and \p Rhs2 and the operator in \p RedI.
  /// Handles both LLVM scalar types and also complex ones.
  /// May emit > 1 instructions depending on the item's type and the
  /// reduction operator.
  Value *genReductionScalarOp(ReductionItem *RedI, IRBuilder<> &Builder,
                              Type *ScalarTy, Value *Rhs1, Value *Rhs2);

  /// Generate the reduction initialization/update for array.
  /// Returns true iff critical section is required around the generated
  /// reduction update code. The method always returns false, when
  /// IsInit is true. If \p NoNeedToOffsetOrDerefOldV is true, then that means
  /// that \p OldV has already been pre-processed to include any pointer
  /// dereference/offset, and can be used directly as the destination base
  /// pointer. (default = false)
  bool genRedAggregateInitOrFini(WRegionNode *W, ReductionItem *RedI, Value *AI,
                                 Value *OldV, Instruction *InsertPt,
                                 bool IsInit, DominatorTree *DT,
                                 bool NoNeedToOffsetOrDerefOldV = false);

  /// Generate the reduction fini code for bool and/or.
  Value *genReductionFiniForBoolOps(Value *Rhs1, Value *Rhs2, Type *ScalarTy,
                                    IRBuilder<> &Builder, bool IsAnd);
  /// @}

  /// Generate the firstprivate initialization code.
  void genFprivInit(FirstprivateItem *FprivI, Instruction *InsertPt);

  /// Utility for last private update or copyprivate code generation.
  void genLprivFini(Item *I, Value *NewV, Value *OldV, Instruction *InsertPt);
  void genLprivFini(LastprivateItem *LprivI, Instruction *InsertPt);

  /// Collect all stores done to the local copy of the lastprivate item \p
  /// LprivI. Puts the collected store instructions in \p LprivIStores.
  void
  collectStoresToLastprivateNewI(WRegionNode *W, LastprivateItem *LprivI,
                                 SmallVectorImpl<Instruction *> &LprivIStores);
  /// Emit Init/Fini code for conditional lastprivate clause.
  void genConditionalLPCode(WRegionNode *W, LastprivateItem *LprivI,
                            Instruction *OMPLBForChunk, Instruction *OMPZtt,
                            Instruction *BranchToNextChunk,
                            Instruction *ConditionalLPBarrier);

  /// Generate the lastprivate update code for taskloop
  void genLprivFiniForTaskLoop(LastprivateItem *LprivI, Instruction *InsertPt);

  /// Generate loop scheduling code.
  /// \p IsLastVal is an output from this routine and is used to emit
  /// lastprivate code.
  /// If \p W is a loop construct with scheduling involving __kmpc_dispatch
  /// calls, \p InsertLastIterCheckBeforeOut is set to point to the instruction
  /// before which the lastprivate/linear finalization code should be inserted.
  /// \p NewOmpLBInstOut is set to the load of omp.lb, which is computed for use
  /// in the loop's ztt (lb < ub). \p NewOmpZttInstOut is set to the Instruction
  /// computing the loop's ztt (lb < ub).
  bool genLoopSchedulingCode(WRegionNode *W, AllocaInst *&IsLastVal,
                             Instruction *&InsertLastIterCheckBeforeOut,
                             Instruction *&NewOmpLBInstOut,
                             Instruction *&NewOmpZttInstOut);

  /// If an item has space allocated in the buffer at the end of the task's
  /// thunk (such as VLAs), make its New field in the privates thunk point to
  /// its corresponding buffer space.
  void linkPrivateItemToBufferAtEndOfThunkIfApplicable(
      Item *I, StructType *KmpPrivatesTy, Value *PrivatesGep,
      Value *TaskTWithPrivates, IRBuilder<> &Builder);

  /// Generate the code to replace the variables in the task loop with
  /// the thunk field dereferences
  bool genTaskLoopInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                           StructType *&KmpSharedTy, AllocaInst *&LBPtr,
                           AllocaInst *&UBPtr, AllocaInst *&STPtr,
                           Value *&LastIterGep, bool isLoop = true);
  bool genTaskInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                       StructType *&KmpSharedTy, Value *&LastIterGep);

  /// Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop_5 and the
  /// corresponding outlined function
  bool genTaskGenericCode(WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy,
                          StructType *KmpSharedTy, AllocaInst *LBPtr,
                          AllocaInst *UBPtr, AllocaInst *STPtr,
                          bool isLoop = true);

  /// Generate the call __kmpc_omp_task_alloc, __kmpc_omp_task and the
  /// corresponding outlined function.
  bool genTaskCode(WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy,
                   StructType *KmpSharedTy);

  /// Generate the call __kmpc_omp_taskwait.
  bool genTaskWaitCode(WRegionNode *W);

  /// Generate the function that destructs firstprivate local vars.
  Function *genTaskDestructorThunk(WRegionNode *W, StructType *TaskThunkType);

  /// Replace the shared variable reference with the thunk field
  /// dereference
  bool genSharedCodeForTaskGeneric(WRegionNode *W);

  /// Replace the reduction variable reference with the dereference of
  /// the return pointer __kmpc_task_reduction_get_th_data
  bool genRedCodeForTaskGeneric(WRegionNode *W);

  /// Generate the struct type kmp_task_red_input
  void genTaskTRedType();

  /// breif Generate the struct type kmp_depend_info
  void genKmpTaskDependInfo();

  /// Generate the call __kmpc_task_reduction_init and the corresponding
  /// preparation.
  void genRedInitForTask(WRegionNode *W, Instruction *InsertBefore);

  /// Generate the initialization code for the depend clause
  AllocaInst *genDependInitForTask(WRegionNode *W, Instruction *InsertBefore);

  /// The wrapper routine to generate the call __kmpc_omp_task_with_deps
  void genTaskDeps(WRegionNode *W, StructType *IdentTy, Value *TidPtr,
                   Value *TaskAlloc, AllocaInst *DummyTaskTDependRec,
                   Instruction *InsertPt, bool IsTaskWait);

  /// Create a struct to contain all shared data for the task. This is allocated
  /// in the caller of the task, and is populated with pointers to shared
  /// variables, reduction variables, and lastprivate variables.
  /// This is redundant in a way, but only contains pointers, so it isn't too
  /// bad for now. We can clean this further later by directly initializing the
  /// thunk.
  AllocaInst *genAndPopulateTaskSharedStruct(WRegionNode *W,
                                             StructType *KmpSharedTy);

  /// Copy the data contained in the shared struct on the task's caller, to the
  /// task's thunk.
  void copySharedStructToTaskThunk(WRegionNode *W, AllocaInst *Src, Value *Dst,
                                   StructType *KmpSharedTy,
                                   StructType *KmpTaskTTWithPrivatesTy,
                                   Function *DestrThunk, Instruction *InsertPt);

  /// Initialize the local copies of firstprivate variables in the task thunk,
  /// using the originals. This is done before executing the task region.
  void genFprivInitForTask(WRegionNode *W, Value *KmpTaskTTWithPrivates,
                           Value *KmpPrivatesGep, StructType *KmpPrivatesTy,
                           Instruction *InsertBefore);

  /// \defgroup TaskVLAFunctions Functions specific to handling VLAs on tasks
  ///
  /// For VLA operands to private/fp/lp clauses on tasks, the size of memory
  /// needed for the array is not known at compile time. So the memory cannot be
  /// allocated as part of the 'privates' struct in the task thunk, as its type
  /// needs to be determined at compile time. So, for VLAs, memory for the array
  /// itself is allocated in a buffer at the end of the task thunk, and three
  /// fields are reserved in the 'privates' struct. For example:
  ///
  /// \code
  ///  int a[n];
  ///  ...
  ///  #pragma omp task firstprivate(a) lastprivate(a) ...
  /// \endcode
  ///
  /// The memory allocated for this in the thunk looks like:
  ///
  /// \code
  ///
  ///  |<-----------task_t_with_privates---------->|
  ///  |<--task_t-->|<---------privates_t--------->|<---------buffer---------->|
  ///
  ///  {{.........} , {...i32*,   i64,    i64  ...}}..|< buffer_for_a >|........
  ///     |               |                           ^
  ///     |               |                           |
  ///     |             <anew>  <size >  <offset>     |< n * 4 bytes  >|
  ///     |               |     <n * 4>               |
  ///     |               |                           |
  ///     |               +---------------------------+
  ///  |<-+---------------<offset>------------------->|
  ///     |
  ///     |
  ///     |<-shared.t->|
  ///     V
  ///     {... i32*... }
  ///         <ashr>
  ///
  /// \endcode
  ///
  /// The space for the actual array is allocated in a buffer space after the
  /// end of 'task_t_with_privates' struct.
  ///
  /// The privates_t struct contains:
  /// - anew: an i32* which stores the address of 'buffer_for_a'. The
  /// initialization of this pointer, to store &'buffer_for_a', is done in the
  /// beginning of the task's outlined function. This code is emitted by
  /// genTaskLoopInitCode().
  /// - size: an i64 (size_t) containing the size in bytes of the array
  /// ('%n * sizeof(i32)').
  /// - offset: an i64 (size_t), containing the integer offset that needs to be
  /// added to the base address of 'task_t_with_privates', to reach the
  /// beginning of 'buffer_for_a'.
  ///
  /// The value of 'size' in bytes for each VLA is computed in
  /// genKmpTaskTWithPrivatesRecordDecl().
  ///
  /// The value of 'offset' for each VLA operand is computed in
  /// computeExtraBufferSpaceNeededAfterEndOfTaskThunk(). The function also
  /// computes the total size of private memory that needs to be passed to the
  /// '__kmp_task_alloc' call, which is sizeof('task_t_with_privates') +
  /// <size of all buffers>.
  ///
  /// The initialization of 'size' and 'offset' fields in the thunk are done in
  /// saveVLASizeAndOffsetToPrivatesThunk().
  ///
  /// Inside the outlined region for the task, 'offset' is needed to link 'anew'
  /// to 'buffer_of_a'. 'size' and 'ashr' (pointer to the original %a) are
  /// needed for lastprivate copyout and reduction finalization loops (not yet
  /// supported).
  ///
  /// {@
  /// Compute the total buffer memory needed per task for local copies of data
  /// corresponding to variable length arrays and similar operands. The function
  /// sets up the offset in bytes for the buffer corresponding to each clause
  /// item, from the beginning of the task thunk. \p TaskThunkWithPrivatesSize,
  /// which is the size of the thunk without any buffer at the end, would
  /// be the offset for the first item.
  /// The function inserts add instructions to compute the offsets corresponding
  /// to each VLA clause item, and also for the total size of the thunk +
  /// buffer. These are inserted before \p InsertBefore. \returns the Total size
  /// of the task thunk, including the buffer at the end.
  static Value *computeExtraBufferSpaceNeededAfterEndOfTaskThunk(
      WRegionNode *W, int TaskThunkWithPrivatesSize, Instruction *InsertBefore);

  /// For variable length arrays on tasks, save 'data size' and 'offset to the
  /// array in the buffer', into the space designated for them in the privates
  /// thunk.
  static void saveVLASizeAndOffsetToPrivatesThunk(WRegionNode *W,
                                                  Value *KmpPrivatesGEP,
                                                  StructType *KmpPrivatesTy,
                                                  Instruction *InsertBefore);
  /// @}

  /// Compute and return the GEP for the 'privates_t' struct contained within
  /// the 'task_t_with_privates' (\p KmpTaskTTWithPrivates) thunk.
  static Value *genPrivatesGepForTask(Value *KmpTaskTTWithPrivates,
                                      StructType *KmpTaskTTWithPrivatesTy,
                                      Instruction *InsertBefore);

  /// Save the loop lower upper bound, upper bound and stride for the use
  /// by the call __kmpc_taskloop_5
  void genLoopInitCodeForTaskLoop(WRegionNode *W, AllocaInst *&LBPtr,
                                  AllocaInst *&UBPtr, AllocaInst *&STPtr);

  /// Generate the outline function of reduction initialization
  Function *genTaskLoopRedInitFunc(WRegionNode *W, ReductionItem *RedI);

  /// Generate the outline function for the reduction update
  Function *genTaskLoopRedCombFunc(WRegionNode *W, ReductionItem *RedI);

  /// Generate the runtime callback to set the last iteration
  /// flag for lastprivates, and copy-construct firstprivates.
  Function *genFLPrivateTaskDup(WRegionNode *W,
                                StructType *KmpTaskTTWithPrivatesTy);

  /// Generate the function type void @routine_entry(i32 %tid, i8*)
  void genKmpRoutineEntryT();

  /// Generate the struct type %struct.kmp_task_t = type { i8*, i32 (i32,
  /// i8*)*, i32, %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t, i64, i64, i64,
  /// i32 }
  void genKmpTaskTRecordDecl();

  /// Generate the struct type kmpc_task_t as well as its private data
  /// area. One example is as follows.
  /// %struct.kmp_task_t_with_privates = type { %struct.kmp_task_t,
  /// %struct..kmp_privates.t }
  /// %struct.kmp_task_t = type { i8*, i32 (i32, i8*)*, i32,
  /// %union.kmp_cmplrdata_t, %union.kmp_cmplrdata_t, i64, i64, i64, i32}
  /// %struct..kmp_privates.t = type { i64, i64, i32 }
  /// If any item is a VLA type (say int x[n]), the 'privates_t' contains a
  /// pointer, and two size_t values (i32*, i64, i64). The size of the array in
  /// bytes (%n * sizeof(i32)) is computed in this function, and any code
  /// generated for that computation is inserted before \p InsertBefore.
  StructType *genKmpTaskTWithPrivatesRecordDecl(WRegionNode *W,
                                                StructType *&KmpSharedTy,
                                                StructType *&KmpPrivatesTy,
                                                Instruction *InsertBefore);

  /// Generate the actual parameters in the outlined function
  /// for copyin variables.
  void genThreadedEntryActualParmList(WRegionNode *W,
                                      std::vector<Value *>& MTFnArgs);

  /// Generate the formal parameters in the outlined function
  /// for copyin variables.
  void genThreadedEntryFormalParmList(WRegionNode *W,
                                      std::vector<Type *>& ParamsTy);

  /// Generate the name of formal parameters in the outlined function
  /// for copyin variables.
  void fixThreadedEntryFormalParmName(WRegionNode *W,
                                      Function *NFn);

  /// Utility to find Alignment of the COPYIN Variable passed.
  unsigned getAlignmentCopyIn(Value *V, const DataLayout DL);

  /// Generate the copy code for the copyin variables.
  void genTpvCopyIn(WRegionNode *W,
                    Function *NFn);

  /// Finalize extracted MT-function argument list for runtime
  Function *finalizeExtractedMTFunction(WRegionNode *W, Function *Fn,
                                        bool IsTidArg, unsigned int TidArgNo,
                                        bool hasBid = true);

  /// Generate __kmpc_fork_call Instruction after CodeExtractor
  CallInst* genForkCallInst(WRegionNode *W, CallInst *CI);

  /// Reset the expression value in the bundle to be empty.
  void resetValueInBundle(WRegionNode *W, Value *V);

  /// Reset the expression value of task depend clause to be empty.
  void resetValueInTaskDependClause(WRegionNode *W);

  /// Reset the expression value in private clause to be empty.
  void resetValueInPrivateClause(WRegionNode *W);

  /// Reset the expression value in livein clause to be empty.
  void resetValueInLiveinClause(WRegionNode *W);

  /// Reset the expression value in Subdevice clause to be empty.
  void resetValueInSubdeviceClause(WRegionNode* W);

  /// Set the value in num_teams, thread_limit and num_threads
  /// clauses to be empty.
  void resetValueInNumTeamsAndThreadsClause(WRegionNode *W);

  /// Reset the value in the Map clause to be empty.
  void resetValueInMapClause(WRegionNode *W);

  /// Reset the value of \p V in OpenMP clauses of \p W to be empty.
  void resetValueInOmpClauseGeneric(WRegionNode *W, Value *V);

  /// Rename duplicate values in \p W's map clauses.
  void renameDuplicateBasesInMapClauses(WRegionNode *W);

  /// \name Utilities to emit kmpc_begin/end_spmd calls for offloading.
  ///
  /// Code generation like this is needed to support `omp_get_num_threads()`
  /// in user code when hierarchical parallelism is used.
  ///
  /// \code
  ///   #pragma omp target
  ///   {
  ///     __kmpc_begin_spmd_target();
  ///       ...
  ///       __kmpc_begin_spmd_parallel();
  ///       #pragma omp parallel
  ///       {}
  ///       __kmpc_end_spmd_parallel()
  ///       ...
  ///     __kmpc_end_spmd_target();
  ///   }
  /// \endcode
  ///
  /// Note that these calls are emitted only if the current module
  /// contains `omp_get_num_threads` function and Paropt determines that a
  /// region may call it, to avoid performance impact of these calls and
  /// the barriers associated with them.
  ///
  /// Emission of these calls can also be disabled using the command line flag:
  ///   -vpo-paropt-simulate-get-num-threads-in-target=false
  ///
  /// @{
  ///
  /// Inserts calls to `__kmpc_begin_spmd_parallel` and
  /// `__kmpc_end_spmd_parallel` around \p W. The calls are inserted outside
  /// \p W's region entry/exit directives.
  bool callBeginEndSpmdParallelAtRegionBoundary(WRegionNode *W);

  /// Inserts calls to `__kmpc_begin_spmd_target` and
  /// `__kmpc_end_spmd_target` at the boundary of \p W. The begin call is
  /// inserted after \p W's entry directive, and the end call is inserted
  /// before \p W's exit directive.
  bool callBeginEndSpmdTargetAtRegionBoundary(WRegionNode *W);

  /// @}

  /// Generate the code for the directive omp target
  bool genTargetOffloadingCode(WRegionNode *W);

  /// Collect the data mapping information for the given region \p W
  /// based on the \p Call instruction created during the region outlining.
  /// The method populates \p ConstSizes, \p MapTypes, \p Names and \p Mappers
  /// vectors with the mapping information for each argument of \p Call.
  /// See genTgtInformationForPtrs() for more details about the meaning
  /// of these vectors.
#if INTEL_CUSTOMIZATION
  /// \p IsWILocalFirstprivate holds 'true' for those pointers that
  /// identify mapped objects that can be firstprivated using SPIR-V
  /// __private storage class vs __global storage class.
  /// \p IsWILocalFirstprivate is used only for SPIR-V targets.
#endif // INTEL_CUSTOMIZATION
  /// \p IsFunctionPtr holds 'true' for pointers to functions.
  /// The result is used to create pointer-to-int (and back) casts for
  /// function pointers passed into kernel functions for SPIR-V targets (to
  /// align with the OpenCL convention).
  /// \p HasRuntimeEvaluationCaptureSize is set to true
  /// iff any of the mappings requires dynamically computed size,
  /// otherwise, it is set to false.
  /// Return the number of entries in the output \p MapTypes.
  unsigned getTargetDataInfo(
      WRegionNode *W, const CallInst *Call,
      SmallVectorImpl<Constant *> &ConstSizes,
      SmallVectorImpl<uint64_t> &MapTypes,
      SmallVectorImpl<GlobalVariable *> &Names,
      SmallVectorImpl<Value *> &Mappers,
#if INTEL_CUSTOMIZATION
      SmallVectorImpl<bool> &IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
      SmallVectorImpl<bool> &IsFunctionPtr,
      bool &HasRuntimeEvaluationCaptureSize) const;

  /// Generate the initialization code for the directive omp target
  CallInst *genTargetInitCode(WRegionNode *W, CallInst *Call, Value *RegionId,
                              Instruction *InsertPt);

  /// Generate the pointers pointing to the array of base pointer, the
  /// array of section pointers, the array of sizes, the array of map types.
  void genOffloadArraysArgument(TgDataInfo *Info, Instruction *InsertPt);

  /// Pass the data to the array of base pointer as well as  array of
  /// section pointers. If the flag hasRuntimeEvaluationCaptureSize is true,
  /// the compiler needs to generate the init code for the size array.
  void genOffloadArraysInit(WRegionNode *W, TgDataInfo *Info, CallInst *Call,
                            Instruction *InsertPt,
                            SmallVectorImpl<Constant *> &ConstSizes,
                            SmallVectorImpl<uint64_t> &MapTypes,
                            SmallVectorImpl<GlobalVariable *> &Names,
                            SmallVectorImpl<Value *> &Mappers,
                            bool hasRuntimeEvaluationCaptureSize);

  /// Utility to construct the assignment to the base pointers, section
  /// pointers (and size pointers if the flag hasRuntimeEvaluationCaptureSize is
  /// true). Sets \p BasePtrGEPOut to the GEP where \p BasePtr is stored.
  void genOffloadArraysInitUtil(IRBuilder<> &Builder, Value *BasePtr,
                                Value *SectionPtr, Value *Size, Value *Mapper,
                                TgDataInfo *Info,
                                SmallVectorImpl<Constant *> &ConstSizes,
                                unsigned &Cnt,
                                bool hasRuntimeEvaluationCaptureSize,
                                Instruction **BasePtrGEPOut = nullptr);

  /// Fixup references generated for global variables in OpenMP
  /// clauses for targets supporting non-default address spaces.
  /// This fixup has to be done as soon as possible after FE.
  bool canonicalizeGlobalVariableReferences(WRegionNode *W);

  // For a global variable in non-target constructs, check if its ancestor
  // WRegionNode has it in privatizing clause. If it does, call
  // genGlobalPrivatizationLaunderIntrin for that Value.
  bool genLaunderIntrinIfPrivatizedInAncestor(WRegionNode *W);

  // If the incoming data is global variable, create a stack variable
  // and replace the global variable with the stack variable.
  // ValuesToChange if set, gives the list of global variables to be changed.
  bool genGlobalPrivatizationLaunderIntrin(
      WRegionNode *W,
      const std::unordered_set<Value *> *ValuesToChange = nullptr);

  /// Generate the sizes and map type flags for the given map type, map
  /// modifier and the expression V.
  /// \param [in]     W               incoming WRegionNode.
  /// \param [in]     V               base pointer.
  /// \param [out]    ConstSizes      array of size information.
  /// \param [out]    MapTypes        array of map types.
  /// \param [out]    Names           array of names.
  /// \param [out]    Mappers         array of mappers.
#if INTEL_CUSTOMIZATION
  /// \param [out]    IsWILocalFirstprivate
  ///                 'true' for pointers that identify mapped objects
  ///                 that can be firstprivatized using SPIR-V __private
  ///                 storage class vs __global storage class.
  ///                 This result is used only for SPIR-V targets.
#endif // INTEL_CUSTOMIZATION
  /// \param [out]    IsFunctionPtr
  ///                 'true' for pointers to functions. The result is
  ///                 used to create pointer-to-int (and back) casts
  ///                 for function pointers passed into kernel functions
  ///                 for SPIR-V targets (to align with the OpenCL
  ///                 convention).
  /// \param [out]    hasRuntimeEvaluationCaptureSize
  ///                 size cannot be determined at compile time.
  /// \param [in] VIsTargetKernelArg `true` iff \p V is a kernel
  /// function argument for a target construct.
  void genTgtInformationForPtrs(WRegionNode *W, Value *V,
                                SmallVectorImpl<Constant *> &ConstSizes,
                                SmallVectorImpl<uint64_t> &MapTypes,
                                SmallVectorImpl<GlobalVariable *> &Names,
                                SmallVectorImpl<Value *> &Mappers,
#if INTEL_CUSTOMIZATION
                                SmallVectorImpl<bool> &IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                                SmallVectorImpl<bool> &IsFunctionPtr,
                                bool &hasRuntimeEvaluationCaptureSize,
                                bool VIsTargetKernelArg = false) const;

  /// Generate multithreaded for a given WRegion
  bool genMultiThreadedCode(WRegionNode *W);

  /// Generate code for Taskyield construct.
  bool genTaskyieldCode(WRegionNode *W);

  /// Generate code for masked/end masked construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genMaskedThreadCode(WRegionNode *W, bool IsTargetSPIRV);

  /// Generate code for single/end single construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genSingleThreadCode(WRegionNode *W, AllocaInst *&IsSingleThread);

  /// Generate code for ordered/end ordered construct for preserving ordered
  /// region execution order
  bool genOrderedThreadCode(WRegionNode *W);

  /// Emit __kmpc_doacross_post/wait call for an 'ordered depend(source/sink)'
  /// construct.
  bool genDoacrossWaitOrPost(WRNOrderedNode *W);

  /// Generates code for the OpenMP critical construct:
  /// #pragma omp critical [(name)]
  bool genCriticalCode(WRNCriticalNode *CriticalNode);

  /// Generate code for OMP scope construct.
  /// #pragma omp scope private(x) reduction(+:x) nowait
  bool genBeginScopeCode(WRegionNode *W);
  bool genEndScopeCode(WRegionNode *W);

  /// Return true if the program is compiled at the offload mode.
  bool hasOffloadCompilation() const {
    return ((Mode & OmpOffload) || VPOParoptUtils::isForcedTargetCompilation());
  }

  /// Finds the alloc stack variables where the tid stores.
  void getAllocFromTid(CallInst *Tid);

  /// Finds the function pointer type for the function
  /// void (*kmpc_micro)(kmp_int32 *global_tid, kmp_int32 *bound_tid, ...)
  FunctionType* getKmpcMicroTaskPointerTy();

  /// The data structure that is used to store the alloca or tid call
  ///  instruction that are used in the WRegion.
  SmallPtrSet<Instruction*, 8> TidAndBidInstructions;

  /// Emits an implicit barrier at the end of WRegion \p W if W contains
  /// variables that are linear, or both firstprivate-lastprivate. e.g.
  ///
  ///   #pragma omp for firstprivate(x) lastprivate(x) nowait
  ///
  /// Emitted pseudocode:
  ///
  /// \code
  ///   %x.local = @x                         ; (1) firstprivate copyin
  ///   __kmpc_static_init(...)               ; (i) init call
  ///   ...
  ///   __kmpc_static_fini(...)
  ///
  ///   __kmpc_barrier(...)                   ; (2)
  ///   @x = %x.local                         ; (3) lastprivate copyout
  ///
  /// \endcode
  ///
  /// The barrier (2) is needed to prevent a race between (1) and (3), which
  /// read from / write to @x.
  ///
  /// For supporting non-monotonic scheduling on loops, the barrier is to be
  /// inserted before the 'init call (i)'. This is done by passing in '(i)'
  /// as \p InsertBefore.
  bool genBarrierForFpLpAndLinears(WRegionNode *W,
                                   Instruction *InsertBefore = nullptr);

  /// Emit and return an implicit barrier if \p W has any conditional lasptivate
  /// clause operands.
  Instruction *genBarrierForConditionalLP(WRegionNode *W);

  /// Emits an if-then branch using \p IsLastLocs and sets \p IfLastIterOut to
  /// the if-then BBlock. This is used for emitting the final copy-out code for
  /// linear and lastprivate clause operands.
  ///
  /// Code generated looks like:
  ///
  /// \code
  ///       Before             |      After
  /// -------------------------+----------------------------------------------
  ///                          |   %15 = load i32, i32* %is.last
  ///                          |   %16 = icmp ne i32 %15, 0
  ///                          |   br i1 %16, label %last.then, label %last.done
  ///                          |
  ///                          |   last.then:        ; IfLastIterOut
  ///                          |   ...
  ///                          |   br last.done
  ///                          |
  ///                          |   last.done:
  ///                          |   br exit.BB.predecessor
  ///                          |
  ///                          |   exit.BB.predecessor:
  ///                          |   br exit.BB
  ///                          |
  /// exit.BB:                 |   exit.BB:
  /// llvm.region.exit(...)    |   llvm.region.exit(...)
  ///
  /// \endcode
  ///
  /// \param [in] IsLastLocs A list of stack variables which are non-zero
  /// if the current thread executes the last iteration of the loop(s).
  /// If there is more than one loop associated with the region, then
  /// the list will contain a variable for each loop. In this case,
  /// the "last iteration" is defined as logical and of the variables'
  /// values being non-zero.
  /// \param [out] IfLastIterOut The BasicBlock for when the last iteration
  /// check is true.
  /// \param [in] InsertBefore If not null, the branch is inserted before it.
  /// Otherwise, the branch is inserted before \p W's exit BB.
  ///
  /// \returns \b true if the branch is emitted, \b false otherwise.
  ///
  /// The branch is not emitted if \p W has no Linear or Lastprivate var.
  bool genLastIterationCheck(WRegionNode *W,
                             const ArrayRef<Value *> IsLastLocs,
                             BasicBlock *&IfLastIterOut,
                             Instruction *InsertBefore = nullptr);

  /// Insert a barrier at the end of the construct if \p InsertBefore is
  /// null. Otherwise, insert the barrier before \p InsertBefore.
  /// If \p BarrierOut is provided, it is set to point to the emitted barrier.
  bool genBarrier(WRegionNode *W, bool IsExplicit, bool IsTargetSPIRV = false,
                  Instruction *InsertBefore = nullptr,
                  Instruction **BarrierOut = nullptr);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  /// Forward declarations for helper classes which implement CSA
  ///  specific privatization for OpenMP constructs.
  class CSAPrivatizer;
  class CSALoopPrivatizer;
  class CSAExpLoopPrivatizer;
  class CSASectionsPrivatizer;
  class CSALoopSplitter;
  friend CSAPrivatizer;
  friend CSALoopSplitter;

  /// Transform "omp parallel" work region for CSA target.
  bool genCSAParallel(WRegionNode *W);

  /// Transform "omp [parallel] for" work region for CSA target.
  /// Returns a pair of boolean values where the first value tells if function
  /// was changed and the second whether work region's directives should be
  /// removed.
  std::pair<bool, bool> genCSALoop(WRegionNode *W);

  /// Transform "omp [parallel] sections" work region for CSA target.
  bool genCSASections(WRegionNode *W);

  /// Lower OpenMP runtime library calls.
  bool translateCSAOmpRtlCalls();

  /// Transform "omp single" work region for CSA target.
  bool genCSASingle(WRegionNode *W);

  /// Transform "omp critical" work region for CSA target.
  bool genCSACritical(WRNCriticalNode *W);

  /// Check whether a given construct is supported in CSA.
  bool isSupportedOnCSA(WRegionNode *W);
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  /// Insert a flush call
  bool genFlush(WRegionNode *W);

  /// Generate prefetch code
  bool genPrefetchCode(WRegionNode *W, bool IsTargetSPIRV);

  /// \name Cancellation Specific Functions
  /// {@

  /// Generates code for the OpenMP cancel constructs:
  /// \code
  /// #pragma omp cancel [type]
  /// #pragma omp cancellation point [type]
  /// \endcode
  bool genCancelCode(WRNCancelNode *W);

  /// Add any cancellation points within \p W's body, to its
  /// `region.entry` directive. This is done in the vpo-paropt-prepare pass, and
  /// is later consumed by the vpo-paropt transformation pass.
  ///
  /// A `cancellation point` can be one of these calls:
  /// \code
  ///   %1 = __kmpc_cancel_barrier(...)
  ///   %2 = __kmpc_cancel(...)
  ///   %3 = __kmpc_cancellationpoint(...)
  /// \endcode
  ///
  /// The IR after the transformation looks like:
  /// \code
  ///   %cp1 = alloca i32
  ///   %cp2 = alloca i32
  ///   %cp3 = alloca i32
  ///   ...
  ///   %0 = call token @llvm.directive.region.entry(...) [...,
  ///   "QUAL.OMP.CANCELLATION.POINTS"(%cp1, %cp2, %cp3) ]
  ///   ...
  ///   %1 = __kmpc_cancel_barrier(...)
  ///   store %1, %cp1
  ///   %2 = __kmpc_cancel(...)
  ///   store %1, %cp2
  ///   %3 = __kmpc_cancellationpoint(...)
  ///   store %1, %cp3
  ///
  ///   call void @llvm.directive.region.exit(%0)
  /// \endcode
  bool propagateCancellationPointsToIR(WRegionNode *W);

  /// Removes from IR, the allocas and stores created by
  /// propagateCancellationPointsToIR(). This is done in the vpo-paropt
  /// transformation pass after the information has already been consumed. The
  /// function also removes these allocas from the
  /// "QUAL.OMP.CANCELLATION.POINTS" clause on the region.entry intrinsic.
  ///
  /// \code
  ///       Before                      |     After
  ///  ---------------------------------+------------------------------------
  ///  %cp = alloca i32                 |   <deleted>
  ///  ...                              |   ...
  ///                                   |
  ///  directive.region.entry(...%cp...)|   directive.region.entry(...null...)
  ///                                   |
  ///  %x = kmpc_cancel(...)            |   %x = kmpc_cancel(...)
  ///  store %x, %cp                    |   <deleted>
  ///  ...                              |   ...
  ///                                   |
  /// \endcode
  bool clearCancellationPointAllocasFromIR(WRegionNode *W);

  /// Generate branches to jump to the end of a construct from
  /// every cancellation point within the construct.
  ///
  /// For each cancellation point '%x' within the body of W:
  ///
  /// \code
  ///       Before                      |     After
  ///  ---------------------------------+------------------------------------
  ///  %x = kmpc_cancel(...)            |     %x = kmpc_cancel(...)
  ///                                   |     if (%x != 0) {
  ///                                   |       goto CANCEL.EXIT.BB;
  ///                                   |     }
  ///                                   |     NOT.CANCELLED.BB:
  ///  <code_after_cancellation_point>  |     <code_after_cancellation_point>
  ///  ...                              |     ...
  ///                                   |
  ///                                   |     CANCEL.EXIT.BB:
  ///                                   |
  ///  EXIT.BB:                         |     EXIT.BB:
  ///  directive.region.exit(%x)        |     directive.region.exit(null)
  ///  return;                          |     return;
  ///
  /// \endcode
  bool genCancellationBranchingCode(WRegionNode *W);

  /// For non-pointer values which will be used directly inside the outlined
  /// region for \p W, rename them using store-then-load, and mark the pointer
  /// where they are stored, as shared on the region's directive. This is done
  /// because the OpenMP runtime only accepts pointer arguments for the outlined
  /// functions, so we need to pass these values in by-reference. For example:
  ///
  /// Consider the following IR example. Assume that `%vla` was in a private
  /// clause on the directive for W before vpo-paropt transformation pass, like
  /// this:
  ///
  /// \code
  ///   %size.val = load i32, i32* %size
  ///   %vla = alloca i32, i32 %size.val
  ///   call @llvm.region.entry("DIR.OMP.PRIVATE"..."QUAL.OMP.PRIVATE"(i32* vla)
  /// \endcode
  ///
  /// Suppose `%vla.private` is the local copy created for `%vla` after handling
  /// the clause in genPrivatizationCode(). Calling this function makes the
  /// following changes:
  ///
  /// \code
  ///       Before                      |     After
  ///  ---------------------------------+------------------------------------
  ///  %size.val = load i32, i32* %size |     %size.val = load i32, i32* %size
  ///  %vla = alloca i32, i32 %size.val |     %vla = alloca i32, i32 %size.val
  ///  ...                              |     ...
  ///                                   |
  ///                                   |     %size2 = alloca i32
  ///                                   |     store i32 %size.val, i32* size2
  ///                                   |
  ///  <EntryBB>:                       |   <EntryBB>:
  ///                                   |     %size.val2 = load i32, i32* size2
  ///  ...                              |     ...
  ///  %vla.private = alloca i32,       |     %vla.private = alloca i32,
  ///                 i32 %size.val     |                    i32 %size.val2
  ///                                   |
  ///                        <entry directive for W>
  ///  (..."DIR.OMP.PARALLEL"...)       |     (..."DIR.OMP.PARALLEL"...
  ///                                   | ;    "QUAL.OMP.SHARED" (i32* size2))
  ///                                   |
  ///                                   | ; Note: `%size2` is implicitly added
  ///                                   | ; as a shared clause item in the
  ///                                   | ; WRegionNode W, but the directive in
  ///                                   | ; the IR is not modified.
  /// \endcode
  ///
  /// If \p W is for a target construct, `QUAL.OMP.FIRSTPRIVATE` is used instead
  /// of `QUAL.OMP.SHARED` for `%size2`.
  /// \see WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion() for
  /// more details.
  bool captureAndAddCollectedNonPointerValuesToSharedClause(WRegionNode *W);

  /// Clean up the intrinsic @llvm.launder.invariant.group and replace
  /// the use of the intrinsic with the its operand.
  bool clearCodemotionFenceIntrinsic(WRegionNode *W);

  /// Returns the corresponding flag for a given map clause modifier.
  /// Does not set TARGET_PARAM may-type flag unless IsTargetKernelArg is true.
  uint64_t getMapTypeFlag(MapItem *MpI,
                         bool AddrIsTargetParamFlag,
                         bool IsFirstComponentFlag,
                         bool IsTargetKernelArg) const;

  /// If \p I is a call to @llvm.launder.invariant.group, then return
  /// the CallInst*. Otherwise, return nullptr.
  static CallInst* isFenceCall(Instruction *I);

  /// Collect the live-in value for the phis at the loop header.
  /// Collect the live-out set for the loop.
  /// "LiveIn" Values are PHINode values with one incoming value from Loop
  /// preheader and others from other basic blocks/Loop Latch.
  /// A defined Value is considered "LiveOut" if it is used outside the loop or
  /// it has loop-carried dependence. A variable has loop-carried dependence if
  /// it is present in "LiveIn" set of the Loop.
  /// LoopInductionVariables are not included in the sets,since they are handled
  /// in a special way using threadID.
  /// Eg:
  /// LoopHeader:                             %preds=LoopPreheader, LoopLatch
  ///  %phi0 = (0 LoopPreheader, %phi3 LoopLatch)
  ///  %phi1 = (0 LoopPreheader, %add LoopLatch)
  ///  %phi2 = (0 LoopPreheader, %phi4 LoopLatch)
  /// ....
  ///
  /// LoopBody1:
  ///  %add = phi1 + 1
  /// ....
  ///
  /// LoopLatch:                                  %preds=LoopBody1, LoopBody2
  ///  %phi3 = (1 LoopBody1, 1 LoopBody2) <<- PHI inserted in
  ///                                         updateConstantLoopHeaderPhis()
  ///  %phi4 = (%phi2 LoopBody2, 1 LoopBody1)
  ///  ...
  ///  br LoopHeader
  ///
  /// LiveIn : %phi3, %add, %phi4
  /// LiveOut: %add, %phi3, %phi4
  void wrnUpdateSSAPreprocess(
      Loop *L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveoutVals,
      EquivalenceClasses<Value *> &ECs);
  /// Replace the live-in value of the phis at the loop header with
  /// the loop carried value.
  void wrnUpdateSSAPreprocessForOuterLoop(
      Loop *L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveOutVals,
      EquivalenceClasses<Value *> &ECs);

  /// Update the SSA form in the region using SSA Updater.
  void wrnUpdateSSAForLoopRecursively(
      Loop *L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveOutVals);

  /// Collect the live-in values for the given loop.
  void wrnCollectLiveInVals(
      Loop &L,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      EquivalenceClasses<Value *> &ECs);

  /// Build the equivalence class for the value a, b if there exists some
  /// phi node e.g. a = phi(b).
  void buildECs(Loop *L, Value *PN, EquivalenceClasses<Value *> &ECs);

  /// The utility to build the equivalence class for the value phi.
  void AnalyzePhisECs(Loop *L, Value *PV, Value *V,
                      EquivalenceClasses<Value *> &ECs,
                      SmallPtrSet<PHINode *, 16> &PhiUsers);

  /// Collect the live-out values for a given loop.
  void wrnCollectLiveOutVals(
      Loop &L, SmallSetVector<Instruction *, 8> &LiveOutVals,
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      EquivalenceClasses<Value *> &ECs);

  /// The utility to update the liveout set from the given BB.
  void wrnUpdateLiveOutVals(Loop *L, BasicBlock *BB,
                            SmallSetVector<Instruction *, 8> &LiveOutVals,
                            EquivalenceClasses<Value *> &ECs);

  /// Generate the copyprivate code.
  bool genCopyPrivateCode(WRegionNode *W, AllocaInst *IsSingleThread);

  /// Generate the helper function for copying the copyprivate data.
  Function *genCopyPrivateFunc(WRegionNode *W, StructType *KmpCopyPrivateTy);

  /// Return true if the device triple contains spir64 or spir.
  bool deviceTriplesHasSPIRV();

  /// Update the SSA form after the basic block LoopExitBB's successor
  /// is added one more incoming edge.
  void rewriteUsesOfOutInstructions(
      DenseMap<Value *, std::pair<Value *, BasicBlock *>> &ValueToLiveinMap,
      SmallSetVector<Instruction *, 8> &LiveOutVals,
      EquivalenceClasses<Value *> &ECs);

  /// Generate local version of \p V variable inside the region.
  ///
  /// \p V is an optionally addrspacecast'ed AllocaInst for \p W region's
  /// normalized upper bound pointer or normalized induction variable
  /// pointer.
  /// Insert a new AllocaInst at the region's entry block allocating
  /// an object as specified by \p ElementTy and \p NumElements.
  /// Copy the original variable value to the allocated area, iff
  /// \p IsFirstPrivate is true.
  /// Replace all uses of the original AllocaInst with the new one.
  ///
  /// Since \p V is a normalized upper bound or induction variable
  /// pointer, we do not expect it to have non-POD type
  /// neither expect it to be By-Ref.
  Value *genRegionPrivateValue(
      WRegionNode *W, Value *V, Type *ElementTy, Value *NumElements,
      bool IsFirstPrivate = false);

  /// Move SIMD directives next to the loop associated
  /// with the given OpenMP loop region \p W.
  ///
  /// \p W is an OpenMP loop region.  If this method finds
  /// a SIMD region, which is a child of \p W, then it moves
  /// the corresponding SIMD directive next to the loop.
  /// After code generation for \p W, the enclosed SIMD
  /// directives may become too distant from the loop itself,
  /// which prevents correct handling of the SIMD loop by the later
  /// passes.  This method helps to solve these problems.
  ///
  /// Note that the enclosed SIMD region will be in inconsistent
  /// state after this method moves the directives, in particular,
  /// the original entry/exit blocks of the SIMD region will no longer
  /// hold the SIMD directives.  This should not be a problem,
  /// as long as we process regions from children to parents.
  /// If we ever need to keep the SIMD region in consistent state,
  /// we have to be able to update the region's entry/exit blocks.
  bool sinkSIMDDirectives(WRegionNode *W);

  /// Add parallel access metadata to memory r/w instructions in the given
  /// OpenMP loop region \p W.
  bool genParallelAccessMetadata(WRegionNode *W);

  /// Remove distribute loop back edge on SPIRV target at O2+ when ND-range is
  /// known and distribute schedule is not specified. Such loops will have at
  /// most only one iteration, so it is safe to remove loop's back edge.
  bool removeDistributeLoopBackedge(WRegionNode *W);

  /// Transform the given OMP loop into the loop as follows.
  ///         do {
  ///             %omp.iv = phi(%omp.lb, %omp.inc)
  ///             ...
  ///             %omp.inc = %omp.iv + 1;
  ///          }while (%omp.inc <= %omp.ub)
  ///
  ///  If the flag First is true, it indicates that it is called
  ///  in the pass VPOParoptPrepare. This utility also promotes the
  ///  loop index variable into the register and performs loop rotation.
  bool regularizeOMPLoop(WRegionNode *W, bool First = true);

  /// Check if loop is optimized away.
  bool isLoopOptimizedAway(WRegionNode *W);

  /// For the given loop \p Index in the loop kind region \p W
  /// promote the loop's IV and UB variables to registers.
  void registerizeLoopEssentialValues(WRegionNode *W, unsigned Index);

  /// Utility to simplify PHI instructions in the given loop \p L.
  /// Here is an example of a PHI instruction that may be produced
  /// by the loop rotation transformation that we want to simplify:
  ///   %2 = phi i64 [ %0, %bb1 ], [ %0, %bb2 ]
  /// We want to relink all users of %2 to %0.
  void simplifyLoopPHINodes(const Loop &L, const SimplifyQuery &SQ) const;

  /// Transform the Ith level of the loop in the region W into the
  /// OMP canonical loop form. In case the loop is optimized away, set
  /// LoopOptimizedAway and return false, otherwise return true.
  bool regularizeOMPLoopImpl(WRegionNode *W, unsigned I);

  /// Transform the given do-while loop into the canonical form
  /// as follows.
  ///         do {
  ///             %omp.iv = phi(%omp.lb, %omp.inc)
  ///             ...
  ///             %omp.inc = %omp.iv + 1;
  ///          }while (%omp.inc <= %omp.ub)

  void fixOMPDoWhileLoop(WRegionNode *W, Loop *L);

  /// Utility to transform the given do-while loop loop into the
  /// canonical do-while loop.
  void fixOmpDoWhileLoopImpl(Loop *L);

  /// Utilty to transform the loop branch predicate from sle/ule to
  /// sgt/ugt in order to faciliate the scev based loop trip count calculation.
  void fixOmpBottomTestExpr(Loop *L);

  /// Replace the use of OldV within region W with the value NewV.
  void replaceUseWithinRegion(WRegionNode *W, Value *OldV, Value *NewV);

  /// Returns the `omp_get_num_threads` function, if current Module contain it,
  /// \b nullptr otherwise.
  Function *getOmpGetNumThreadsFunctionIfPresent();

  /// Collect information about potential callers of `omp_get_num_threads` (if
  /// not already computed).
  void collectOmpNumThreadsCallerInfo();

  /// \returns \b true if the \p W may call `omp_get_num_threads`, or another
  /// function that may call it. \b false otherwise.
  bool mayCallOmpGetNumThreads(WRegionNode *W);

  /// \returns \b true if current function is marked with
  /// `openmp-declare-target=true` attribute, \b false otherwise.
  bool isFunctionOpenMPTargetDeclare();

  /// Initialize the loop descriptor struct with the loop level
  /// as well as the lb, ub, stride for each level of the loop.
  /// The loop descriptor struct as follows.
  ///    struct tgt_nd_desc {
  ///      int64_t   levels;      // The levels of the loop nest
  ///      struct tgt_loop_desc {
  ///        int64_t lb;          // The lb of the ith loop
  ///        int64_t ub;          // The ub of the ith loop
  ///        int64_t stride;      // The stride of the ith loop
  ///      }loop_desc[levels];
  ///   };
  AllocaInst *genTgtLoopParameter(WRegionNode *W);

  /// Generate the cast i8* for the incoming value BPVal.
  Value *genCastforAddr(Value *BPVal, IRBuilder<> &Builder);

  // Replace the new generated local variables with global variables
  // in the target initialization code.
  // Given a global variable in the offloading region, the compiler will
  // generate different code for the following two cases.
  //
  // case 1: global variable is not in the map clause.
  // The compiler generates %aaa stack variable which is initialized with
  // the value of @aaa. The base pointer and section pointer arrays are
  // initialized with %aaa.
  //
  //   #pragma omp target
  //   {  aaa++; }
  //
  // ** IR Dump After VPO Paropt Pass ***
  // entry:
  //   %.offload_baseptrs = alloca [1 x i8*]
  //   %.offload_ptrs = alloca [1 x i8*]
  //   %aaa = alloca i32
  //   %0 = load i32, i32* @aaa
  //   store i32 %0, i32* %aaa
  //   br label %codeRepl
  //
  // codeRepl:
  //   %1 = bitcast i32* %aaa to i8*
  //   %2 = getelementptr inbounds [1 x i8*],
  //        [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
  //   store i8* %1, i8** %2
  //   %3 = getelementptr inbounds [1 x i8*],
  //        [1 x i8*]* %.offload_ptrs, i32 0, i32 0
  //   %4 = bitcast i32* %aaa to i8*
  //   store i8* %4, i8** %3
  //
  // case 2: global variable is in the map clause
  // The compiler initializes the base pointer and section pointer arrays
  // with @aaa.
  //
  //   #pragma omp target map(aaa)
  //   {  aaa++; }
  //
  // ** IR Dump After VPO Paropt Pass ***
  // codeRepl:
  //   %1 = bitcast i32* @aaa to i8*
  //   %2 = getelementptr inbounds [1 x i8*],
  //        [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
  //   store i8* %1, i8** %2
  //   %3 = getelementptr inbounds [1 x i8*],
  //        [1 x i8*]* %.offload_ptrs, i32 0, i32 0
  //   %4 = bitcast i32* @aaa to i8*
  //   store i8* %4, i8** %3
  bool clearLaunderIntrinBeforeRegion(WRegionNode *W);

  /// Generate the target intialization code for the pointers based
  /// on the order of the map clause.
  void genOffloadArraysInitForClause(WRegionNode *W, TgDataInfo *Info,
                                     CallInst *Call, Instruction *InsertPt,
                                     SmallVectorImpl<Constant *> &ConstSizes,
                                     bool hasRuntimeEvaluationCaptureSize,
                                     Value *BPVal, bool &Match,
                                     IRBuilder<> &Builder, unsigned &Cnt);

  /// Generate code for OMP taskgroup construct.
  /// #pragma omp taskgroup
  bool genTaskgroupRegion(WRegionNode *W);
#if INTEL_CUSTOMIZATION

  /// Add alias_scope and no_alias metadata to improve the alias
  /// results in the outlined function.
  void improveAliasForOutlinedFunc(WRegionNode *W);

  /// Promote shared items to firstprivate (effectively) if we can prove that
  /// item is not modified inside the region. Such items remain 'shared' on the
  /// directive bundle, but a private instance is allocated and initialized
  /// inside the region and all references to the original instance are replaced
  /// with the private one.
  bool privatizeSharedItems(WRegionNode *W);

  /// Analyse work region's clauses and check if they can be simplified. If
  /// simplification cannot be performed by the compiler emit diagnostic message
  /// for the user. Return true if work region has been modified.
  bool simplifyRegionClauses(WRegionNode *W);

  /// Analyse work region's lastprivate clauses and check if they can be
  /// simplified. If simplification cannot be performed by the compiler emit
  /// diagnostic message for the user. Return true if work region has been
  /// modified.
  bool simplifyLastprivateClauses(WRegionNode *W);
#endif // INTEL_CUSTOMIZATION

  /// Guard each instruction that has a side effect with master thread id
  /// check, so that only the master thread (id == 0) in the team executes
  /// the code, then put a barrier before the start and after the end of
  /// every parallel region, so that all the threads in the team wait for
  /// the master thread, and can see its update of team shared memory.
  /// \p KernelF is an outlined function of region \p W.
  void guardSideEffectStatements(WRegionNode *W, Function *KernelF);

public:
  /// Replace printf() calls in \p F with _Z18__spirv_ocl_printfPU3AS2ci()
  /// \p PrintfDecl is the function printf(). \p OCLPrintfDecl is the function
  /// _Z18__spirv_ocl_printfPU3AS2ci(). If \p PrintfDecl is null, the utility
  /// returns without doing anything. if \p F is null, then all call
  /// instructions to the function \p PrintfDecl are replaced.
  static void replacePrintfWithOCLBuiltin(Function *PrintfDecl,
                                          Function *OCLPrintfDecl,
                                          Function *F = nullptr);

private:
  /// Set the kernel arguments' address space as ADDRESS_SPACE_GLOBAL.
  /// Propagate the address space from the arguments to the usage of the
  /// arguments.
  Function *finalizeKernelFunction(WRNTargetNode *WT, Function *Fn,
      CallInst *&Call, const SmallVectorImpl<uint64_t> &MapTypes,
#if INTEL_CUSTOMIZATION
      const SmallVectorImpl<bool> &IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
      const SmallVectorImpl<bool> &IsFunctionPtr,
      const SmallVectorImpl<Constant *> &ConstSizes);

  ///  Generate the iteration space partitioning code based on OpenCL.
  ///  Given a loop as follows.
  ///  \code
  ///    for (i = 0; i <= ub; i++)
  ///  \endcode
  ///  The output of partitioning as below.
  ///  \code
  ///    chunk_size = (ub + get_local_size()) / get_local_size();
  ///    new_lb = get_local_id * chunk_size;
  ///    new_ub = min(chunk_size - 1, ub);
  ///    for (i = new_lb; i <= new_ub; i++)
  ///  \endcode
  ///  Here we assume the global_size is equal to local_size, which means
  ///  there is only one workgroup.
  ///
  /// \param IsLastLocs is an output list of stack variable pointers
  /// holding the 'is last iteration' predicate values for the loops
  /// associated with the region.
  bool genOCLParallelLoop(WRegionNode *W,
                          SmallVectorImpl<Value *> &IsLastLocs);

  /// Replace calls to "__atomic_[load/store/compare_exchange]", with calls to
  /// "__kmpc_atomic_[load/store/compare_exchange]". This involves:
  ///  * changing argument 0 to i64 (as opposed to i32/i64 depending on target).
  ///  * adding address space casts (to `address space generic`) for pointer
  ///  arguments.
  bool renameAndReplaceLibatomicCallsForSPIRV(Function *F);

  /// Generate the placeholders for the loop lower bound and upper bound.
  /// \param [in]  W             OpenMP loop region node.
  /// \param [in]  Idx           dimension number.
  /// \param [in]  AllocaBuilder IRBuilder for new alloca instructions.
  /// \param [out] LowerBnd      stack variable holding the loop's lower bound.
  /// \param [out] UpperBnd      stack variable holding the loop's upper bound.
  /// \param [out] SchedStride   stack variable holding the loop's stride.
  /// \param [out] TeamLowerBnd  stack variable holding the team's lower bound.
  /// \param [out] TeamUpperBnd  stack variable holding the team's upper bound.
  /// \param [out] TeamStride    stack variable holding the team's stride.
  /// \param [out] IsLastLoc     stack variable holding the 'is last iteration'
  ///                            predicate value.
  /// \param [out] UpperBndVal   orginal loop bound value.
  /// \param [in]  ChunkForTeams initialize TeamLowerBnd, TeamUpperBnd
  ///                            and TeamStride output values.
  void genLoopBoundUpdatePrep(WRegionNode *W, unsigned Idx,
                              IRBuilder<> &AllocaBuilder,
                              AllocaInst *&LowerBnd, AllocaInst *&UpperBnd,
                              AllocaInst *&SchedStride,
                              AllocaInst *&TeamLowerBnd,
                              AllocaInst *&TeamUpperBnd,
                              AllocaInst *&TeamStride,
                              Value *&IsLastLoc,
                              Value *&UpperBndVal,
                              bool ChunkForTeams);

  /// Generate the OCL loop bound update code.
  void genOCLLoopBoundUpdateCode(WRegionNode *W, unsigned Idx,
                                 AllocaInst *LowerBnd, AllocaInst *UpperBnd,
                                 AllocaInst *&SchedStride);

  /// Generate the loop update code for DistParLoop under OpenCL.
  /// \param [in]  W             OpenMP distribute region node.
  /// \param [in]  Idx           dimension number.
  /// \param [in]  LowerBnd      stack variable holding the loop's lower bound.
  /// \param [in]  UpperBnd      stack variable holding the loop's upper bound.
  /// \param [in]  TeamLowerBnd  stack variable holding the team's lower bound.
  /// \param [in]  TeamUpperBnd  stack variable holding the team's upper bound.
  /// \param [in]  TeamStride    stack variable holding the team's stride.
  /// \param [in]  DistSchedKind team schedule kind.
  /// \param [out] TeamLB        team's lower bound value.
  /// \param [out] TeamUB        team's upper bound value.
  /// \param [out] TeamST        team's stride value.
  void genOCLDistParLoopBoundUpdateCode(
      WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd, AllocaInst *UpperBnd,
      AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
      AllocaInst *TeamStride, WRNScheduleKind DistSchedKind,
      Instruction *&TeamLB, Instruction *&TeamUB, Instruction *&TeamST);

  /// \breif Generate the OCL loop scheduling code.
  /// \param [in] W             OpenMP loop region node.
  /// \param [in] Idx           dimension number.
  /// \param [in] LowerBnd      stack variable holding the loop's lower bound.
  /// \param [in] UpperBnd      stack variable holding the loop's upper bound.
  /// \param [in] SchedStride stack variable holding the dispatch loop's stride.
  /// \param [in] TeamLowerBnd  stack variable holding the team's lower bound.
  /// \param [in] TeamUpperBnd  stack variable holding the team's upper bound.
  /// \param [in] TeamStride    stack variable holding the team's stride.
  /// \param [in] UpperBndVal   original loop upper bound value.
  /// \param [in] IsLastLoc     stack variable holding the 'is last iteration'
  ///                           predicate value.
  /// \param [in] GenDispLoop   create team dispatch loop.
  /// \param [in] TeamLB        team's lower bound value.
  /// \param [in] TeamUB        team's upper bound value.
  /// \param [in] TeamST        team's stride value.
  void
  genOCLLoopPartitionCode(WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd,
                          AllocaInst *UpperBnd, AllocaInst *SchedStride,
                          AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
                          AllocaInst *TeamStride, Value *UpperBndVal,
                          Value *IsLastLoc, bool GenTeamDistDispatchLoop,
                          Instruction *TeamLB, Instruction *TeamUB,
                          Instruction *TeamST);

  // Generate dispatch loop for static chunk.
  /// \param [in] L               loop.
  /// \param [in] LoadLB          loop lower bound value.
  /// \param [in] LoadUB          loop upper bound value.
  /// \param [in] LowerBnd        stack variable holding the loop's lower bound.
  /// \param [in] UpperBnd        stack variable holding the loop's upper bound.
  /// \param [in] UpperBndVal     original loop upper bound value.
  /// \param [in] SchedStride stack variable holding the dispatch loop's stride.
  /// \param [in] LoopExitBB         loop exit basic block.
  /// \param [in] StaticInitBB basic block where the top test expression stays.
  /// \param [in] LoopRegionExitBB   region exit basic block.
  Loop *genDispatchLoopForStatic(Loop *L, LoadInst *LoadLB, LoadInst *LoadUB,
                                 AllocaInst *LowerBnd, AllocaInst *UpperBnd,
                                 Value *UpperBndVal, AllocaInst *SchedStride,
                                 BasicBlock *LoopExitBB,
                                 BasicBlock *StaticInitBB,
                                 BasicBlock *LoopRegionExitBB);

  // Generate dispatch loop for teams distriubte.
  /// \param [in] L loop.
  /// \param [in] TeamLB       team's lower bound value.
  /// \param [in] TeamUB       team's upper bound value.
  /// \param [in] TeamST       team's stride value.
  /// \param [in] TeamLowerBnd stack variable holding the team's lower bound.
  /// \param [in] TeamUpperBnd stack variable holding the team's upper bound.
  /// \param [in] TeamStride   stack variable holding the team's stride.
  /// \param [in] UpperBndVal      original loop upper bound value.
  /// \param [in] LoopExitBB       loop exit basic block.
  /// \param [in] LoopRegionExitBB basic block containing top test expression.
  /// \param [in] TeamInitBB       team initialization basic block.
  /// \param [in] TeamExitBB       team exit basic block.
  /// \param [in] TeamExitBBSplit  split point for TeamExitBB; all instructions
  ///                              from the beginning of the block up to
  ///                              and including this instruction must
  ///                              remain inside the team distribute loop.
  ///                              If it is nullptr, then all instructions
  ///                              from TeamExitBB will be outside of the loop.
  Loop *genDispatchLoopForTeamDistribute(
      Loop *L, Instruction *TeamLB, Instruction *TeamUB, Instruction *TeamST,
      AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
      AllocaInst *TeamStride, Value *UpperBndVal, BasicBlock *LoopExitBB,
      BasicBlock *LoopRegionExitBB, BasicBlock *TeamInitBB,
      BasicBlock *TeamExitBB, Instruction *TeamExitBBSplit);

  /// Initialize the incoming array Arg with the constant Idx.
  void initArgArray(SmallVectorImpl<Value *> *Arg, unsigned Idx);

  /// If the given region \p W represent a multi-level loop nest, then
  /// the method returns \p TargetScheduleKind, otherwise, it returns
  /// \p SchedKind.
  WRNScheduleKind getSchedKindForMultiLevelLoops(
      WRegionNode *W, WRNScheduleKind ScheduleKind,
      WRNScheduleKind TargetScheduleKind);

#if 0
  /// Return original global variable if the value Orig is the return value
  /// of a fence call.
  static Value *getRootValueFromFenceCall(Value *Orig);
#endif

  /// Clang inserts fence acquire/release instructions for some constructs
  /// (atomic, critical, single, master, barrier and taskwait).
  /// For GPU compilation, we must remove such fence instructions because
  /// they are unsupported by SPIRV and OpenCL.  This routine does that.
  bool removeCompilerGeneratedFences(WRegionNode *W);

  /// For the given region \p W, find exit block of the loop
  /// identified by index \p Idx.
  BasicBlock *getLoopExitBB(WRegionNode *W, unsigned Idx = 0);

  /// Insert artificial uses for arguments of some clauses
  /// of the given region before the region.
  bool promoteClauseArgumentUses(WRegionNode *W);

  /// Get substrings from the "openmp-variant" string attribute. Supports the
  /// adjust_args(need_device_ptr:...) and append_args(interop(...)) clauses.
  StringRef getVariantInfo(WRegionNode *W, CallInst *BaseCall,
                           StringRef &MatchConstruct, uint64_t &DeviceArchs,
                           llvm::Optional<uint64_t> &InteropPositionOut,
                           StringRef &NeedDevicePtrStr, StringRef &InteropStr);

  /// Get substrings from the "openmp-variant" string attribute to support
  /// the TARGET VARIANT DISPATCH construct
  StringRef getVariantInfo(WRegionNode *W, CallInst *BaseCall,
                           StringRef &MatchConstruct, uint64_t &DeviceArchs,
                           llvm::Optional<uint64_t> &InteropPositionOut);

  /// Emit code to get device pointers for variant dispatch
  void getAndReplaceDevicePtrs(WRegionNode *W, CallInst *VariantCall);

  /// Emit dispatch code for the "target variant dispatch" construct
  bool genTargetVariantDispatchCode(WRegionNode *W);

  /// Emit code to handle need_device_ptr for dispatch
  void processNeedDevicePtr(WRegionNode *W, CallInst *VariantCall,
                            StringRef &NeedDevicePtrStr);

  /// Emit code to handle depend clause for dispatch
  void genDependForDispatch(WRegionNode *W, CallInst *VariantCall);

  /// Emit code for the OMP5.1 \b dispatch construct
  bool genDispatchCode(WRegionNode *W);

  /// Emit Interop code for the "interop" construct
  bool genInteropCode(WRegionNode* W);

  /// Replace loop construct with the mapped directive in IR
  bool replaceGenericLoop(WRegionNode *W);

  /// The given \p W region is one of the kinds allowing internal normalized
  /// upper bound clause (e.g. "omp parallel for").
  /// For the given \p W region try to find the enclosing "omp target"
  /// region and see if the normalized upper bound value may be computed
  /// before the "omp target" region. If it is possible, this method
  /// will setup ND-range information for the "omp target" region.
  /// There are additional limitations to when we actually create
  /// ND-range information (see implementation for details).
  bool constructNDRangeInfo(WRegionNode *W);

  /// If the given region is an OpenMP loop construct with collapse
  /// clause, then the method will collapse the loop nest accordingly.
  /// Otherwise, it will do nothing.
  bool collapseOmpLoops(WRegionNode *W);

  /// If the given region is an OpenMP loop transformation tile construct,
  /// then the method will tile the loop nest accordingly.
  /// Otherwise, it will do nothing.
  bool tileOmpLoops(WRegionNode *W);

  /// For SPIR-V target propagate simdlen() from SIMD loops
  /// to the enclosing target region. If there are multiple
  /// SIMD loops with different simdlen() values, then the minimum
  /// value will be propagated. During the propagation, simdlen()
  /// values not equal to 8, 16 or 32 are ignored.
  /// The propagated value will be used to specify SPIR-V widening
  /// width for the outlined target region.
  void propagateSPIRVSIMDWidth() const;

  /// The given loop region \p WL is enclosed into "omp target" region \p WT.
  /// \p NDRangeDims specifies "known" tripcount(s) for the loop(s)
  /// associated with \p WL (NDRangeDims[0] - tripcount for the outermost loop).
  /// The tripcounts are known in the sense that they may be computed
  /// before the "omp target" region. The method sets QUAL_OMP_OFFLOAD_NDRANGE
  /// clause for \p WT listing the known tripcount(s), and also sets
  /// QUAL_OMP_OFFLOAD_KNOWN_NDRANGE for \p WL.
  void setNDRangeClause(
      WRegionNode *WT, WRegionNode *WL, ArrayRef<Value *> NDRangeDims,
      ArrayRef<Type *> NDRangeTypes) const;

  /// Checks if the given OpenMP loop region should use SPIR partitioning
  /// with known loop(s) bounds and if it is profitable.
  /// Comparing to fixupKnownNDRange() the checks are done for loops
  /// that may not have QUAL_OMP_OFFLOAD_KNOWN_NDRANGE setup yet.
  /// This method should be used by both loop collapsing and
  /// fixupKnownNDRange(), so that their behavior is synchronized
  /// regarding the loop paritioning and the actual specific vs default
  /// ND-range used for the kernel invocation.
  bool shouldNotUseKnownNDRange(WRegionNode *W) const;

  /// Checks if the given OpenMP loop region may use SPIR paritioning
  /// with known loop(s) bounds and if it is profitable.
  /// It analyzes only QUAL_OMP_OFFLOAD_KNOWN_NDRANGE loops, which means
  /// that the loops' bounds may be computed before the enclosing target region.
  /// If known loop bounds may/must not be used, then the routine deletes
  /// QUAL_OMP_OFFLOAD_KNOWN_NDRANGE from the loop region, and also
  /// deletes QUAL_OMP_OFFLOAD_NDRANGE from the enclosing target region.
  bool fixupKnownNDRange(WRegionNode *W) const;

  /// Analyzes the current Function's WRegionList and sets starting
  /// ND-range dimensions for OpenMP loop regions. It also sets
  /// NDRangeDistributeDim for target regions, when needed.
  void assignParallelDimensions() const;

  /// Add range metadata to OpenMP API calls inside the current function for
  /// which result's range is known - omp_get_num_threads, omp_get_thread_num,
  /// omp_get_num_teams, omp_get_team_num.
  bool addRangeMetadataToOmpCalls() const;

  // If the given teams region \p WT has reduction clauses, then
  // set HasTeamsReduction attribute for the enclosing target region
  // (if any).
  void updateKernelHasTeamsReduction(const WRNTeamsNode *WT) const;

  /// Remove all clauses from the given work region and all nested regions
  /// except SIMD. SIMD directives remain unchanged in IR, but SIMD WRNs are
  /// updated to have null entry/exit directives.
  /// Return true if IR has been updated and false otherwise. Upon return
  /// FoundSIMD is set to true if there were any SIMD directives found and to
  /// false otherwise.
  bool removeClausesFromNestedRegionsExceptSIMD(WRegionNode *W,
                                                bool &FoundSIMD) const;

  /// Recompute insertion points for barriers that need to be inserted after
  /// parallel regions inside the given target region \p W inside outlined
  /// function \p KernelF for the case when there are no instructions with side
  /// effects outside of the parallel regions. Return true if barriers need to
  /// be inserted with insertion points added to \p InsertBarrierAt or false
  /// otherwise.
  bool needBarriersAfterParallel(
      WRegionNode *W, Function *KernelF,
      SmallDenseMap<Instruction *, bool> &InsertBarrierAt);
};

} /// namespace vpo
#if INTEL_CUSTOMIZATION

// Traits of WRegionNode for OptReportBuilder.
template <> struct OptReportTraits<vpo::WRegionNode> {
  using ObjectHandleTy = std::pair<vpo::WRegionNode &, vpo::WRegionListTy &>;

  static OptReport getOptReport(const ObjectHandleTy &Handle) {
    return cast_or_null<MDTuple>(
        Handle.first.getEntryDirective()->getMetadata(OptReportTag::Root));
  }

  static void setOptReport(const ObjectHandleTy &Handle, OptReport OR) {
    assert(OR && "eraseOptReport method should be used to remove OptReport");
    Handle.first.getEntryDirective()->setMetadata(OptReportTag::Root, OR.get());
  }

  static void eraseOptReport(const ObjectHandleTy &Handle) {
    Handle.first.getEntryDirective()->setMetadata(OptReportTag::Root, nullptr);
  }

  static DebugLoc getDebugLoc(const ObjectHandleTy &Handle) {
    return Handle.first.getEntryDirective()->getDebugLoc();
  }

  static Optional<std::string> getOptReportTitle(const ObjectHandleTy &Handle) {
    return "OMP " + Handle.first.getSourceName().upper();
  }

  static OptReport getOrCreatePrevOptReport(const ObjectHandleTy &Handle,
                                            const OptReportBuilder &Builder) {
    auto &W = Handle.first;
    vpo::WRegionNode *PrevSiblingW = nullptr;

    if (auto *Parent = W.getParent())
      for (auto *Child : reverse(Parent->getChildren())) {
        if (Child == &W)
          break;
        PrevSiblingW = Child;
      }
    else
      for (auto *Region : Handle.second) {
        if (Region == &W)
          break;
        if (!Region->getParent())
          PrevSiblingW = Region;
      }

    if (!PrevSiblingW || !PrevSiblingW->getEntryDirective())
      return nullptr;

    return Builder(*PrevSiblingW, Handle.second).getOrCreateOptReport();
  }

  static OptReport getOrCreateParentOptReport(const ObjectHandleTy &Handle,
                                              const OptReportBuilder &Builder) {
    auto &W = Handle.first;

    // Attach to the parent region, if it exists.
    if (auto *Dest = W.getParent())
      return Builder(*Dest, Handle.second).getOrCreateOptReport();

    // Attach to the Function, otherwise.
    if (Function *Dest = W.getEntryBBlock()->getParent())
      return Builder(*Dest).getOrCreateOptReport();

    llvm_unreachable("Failed to find a parent.");
  }

  using ChildNodeTy = vpo::WRegionNode;
  using ChildHandleTy = typename OptReportTraits<ChildNodeTy>::ObjectHandleTy;
  using NodeVisitorTy = function_ref<void(ChildHandleTy)>;
  static void traverseChildNodesBackward(const ObjectHandleTy &Handle,
                                         NodeVisitorTy Func) {}
};
#endif // INTEL_CUSTOMIZATION
} // namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORM_H
#endif // INTEL_COLLAB
