#if INTEL_COLLAB // -*- C++ -*-
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
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

#include <queue>

namespace llvm {

namespace vpo {

/// opencl address space.
enum AddressSpace {
  ADDRESS_SPACE_PRIVATE = 0,
  ADDRESS_SPACE_GLOBAL = 1,
  ADDRESS_SPACE_CONSTANT = 2,
  ADDRESS_SPACE_LOCAL = 3,
  ADDRESS_SPACE_GENERIC = 4
};

typedef SmallVector<WRegionNode *, 32> WRegionListTy;

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
                     const TargetLibraryInfo *TLI, AliasAnalysis *AA, int Mode,
#if INTEL_CUSTOMIZATION
                     OptReportVerbosity::Level ORVerbosity,
#endif  // INTEL_CUSTOMIZATION
                     OptimizationRemarkEmitter &ORE,
                     unsigned OptLevel = 2, bool SwitchToOffload = false,
                     bool DisableOffload = false)
      : MT(MT), F(F), WI(WI), DT(DT), LI(LI), SE(SE), TTI(TTI), AC(AC),
        TLI(TLI), AA(AA), Mode(Mode),
        TargetTriple(F->getParent()->getTargetTriple()),
#if INTEL_CUSTOMIZATION
        ORVerbosity(ORVerbosity),
#endif  // INTEL_CUSTOMIZATION
        ORE(ORE), OptLevel(OptLevel), SwitchToOffload(SwitchToOffload),
        DisableOffload(DisableOffload),
        IdentTy(nullptr), TidPtrHolder(nullptr), BidPtrHolder(nullptr),
        KmpcMicroTaskTy(nullptr), KmpRoutineEntryPtrTy(nullptr),
        KmpTaskTTy(nullptr), KmpTaskTRedTy(nullptr),
        KmpTaskDependInfoTy(nullptr) {

#if INTEL_CUSTOMIZATION
        // Set up Builder for generating remarks using Loop Opt Report
        // framework (under -qopt-report).
        LORBuilder.setup(F->getContext(), ORVerbosity);
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
  void addBranchToEndDirective(WRegionNode *W);

  /// Top level interface for parallel and prepare transformation
  bool paroptTransforms();

  bool addNormUBsToParents(WRegionNode* W);

  bool isModeOmpNoCollapse() { return Mode & vpo::OmpNoCollapse; }
  bool isModeOmpSimt() { return Mode & vpo::OmpSimt; }

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

  AliasAnalysis *AA;

  /// Paropt compilation mode
  int Mode;

  /// Target triple that we are compiling for.
  Triple TargetTriple;

#if INTEL_CUSTOMIZATION
  /// Verbosity level for generating remarks using Loop Opt Report framework (under -qopt-report).
  OptReportVerbosity::Level ORVerbosity;

  /// Builder for generating remarks using Loop Opt Report framework (under -qopt-report).
  LoopOptReportBuilder LORBuilder;
#endif  // INTEL_CUSTOMIZATION

  /// Optimization remark emitter.
  OptimizationRemarkEmitter &ORE;

  /// Optimization level.
  unsigned OptLevel;

  /// Offload compilation mode.
  bool SwitchToOffload;

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
    /// The number of pointers passed to the runtime library.
    unsigned NumberOfPtrs = 0u;
    explicit TgDataInfo() {}
    void clearArrayInfo() {
      BaseDataPtrs = nullptr;
      DataPtrs = nullptr;
      DataSizes = nullptr;
      DataMapTypes = nullptr;
      NumberOfPtrs = 0u;
    }
    bool isValid() {
      return BaseDataPtrs && DataPtrs && DataSizes && DataMapTypes &&
             NumberOfPtrs;
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
    return
        VPOAnalysisUtils::isTargetSPIRV(F->getParent()) &&
        hasOffloadCompilation();
  }

  /// Use the WRNVisitor class (in WRegionUtils.h) to walk the
  /// W-Region Graph in DFS order and perform outlining transformation.
  /// \param[out] NeedTID : 'true' if any W visited has W->needsTID()==true
  /// \param[out] NeedBID : 'true' if any W visited has W->needsBID()==true
  void gatherWRegionNodeList(bool &NeedTID, bool &NeedBID);

  /// Generate code for private variables
  bool genPrivatizationCode(WRegionNode *W);

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

  /// Generate code for firstprivate variables
  bool genFirstPrivatizationCode(WRegionNode *W);

  /// Generate code for lastprivate variables
  bool genLastPrivatizationCode(WRegionNode *W, BasicBlock *IfLastIterBB,
                                Instruction *OMPLBForChunk = nullptr,
                                Instruction *BranchToNextChunk = nullptr,
                                Instruction *OMPZtt = nullptr);

  /// Generate destructor calls for [first|last]private variables
  bool genDestructorCode(WRegionNode *W);

  /// Extract the type and size of local Alloca to be created to privatize
  /// \p OrigValue.
  /// \param [in] OrigValue Input Value
  /// \param [out] ElementType Type of one element
  /// \param [out] NumElements Number of elements, in case \p OrigValue is
  /// an array, \b nullptr otherwise.
  /// \param [out] AddrSpace Address space of the input value object.
  static void getItemInfoFromValue(Value *OrigValue, Type *&ElementType,
                                   Value *&NumElements, unsigned &AddrSpace);

  /// Extract the type and size of local Alloca to be created to privatize
  /// \p I.
  /// \param [in] I Input Item
  /// \param [out] ElementType Type of one element
  /// \param [out] NumElements Number of elements, in case \p OrigValue is
  /// an array, \b nullptr otherwise.
  /// \param [out] AddrSpace Address space of the input item object.
  static void getItemInfo(Item *I, Type *&ElementType, Value *&NumElements,
                          unsigned &AddrSpace);

  /// Generate an optionally addrspacecast'ed pointer Value for an array
  /// of Type \p ElementType, size \p NumElements, name \p VarName.
  /// \p NumElements can be null for one element.
  /// If new instructions need to be generated, they will be inserted
  /// before \p InsertPt.
  /// \p AllocaAddrSpace specifies address space in which the memory
  /// for the privatized variable needs to be allocated. If it is
  /// llvm::None, then the address space matches the default alloca's
  /// address space, as specified by DataLayout. Note that some address
  /// spaces may require allocating the private version of the variable
  /// as a GlobalVariable, not as an AllocaInst.
  /// If \p ValueAddrSpace does not match llvm::None,
  /// then the generated Value will be immediately addrspacecast'ed
  /// and the generated AddrSpaceCastInst or AddrSpaceCast constant
  /// expression will be returned as a result.
  static Value *genPrivatizationAlloca(
      Type *ElementType, Value *NumElements,
      Instruction *InsertPt, const Twine &VarName = "",
      llvm::Optional<unsigned> AllocaAddrSpace = llvm::None,
      llvm::Optional<unsigned> ValueAddrSpace = llvm::None);

  /// Generate an optionally addrspacecast'ed pointer Value for the local copy
  /// of \p OrigValue, with \p NameSuffix appended at the end of its name.
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
  static Value *genPrivatizationAlloca(
      Value *OrigValue,
      Instruction *InsertPt,
      const Twine &NameSuffix = "",
      llvm::Optional<unsigned> AllocaAddrSpace = llvm::None,
      bool PreserveAddressSpace = true);

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
  static Value *genPrivatizationAlloca(
      Item *I, Instruction *InsertPt,
      const Twine &NameSuffix = "",
      llvm::Optional<unsigned> AllocaAddrSpace = llvm::None,
      bool PreserveAddressSpace = true);

  /// Returns address space that should be used for privatizing variables
  /// referenced in PRIVATE clauses of the given region \p W.
  /// If the return value is llvm::None, then the address space
  /// should be equal to default alloca address space, as defined
  /// by DataLayout.
  llvm::Optional<unsigned> getPrivatizationAllocaAddrSpace(
      const WRegionNode *W) const;

  /// Replace the variable with the privatized variable
  void genPrivatizationReplacement(WRegionNode *W, Value *PrivValue,
                                   Value *NewPrivInst);

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

  /// For array [section] reduction finalization loop, compute the base address
  /// of the source and destination arrays, number of elements, and the type of
  /// destination array elements.
  /// \param [in] ReductionItem Reduction Item.
  /// \param [in] AI Local Value for the reduction operand.
  /// \param [in] OldV Original reduction operand Value.
  /// \param [in] InsertPt Insert point for any Instructions to be inserted.
  /// \param [in] Builder IRBuilder using InsertPt for any new Instructions.
  /// \param [out] NumElements Number of elements in the array [section].
  /// \param [out] SrcArrayBegin Base address of the local reduction array.
  /// \param [out] DestArrayBegin Starting address of the original reduction
  /// array [section].
  /// \param [out] DestElementTy Type of each element of the array [section].
  void genAggrReductionFiniSrcDstInfo(const ReductionItem &RedI, Value *AI,
                                      Value *OldV, Instruction *InsertPt,
                                      IRBuilder<> &Builder, Value *&NumElements,
                                      Value *&SrcArrayBegin,
                                      Value *&DestArrayBegin,
                                      Type *&DestElementTy);

  /// Initialize `Size`, `ElementType`, `Offset` and `BaseIsPointer` fields for
  /// ArraySectionInfo of the map/reduction item \p CI. It may need to emit some
  /// Instructions, which is done \b before \p InsertPt.
  void computeArraySectionTypeOffsetSize(Item &CI, Instruction *InsertPt);

  /// Transform all array sections in \p W region's map clauses
  /// into map chains. New instructions to compute parameters of
  /// the corresponding map chains are inserted \b before \p InsertPt.
  /// After the transformation a map clause with an array section
  /// will contain a single map chain element, which is dynamically
  /// allocated by this method. The MapItem destructor is responsible
  /// for deallocating this map chain element.
  void genMapChainsForMapArraySections(WRegionNode *W, Instruction *InsertPt);

  /// Return the Value to replace the occurrences of the original clause
  /// operand inside the body of the associated WRegion. It may need to emit
  /// some Instructions, which is done \b before \p InsertPt.
  static Value *getClauseItemReplacementValue(Item *I, Instruction *InsertPt);

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
  /// reduction update code.
  bool genReductionFini(WRegionNode *W, ReductionItem *RedI, Value *OldV,
                        Instruction *InsertPt, DominatorTree *DT);

  /// Generate the reduction initialization code for Min/Max.
  Value *genReductionMinMaxInit(ReductionItem *RedI, Type *Ty, bool IsMax);

  /// Generate the reduction intialization instructions.
  Value *genReductionScalarInit(ReductionItem *RedI, Type *ScalarTy);

  /// Generate the reduction code for reduction clause.
  bool genReductionCode(WRegionNode *W);

  /// Prepare the empty basic block for the array
  /// reduction or firstprivate initialization.
  void createEmptyPrvInitBB(WRegionNode *W, BasicBlock *&RedBB);

  /// Prepare the empty basic block for the array
  /// reduction or lastprivate update.
  /// If \p W is a loop region, and the loop has ZTT check,
  /// then the new block will be inserted at the exit block
  /// of the loop, unless \p HonorZTT is false.  Otherwise,
  /// the new block will be inserted at the region's exit
  /// block
  void createEmptyPrivFiniBB(WRegionNode *W, BasicBlock *&RedEntryBB,
                             bool HonorZTT = true);

  /// Generate the reduction update instructions for min/max.
  Value* genReductionMinMaxFini(ReductionItem *RedI, Value *Rhs1, Value *Rhs2,
                             Type *ScalarTy, IRBuilder<> &Builder, bool IsMax);

  /// Generate the reduction update instructions.
  /// Returns true iff critical section is required around the generated
  /// reduction update code.
  bool genReductionScalarFini(
      WRegionNode *W, ReductionItem *RedI,
      Value *ReductionVar, Value *ReductionValueLoc,
      Type *ScalarTy, IRBuilder<> &Builder);

  /// Generate the reduction initialization/update for array.
  /// Returns true iff critical section is required around the generated
  /// reduction update code. The method always returns false, when
  /// IsInit is true.
  bool genRedAggregateInitOrFini(WRegionNode *W, ReductionItem *RedI,
                                 Value *AI, Value *OldV,
                                 Instruction *InsertPt, bool IsInit,
                                 DominatorTree *DT);

  /// Generate the reduction fini code for bool and/or.
  Value *genReductionFiniForBoolOps(ReductionItem *RedI, Value *Rhs1,
                                    Value *Rhs2, Type *ScalarTy,
                                    IRBuilder<> &Builder, bool IsAnd);
  /// @}

  /// Generate the firstprivate initialization code.
  void genFprivInit(FirstprivateItem *FprivI, Instruction *InsertPt);

  /// Utility for last private update or copyprivate code generation.
  void genLprivFini(Value *NewV, Value *OldV, Instruction *InsertPt);
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

  /// Generate the code to replace the variables in the task loop with
  /// the thunk field dereferences
  bool genTaskLoopInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                           StructType *&KmpSharedTy, Value *&LBPtr,
                           Value *&UBPtr, Value *&STPtr, Value *&LastIterGep,
                           bool isLoop = true);
  bool genTaskInitCode(WRegionNode *W, StructType *&KmpTaskTTWithPrivatesTy,
                       StructType *&KmpSharedTy, Value *&LastIterGep);

  /// Generate the call __kmpc_omp_task_alloc, __kmpc_taskloop and the
  /// corresponding outlined function
  bool genTaskGenericCode(WRegionNode *W, StructType *KmpTaskTTWithPrivatesTy,
                          StructType *KmpSharedTy, Value *LBPtr, Value *UBPtr,
                          Value *STPtr, bool isLoop = true);

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

  /// Set up the mapping between the variables (firstprivate,
  /// lastprivate, reduction and shared) and the counterparts in the thunk.
  AllocaInst *genTaskPrivateMapping(WRegionNode *W, StructType *KmpSharedTy);

  /// Initialize the data in the shared data area inside the thunk
  void genSharedInitForTaskLoop(WRegionNode *W, AllocaInst *Src, Value *Dst,
                                StructType *KmpSharedTy,
                                StructType *KmpTaskTTWithPrivatesTy,
                                Function *DestrThunk, Instruction *InsertPt);

  /// Save the loop lower upper bound, upper bound and stride for the use
  /// by the call __kmpc_taskloop
  void genLoopInitCodeForTaskLoop(WRegionNode *W, Value *&LBPtr, Value *&UBPtr,
                                  Value *&STPtr);

  /// Generate the outline function of reduction initilaization
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
  StructType *genKmpTaskTWithPrivatesRecordDecl(WRegionNode *W,
                                                StructType *&KmpSharedTy,
                                                StructType *&KmpPrivatesTy);

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

  /// Reset the expression value in IsDevicePtr clause to be empty.
  void resetValueInIsDevicePtrClause(WRegionNode *W);

  /// Set the value in num_teams, thread_limit and num_threads
  /// clauses to be empty.
  void resetValueInNumTeamsAndThreadsClause(WRegionNode *W);

  /// Reset the value in the Map clause to be empty.
  void resetValueInMapClause(WRegionNode *W);

  /// Reset the value of \p V in OpenMP clauses of \p W to be empty.
  void resetValueInOmpClauseGeneric(WRegionNode *W, Value *V);

  /// Generate the code for the directive omp target
  bool genTargetOffloadingCode(WRegionNode *W);

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
                            bool hasRuntimeEvaluationCaptureSize);

  /// Utilities to construct the assignment to the base pointers, section
  /// pointers and size pointers if the flag hasRuntimeEvaluationCaptureSize is
  /// true.
  void genOffloadArraysInitUtil(IRBuilder<> &Builder, Value *BasePtr,
                                Value *SectionPtr, Value *Size,
                                TgDataInfo *Info,
                                SmallVectorImpl<Constant *> &ConstSizes,
                                unsigned &Cnt,
                                bool hasRuntimeEvaluationCaptureSize);

  /// Fixup references generated for global variables in OpenMP
  /// clauses for targets supporting non-default address spaces.
  /// This fixup has to be done as soon as possible after FE.
  bool canonicalizeGlobalVariableReferences(WRegionNode *W);

  // If the incoming data is global variable, create a stack variable
  // and replace the global variable with the stack variable.
  bool genGlobalPrivatizationLaunderIntrin(WRegionNode *W);

  /// build the CFG for if clause.
  void buildCFGForIfClause(Value *Cmp, Instruction *&ThenTerm,
                           Instruction *&ElseTerm, Instruction *InsertPt);

  /// Generate the sizes and map type flags for the given map type, map
  /// modifier and the expression V.
  /// \param [in]     W               incoming WRegionNode.
  /// \param [in]     V               base pointer.
  /// \param [out]    ConstSizes      array of size information.
  /// \param [out]    MapTypes        array of map types.
  /// \param [out]    hasRuntimeEvaluationCaptureSize
  ///                 size cannot be determined at compile time.
  void genTgtInformationForPtrs(WRegionNode *W, Value *V,
                                SmallVectorImpl<Constant *> &ConstSizes,
                                SmallVectorImpl<uint64_t> &MapTypes,
                                bool &hasRuntimeEvaluationCaptureSize);

  /// Generate multithreaded for a given WRegion
  bool genMultiThreadedCode(WRegionNode *W);

  /// Generate code for master/end master construct and update LLVM
  /// control-flow and dominator tree accordingly
  bool genMasterThreadCode(WRegionNode *W, bool IsTargetSPIRV);

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

  /// Return true if the program is compiled at the offload mode.
  bool hasOffloadCompilation() const {
    return ((Mode & OmpOffload) || SwitchToOffload);
  }

  /// Finds the alloc stack variables where the tid stores.
  void getAllocFromTid(CallInst *Tid);

  /// Finds the function pointer type for the function
  /// void (*kmpc_micro)(kmp_int32 *global_tid, kmp_int32 *bound_tid, ...)
  FunctionType* getKmpcMicroTaskPointerTy();

  /// The data structure which builds the map between the
  /// alloc/tid and the uses instruction in the WRegion.
  SmallDenseMap<Instruction *, std::vector<Instruction *> > IdMap;

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

  /// Emits an if-then branch using \p IsLastVal and sets \p IfLastIterOut to
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
  /// \param [in] IsLastVal A stack variable which is non-zero if the current
  /// iteration is the last one.
  /// \param [out] IfLastIterOut The BasicBlock for when the last iteration
  /// check is true.
  /// \param [in] InsertBefore If not null, the branch is inserted before it.
  /// Otherwise, the branch is inserted before \p W's exit BB.
  ///
  /// \returns \b true if the branch is emitted, \b false otherwise.
  ///
  /// The branch is not emitted if \p W has no Linear or Lastprivate var.
  bool genLastIterationCheck(WRegionNode *W, Value *IsLastVal,
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

  /// Check whether a given construct is supported in CSA.
  bool isSupportedOnCSA(WRegionNode *W);
#endif  // INTEL_FEATURE_CSA
#endif  // INTEL_CUSTOMIZATION

  /// Insert a flush call
  bool genFlush(WRegionNode *W);

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

  /// @}

  /// Rename operands of various clauses by replacing them with a
  /// store-then-load, and adding operand-address pair to the entry directive.
  /// This renaming is done to prevent CSE/Instcombine transformations which
  /// break OpenMP semantics by combining/recomputing bitcasts/GEPs across
  /// region boundaries.
  /// This renaming is done for operands to private, firstprivate, lastprivate,
  /// reduction, shared, and map clauses.
  ///
  /// This renaming is done in the vpo-paropt-prepare phase, and is undone
  /// in the vpo-paropt-restore phase beofore the vpo-paropt transformation
  /// pass.
  ///
  /// The IR before and after this renaming looks like:
  ///
  /// \code
  ///            Before                   |            After
  ///          ---------------------------+---------------------------
  ///                                     |   store i32* %y, i32** %y.addr
  ///                                     |
  ///   %1 = begin_region[... %y...]      |   %1 = begin_region[... %y...
  ///                                     |        "QUAL.OMP.OPERAND.ADDR"
  ///                                     |         (i32* %y,i32** %y.addr)]
  ///                                     |
  ///                                     |   %y1 = load i32*, i32** %y.addr
  ///                                     |
  ///   ...                               |   ...
  ///   <%y used inside the region>       |   <%y1 used inside the region>
  ///                                     |
  ///                                     |
  ///   end_region(%1)                    |   end_region(%1)
  /// \endcode
  ///
  /// In the region, `%y1` is used to replace all uses of `$y`. If there is no
  /// use of `%y` inside the region, then the load `%y` is not emitted.
  /// The operand-addr pair in the auxiliary clause `QUAL.OMP.OPERAND.ADDR` is
  /// used to undo the renaming in the VPORestoreOperandsPass.
  /// \see VPOUtils::restoreOperands() for details on how the renaming is
  /// undone and the original operands are restored.
  bool renameOperandsUsingStoreThenLoad(WRegionNode *W);

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
  /// If \p W is for a target construct, `QUAL.OMP.MAP.TO` is used instead of
  /// `QUAL.OMP.SHARED` for `%size2`.
  /// \see WRegionUtils::collectNonPointerValuesToBeUsedInOutlinedRegion() for
  /// more details.
  bool captureAndAddCollectedNonPointerValuesToSharedClause(WRegionNode *W);

  /// Clean up the intrinsic @llvm.launder.invariant.group and replace
  /// the use of the intrinsic with the its operand.
  bool clearCodemotionFenceIntrinsic(WRegionNode *W);

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
    GT_MAP_RETURN_PARAM = 0x40,
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

  /// Returns the corresponding flag for a given map clause modifier.
  uint64_t getMapTypeFlag(MapItem *MpI,
                         bool AddrIsTargetParamFlag,
                         bool IsFirstComponentFlag);

  /// Create a pointer, store address of \p V to the pointer, and replace uses
  /// of \p V with a load from that pointer.
  ///
  /// \code
  ///   %v = alloca i32
  ///   ...
  ///   %v.addr = alloca i32*
  ///   ...
  ///   store i32* %v, i32** %v.addr
  ///   ; <InsertPtForStore>
  ///
  ///   +- <EntryBB>:
  ///   | ...
  ///   | %0 = llvm.region.entry() [... "PRIVATE" (i32* %v) ]
  ///   | ...
  ///   | %v1 = load i32*, i32** %v.addr
  ///   +-
  ///   ...
  ///   ; Replace uses of %v with %v1
  ///   ...
  /// \endcode
  ///
  /// If \p InsertLoadInBeginningOfEntryBB is \b true, the load `%v1` is
  /// inserted in the beginning on EntryBB (BBlock containing `%0`), and the
  /// use of `%v` in `%0` is also replaced with `%v1`. Otherwise, by default,
  /// `v1` is inserted at the end of EntryBB.
  ///
  /// \returns the pointer where \p V is stored (`%v.addr` above).
  static Value *
  replaceWithStoreThenLoad(WRegionNode *W, Value *V,
                           Instruction *InsertPtForStore,
                           bool InsertLoadInBeginningOfEntryBB = false);

  /// If \p I is a call to @llvm.launder.invariant.group, then return
  /// the CallInst*. Otherwise, return nullptr.
  static CallInst* isFenceCall(Instruction *I);

  /// Collect the live-in value for the phis at the loop header.
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
  void buildECs(Loop *L, PHINode *PN, EquivalenceClasses<Value *> &ECs);

  /// The utility to build the equivalence class for the value phi.
  void AnalyzePhisECs(Loop *L, Value *PV, Value *V,
                      EquivalenceClasses<Value *> &ECs,
                      SmallPtrSet<PHINode *, 16> &PhiUsers);

  /// Collect the live-out values for a given loop.
  void wrnCollectLiveOutVals(Loop &L,
                             SmallSetVector<Instruction *, 8> &LiveOutVals,
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
  /// Insert a new AllocaInst at the region's entry block.
  /// Copy the original variable value to the allocated area, iff
  /// \p IsFirstPrivate is true.
  /// Replace all uses of the original AllocaInst with the new one.
  ///
  /// Since \p V is a normalized upper bound or induction variable
  /// pointer, we do not expect it to have non-POD type
  /// neither expect it to be By-Ref.
  Value *genRegionPrivateValue(
      WRegionNode *W, Value *V, bool IsFirstPrivate = false);

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
  /// OMP canonical loop form.
  void regularizeOMPLoopImpl(WRegionNode *W, unsigned I);

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

  /// Return true if one of the region W's ancestor is OMP target
  /// construct or the function where W lies in has target declare attribute.
  bool hasParentTarget(WRegionNode *W);

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
  AllocaInst *genTgtLoopParameter(WRegionNode *W, WRegionNode *WL);

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
#endif  // INTEL_CUSTOMIZATION

  /// Guard each instruction that has a side effect with master thread id
  /// check, so that only the master thread (id == 0) in the team executes
  /// the code, then put a barrier before the start and after the end of
  /// every parallel region, so that all the threads in the team wait for
  /// the master thread, and can see its update of team shared memory.
  /// \p KernelEntryDir and \p KernelExitDir are correspondingly the entry and
  /// exit directives for the WRegion whose outlined region \p kernelF is.
  void guardSideEffectStatements(Function *KernelF,
                                 SmallPtrSetImpl<Value *> &PrivateVariables,
                                 Instruction *KernelEntryDir = nullptr,
                                 Instruction *KernelExitDir = nullptr);

  /// Replace printf() calls in \p F with _Z18__spirv_ocl_printfPU3AS2ci()
  void replacePrintfWithOCLBuiltin(Function *F);

  /// Set the kernel arguments' address space as ADDRESS_SPACE_GLOBAL.
  /// Propagate the address space from the arguments to the usage of the
  /// arguments.
  Function *finalizeKernelFunction(WRegionNode *W, Function *Fn,
                                   CallInst *&Call);

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
  bool genOCLParallelLoop(WRegionNode *W);

  /// Replace calls to "__atomic_[load/store/compare_exchange]", with calls to
  /// "__kmpc_atomic_[load/store/compare_exchange]". This involves:
  ///  * changing argument 0 to i64 (as opposed to i32/i64 depending on target).
  ///  * adding address space casts (to `address space generic`) for pointer
  ///  arguments.
  bool renameAndReplaceLibatomicCallsForSPIRV(Function *F);

  /// Generate the placeholders for the loop lower bound and upper bound.
  /// \param [in]  W            OpenMP loop region node.
  /// \param [in]  Idx          dimension number.
  /// \param [out] LowerBnd     stack variable holding the loop's lower bound.
  /// \param [out] UpperBnd     stack variable holding the loop's upper bound.
  /// \param [out] SchedStride  stack variable holding the loop's stride.
  /// \param [out] TeamLowerBnd stack variable holding the team's lower bound.
  /// \param [out] TeamUpperBnd stack variable holding the team's upper bound.
  /// \param [out] TeamStride   stack variable holding the team's stride.
  /// \param [out] UpperBndVal  orginal loop bound value.
  void genLoopBoundUpdatePrep(WRegionNode *W, unsigned Idx,
                              AllocaInst *&LowerBnd, AllocaInst *&UpperBnd,
                              AllocaInst *&SchedStride,
                              AllocaInst *&TeamLowerBnd,
                              AllocaInst *&TeamUpperBnd,
                              AllocaInst *&TeamStride, Value *&UpperBndVal);

  /// Generate the OCL loop bound update code.
  void genOCLLoopBoundUpdateCode(WRegionNode *W, unsigned Idx,
                                 AllocaInst *LowerBnd, AllocaInst *UpperBnd,
                                 AllocaInst *TeamLowerBnd,
                                 AllocaInst *TeamUpperBnd,
                                 AllocaInst *SchedStride);

  /// Generate the loop update code for DistParLoop under OpenCL.
  /// \param [in]  W            OpenMP distribute region node.
  /// \param [in]  Idx          dimension number.
  /// \param [in]  LowerBnd     stack variable holding the loop's lower bound.
  /// \param [in]  UpperBnd     stack variable holding the loop's upper bound.
  /// \param [in]  TeamLowerBnd stack variable holding the team's lower bound.
  /// \param [in]  TeamUpperBnd stack variable holding the team's upper bound.
  /// \param [in]  TeamStride   stack variable holding the team's stride.
  /// \param [out] DistSchedKind team schedule kind.
  /// \param [out] TeamLB       team's lower bound value.
  /// \param [out] TeamUB       team's upper bound value.
  /// \param [out] TeamST       team's stride value.
  void genOCLDistParLoopBoundUpdateCode(
      WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd, AllocaInst *UpperBnd,
      AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
      AllocaInst *TeamStride, WRNScheduleKind &DistSchedKind,
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
  /// \param [in] DistSchedKind team schedule kind.
  /// \param [in] TeamLB        team's lower bound value.
  /// \param [in] TeamUB        team's upper bound value.
  /// \param [in] TeamST        team's stride value.
  void
  genOCLLoopPartitionCode(WRegionNode *W, unsigned Idx, AllocaInst *LowerBnd,
                          AllocaInst *UpperBnd, AllocaInst *SchedStride,
                          AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
                          AllocaInst *TeamStride, Value *UpperBndVal,
                          WRNScheduleKind DistSchedKind, Instruction *TeamLB,
                          Instruction *TeamUB, Instruction *TeamST);

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
  Loop *genDispatchLoopForTeamDistirbute(
      Loop *L, Instruction *TeamLB, Instruction *TeamUB, Instruction *TeamST,
      AllocaInst *TeamLowerBnd, AllocaInst *TeamUpperBnd,
      AllocaInst *TeamStride, Value *UpperBndVal, BasicBlock *LoopExitBB,
      BasicBlock *LoopRegionExitBB, BasicBlock *TeamInitBB,
      BasicBlock *TeamExitBB, Instruction *TeamExitBBSplit);

  /// Initialize the incoming array Arg with the constant Idx.
  void initArgArray(SmallVectorImpl<Value *> *Arg, unsigned Idx);

  /// The compiler sets DistSchedKind to be TargetScheduleKind for the case of
  /// multi-level loop nest.
  void setSchedKindForMultiLevelLoops(WRegionNode *W,
                                      WRNScheduleKind &ScheduleKind,
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

  /// Emit dispatch code for the "target variant dispatch" construct
  bool genTargetVariantDispatchCode(WRegionNode *W);

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

  /// Marks the given region \p W as may-have-openmp-critical,
  /// if it contains "omp critical" or a call that may "invoke"
  /// "omp critical".
  void setMayHaveOMPCritical(WRegionNode *W) const;
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_TRANSFORM_H
#endif // INTEL_COLLAB
