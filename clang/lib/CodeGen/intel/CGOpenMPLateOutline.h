#if INTEL_COLLAB                                           // -*- C++ -*-
//===--- CGOpenMPLateOutline.h - OpenMP Late-Outlining --------*- C++ -*---===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
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
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This provides classes for OpenMP late-outlining code generation.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_CODEGEN_CGOPENMPLATEOUTLINE_H
#define LLVM_CLANG_LIB_CODEGEN_CGOPENMPLATEOUTLINE_H

#include "CodeGenFunction.h"
#include "CGCXXABI.h"
#include "clang/AST/StmtOpenMP.h"

namespace clang {

class OMPCaptureNoInitAttr;

namespace CodeGen {

enum OMPAtomicClause {
  OMP_read,
  OMP_write,
  OMP_update,
  OMP_capture,
  OMP_read_seq_cst,
  OMP_write_seq_cst,
  OMP_update_seq_cst,
  OMP_capture_seq_cst,
};

class OpenMPLateOutliner {
  struct ArraySectionDataTy final {
    llvm::Value *LowerBound = nullptr;
    llvm::Value *Length = nullptr;
    llvm::Value *Stride = nullptr;
    llvm::Value *VLASize = nullptr;
  };
  using ArraySectionTy = llvm::SmallVector<ArraySectionDataTy, 4>;

  // Used temporarily to build a bundle.
  StringRef BundleString;
  SmallVector<llvm::Value*, 8> BundleValues;
  void clearBundleTemps() { BundleString = ""; BundleValues.clear(); }

  struct DirectiveIntrinsicSet final {
    OpenMPDirectiveKind DKind;
    SmallVector<llvm::OperandBundleDef, 8> OpBundles;
    StringRef End;
    llvm::CallInst *CallEntry = nullptr;
    DirectiveIntrinsicSet(StringRef E, OpenMPDirectiveKind K)
          : DKind(K), End(E) {}
    void clear() { OpBundles.clear(); }
  };
  SmallVector<DirectiveIntrinsicSet, 4> Directives;
  CodeGenFunction &CGF;
  llvm::LLVMContext &C;

  // For region entry/exit implementation
  llvm::Function *RegionEntryDirective = nullptr;
  llvm::Function *RegionExitDirective = nullptr;

  // Used to insert instructions outside the region.
  llvm::Instruction *MarkerInstruction = nullptr;

  /// This class manages the building of the clause qualifier string.  An
  /// object is used in ClauseEmissionHelper to ensure the string is alive
  /// until the end of the emission.  Use it in all cases where a constant
  /// string cannot be used for the BundleString part of the clause.
  class ClauseStringBuilder final {
    SmallString<128> Str;
    StringRef Separator = ":";

    // Modifiers
    bool NonPod = false;
    bool ByRef = false;
    bool Unsigned = false;
    bool Conditional = false;
    bool ArrSect = false;
    bool Monotonic = false;
    bool NonMonotonic = false;
    bool Simd = false;
    bool Always = false;
    bool Close = false;
    bool Present = false;
    bool Cmplx = false;
    bool PtrToPtr = false;
    bool Chain = false;
    bool InScan = false;
    bool Task = false;
    bool Target = false;
    bool TargetSync = false;
    bool Prefer = false;
    bool IV = false;
    bool Typed = false;
    bool Fptr = false;

    void addSeparated(StringRef QualString) {
      Str += Separator;
      Str += QualString;
      Separator = ".";
    }

    void insertModifiers() {
      if (NonPod)
        addSeparated("NONPOD");
      if (ByRef)
        addSeparated("BYREF");
      if (Unsigned)
        addSeparated("UNSIGNED");
      if (Conditional)
        addSeparated("CONDITIONAL");
      if (ArrSect)
        addSeparated("ARRSECT");
      if (Monotonic)
        addSeparated("MONOTONIC");
      if (NonMonotonic)
        addSeparated("NONMONOTONIC");
      if (Simd)
        addSeparated("SIMD");
      if (Always)
        addSeparated("ALWAYS");
      if (Close)
        addSeparated("CLOSE");
      if (Present)
        addSeparated("PRESENT");
      if (Cmplx)
        addSeparated("CMPLX");
      if (PtrToPtr)
        addSeparated("PTR_TO_PTR");
      if (Chain)
        addSeparated("CHAIN");
      if (InScan)
        addSeparated("INSCAN");
      if (Task)
        addSeparated("TASK");
      if (Target)
        addSeparated("TARGET");
      if (TargetSync)
        addSeparated("TARGETSYNC");
      if (Prefer)
        addSeparated("PREFER");
      if (IV)
        addSeparated("IV");
      if (Typed)
        addSeparated("TYPED");
      if (Fptr)
        addSeparated("FPTR");
    }

  public:
    ClauseStringBuilder() = default;
    ClauseStringBuilder(StringRef InitStr) { Str = InitStr; }
    void setNonPod() { NonPod = true; }
    void setByRef() { ByRef = true; }
    void setUnsigned() { Unsigned = true; }
    void setConditional() { Conditional = true; }
    void setArrSect() { ArrSect = true; }
    void setMonotonic() { Monotonic = true; }
    void setNonMonotonic() { NonMonotonic = true; }
    void setSimd() { Simd = true; }
    void setAlways() { Always = true; }
    void setClose() { Close =  true; }
    void setPresent() { Present =  true; }
    void setCmplx() { Cmplx = true; }
    void setPtrToPtr() {PtrToPtr = true; }
    void setChain() { Chain = true; }
    void setInScan() { InScan = true; }
    void setTask() { Task = true; }
    void setTarget() { Target = true; }
    void setTargetSync() { TargetSync = true; }
    void setPrefer() { Prefer = true; }
    void setIV() { IV = true; }
    void setTyped() { Typed = true; }
    void setFptr() { Fptr = true; }

    void add(StringRef S) { Str += S; }
    StringRef getString() {
      insertModifiers();
      return Str;
    }
  };

  enum ImplicitClauseKind {
    ICK_private,
    ICK_firstprivate,
    ICK_lastprivate,
    ICK_shared,
    ICK_linear,
    ICK_linear_private,
    ICK_linear_lastprivate,
    ICK_reduction,
    ICK_inreduction,
    ICK_normalized_iv,
    ICK_normalized_ub,
    ICK_livein,
    ICK_unknown
  };

  class ClauseEmissionHelper final {
    llvm::IRBuilderBase::InsertPoint SavedIP;
    OpenMPLateOutliner &O;
    OpenMPClauseKind CK;
    ClauseStringBuilder CSB;
    bool EmitClause;
    ImplicitClauseKind ImplicitClause = ICK_unknown;

  public:
    ClauseEmissionHelper(OpenMPLateOutliner &O, OpenMPClauseKind CK,
                         StringRef InitStr = "", bool EmitClause = true)
        : O(O), CK(CK), CSB(InitStr), EmitClause(EmitClause) {
      if (O.insertPointChangeNeeded()) {
        SavedIP = O.CGF.Builder.saveIP();
        O.setInsertPoint();
      }
    }
    ~ClauseEmissionHelper() {
      if (O.insertPointChangeNeeded())
        O.CGF.Builder.restoreIP(SavedIP);
      if (EmitClause)
        O.emitClause(CK, ImplicitClause);
    }
    ClauseStringBuilder &getBuilder() { return CSB; }
    void setClauseKind(OpenMPClauseKind K) {CK = K; }
    void setImplicitClause(ImplicitClauseKind ICK) {ImplicitClause = ICK; }
  };
  const OMPExecutableDirective &Directive;
  OpenMPDirectiveKind CurrentDirectiveKind;
  OMPClause *CurrentClause = nullptr;

  static ArraySectionDataTy emitArraySectionData(const Expr *E,
                                                 CodeGenFunction &CGF);
  Address emitOMPArraySectionExpr(const Expr *E, ArraySectionTy *AS);

  // Add the string argument.
  void addArg(StringRef Str);

  // Add a llvm::Value directly.
  void addArg(llvm::Value *V, bool Handled = false, bool IsTyped = false,
              llvm::Type *ElementType = nullptr,
              llvm::Value *ZeroValue = nullptr,
              llvm::Value *NumElements = nullptr);

  // Add an llvm::Value with extra 'typed' arguments.
  void addNoElementTypedArg(llvm::Value *V, llvm::Type *ElementType,
                            bool Handled = false);

  void addSingleElementTypedArg(llvm::Value *V, llvm::Type *ElementType,
                                bool Handled = false);

  // Add an argument that is the result of emitting an Expr.
  void addArg(const Expr *E, bool IsRef = false, bool IsTyped = false,
              bool NeedsTypedElements = true, llvm::Type *ElementType = nullptr,
              bool ArraySecUsesBase = true);

  // Add through the Expr with 'typed' arguments.
  void addTypedArg(const Expr *E, bool IsRef = false,
                   bool NeedsTypedElements = true);

  void addFenceCalls(bool IsBegin);
  bool isAllowedClauseForDirectiveFull(OpenMPDirectiveKind DKind,
                                       OpenMPClauseKind CK,
                                       ImplicitClauseKind ICK);
  void getApplicableDirectives(OpenMPClauseKind CK,
                               ImplicitClauseKind ICK,
                               SmallVector<DirectiveIntrinsicSet *, 4> &Dirs);
  void startDirectiveIntrinsicSet(StringRef B, StringRef E,
                                  OpenMPDirectiveKind K);
  void emitDirective(DirectiveIntrinsicSet &D, StringRef Name);
  void emitClause(OpenMPClauseKind CK, ImplicitClauseKind ICK = ICK_unknown);
  void emitOMPSharedClause(const OMPSharedClause *Cl);
  void emitOMPPrivateClause(const OMPPrivateClause *Cl);
  void emitOMPLastprivateClause(const OMPLastprivateClause *Cl);
  void emitOMPLinearClause(const OMPLinearClause *Cl);
  template <typename RedClause>
  void emitOMPReductionClauseCommon(const RedClause *Cl, StringRef QualName,
                                    ImplicitClauseKind ICK = ICK_unknown);
  void emitOMPReductionClause(const OMPReductionClause *Cl);
  void emitOMPOrderedClause(const OMPOrderedClause *C);
  void buildMapQualifier(OpenMPLateOutliner::ClauseStringBuilder &CSB,
                         OpenMPMapClauseKind MapType,
                         const SmallVector<OpenMPMapModifierKind, 1> Modifiers,
                         const VarDecl *MapVar);
  void emitOMPAllMapClauses();
  void emitOMPAllDependClauses();
  void emitOMPMapClause(const OMPMapClause *C);
  void emitOMPScheduleClause(const OMPScheduleClause *C);
  void emitOMPFirstprivateClause(const OMPFirstprivateClause *Cl);
  void emitOMPCopyinClause(const OMPCopyinClause *Cl);
  void emitOMPIfClause(const OMPIfClause *Cl);
  void emitOMPNumThreadsClause(const OMPNumThreadsClause *Cl);
  void emitOMPDefaultClause(const OMPDefaultClause *Cl);
  void emitOMPProcBindClause(const OMPProcBindClause *Cl);
  void emitOMPSafelenClause(const OMPSafelenClause *Cl);
  void emitOMPSimdlenClause(const OMPSimdlenClause *Cl);
  void emitOMPCollapseClause(const OMPCollapseClause *Cl);
  void emitOMPAlignedClause(const OMPAlignedClause *Cl);
  void emitOMPFinalClause(const OMPFinalClause *);
  void emitOMPCopyprivateClause(const OMPCopyprivateClause *);
  void emitOMPNowaitClause(const OMPNowaitClause *);
  void emitOMPUntiedClause(const OMPUntiedClause *);
  void emitOMPMergeableClause(const OMPMergeableClause *);
  void emitOMPFlushClause(const OMPFlushClause *);
  void emitOMPReadClause(const OMPReadClause *);
  void emitOMPWriteClause(const OMPWriteClause *);
  void emitOMPUpdateClause(const OMPUpdateClause *);
  void emitOMPCaptureClause(const OMPCaptureClause *);
  void emitOMPCompareClause(const OMPCompareClause *);
  void emitOMPSeqCstClause(const OMPSeqCstClause *);
  void emitOMPDependClause(const OMPDependClause *);
  void emitOMPDeviceClause(const OMPDeviceClause *);
  void emitOMPSubdeviceClause(const OMPSubdeviceClause *);
  void emitOMPThreadsClause(const OMPThreadsClause *);
  void emitOMPSIMDClause(const OMPSIMDClause *);
  void emitOMPNumTeamsClause(const OMPNumTeamsClause *);
  void emitOMPThreadLimitClause(const OMPThreadLimitClause *);
  void emitOMPPriorityClause(const OMPPriorityClause *);
  void emitOMPGrainsizeClause(const OMPGrainsizeClause *);
  void emitOMPNogroupClause(const OMPNogroupClause *);
  void emitOMPNumTasksClause(const OMPNumTasksClause *);
  void emitOMPHintClause(const OMPHintClause *);
  void emitOMPDistScheduleClause(const OMPDistScheduleClause *);
  void emitOMPDefaultmapClause(const OMPDefaultmapClause *);
  void emitOMPFromClause(const OMPFromClause *);
  void emitOMPToClause(const OMPToClause *);
  void emitOMPUseDevicePtrClause(const OMPUseDevicePtrClause *);
  void emitOMPIsDevicePtrClause(const OMPIsDevicePtrClause *);
  void emitOMPTaskReductionClause(const OMPTaskReductionClause *);
  void emitOMPInReductionClause(const OMPInReductionClause *);
  void emitOMPUnifiedAddressClause(const OMPUnifiedAddressClause *);
  void emitOMPUnifiedSharedMemoryClause(const OMPUnifiedSharedMemoryClause *);
  void emitOMPReverseOffloadClause(const OMPReverseOffloadClause *);
  void emitOMPDynamicAllocatorsClause(const OMPDynamicAllocatorsClause *);
  void
  emitOMPAtomicDefaultMemOrderClause(const OMPAtomicDefaultMemOrderClause *);
  void emitOMPAllocatorClause(const OMPAllocatorClause *);
  void emitOMPAllocateClause(const OMPAllocateClause *);
  void emitOMPNontemporalClause(const OMPNontemporalClause *);
  void emitOMPTileClause(const OMPTileClause *);
  void emitOMPOmpxMonotonicClause(const OMPOmpxMonotonicClause *);
  void emitOMPOmpxOverlapClause(const OMPOmpxOverlapClause *);
  void emitOMPDataClause(const OMPDataClause *);
  void emitOMPFilterClause(const OMPFilterClause *);
  void emitOMPBindClause(const OMPBindClause *);
  void emitOMPOrderClause(const OMPOrderClause *);
  void emitOMPAcqRelClause(const OMPAcqRelClause *);
  void emitOMPAcquireClause(const OMPAcquireClause *);
  void emitOMPReleaseClause(const OMPReleaseClause *);
  void emitOMPRelaxedClause(const OMPRelaxedClause *);
  void emitOMPDepobjClause(const OMPDepobjClause *);
  void emitOMPDestroyClause(const OMPDestroyClause *);
  void emitOMPDetachClause(const OMPDetachClause *);
  void emitOMPInclusiveClause(const OMPInclusiveClause *);
  void emitOMPExclusiveClause(const OMPExclusiveClause *);
  void emitOMPUsesAllocatorsClause(const OMPUsesAllocatorsClause *);
  void emitOMPAffinityClause(const OMPAffinityClause *);
  void emitOMPSizesClause(const OMPSizesClause *);
  void emitOMPUseDeviceAddrClause(const OMPUseDeviceAddrClause *);
  void emitOMPHasDeviceAddrClause(const OMPHasDeviceAddrClause *);
  void emitOMPInitClause(const OMPInitClause *);
  void emitOMPUseClause(const OMPUseClause *);
  void emitOMPNovariantsClause(const OMPNovariantsClause *);
  void emitOMPNocontextClause(const OMPNocontextClause *);
#if INTEL_CUSTOMIZATION
  void emitOMPOmpxAssertClause(const OMPOmpxAssertClause *);
#if INTEL_FEATURE_CSA
  void emitOMPDataflowClause(const OMPDataflowClause *);
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  void emitOMPAlignClause(const OMPAlignClause *Cl);
  void emitOMPFullClause(const OMPFullClause *Cl);
  void emitOMPPartialClause(const OMPPartialClause *Cl);
  void emitOMPOmpxPlacesClause(const OMPOmpxPlacesClause *Cl);
  void emitOMPInteropClause(const OMPInteropClause *);

  llvm::Value *emitOpenMPDefaultConstructor(const Expr *IPriv,
                                            bool IsUDR = false);
  llvm::Value *emitOpenMPDestructor(QualType Ty, bool IsUDR = false);
  llvm::Value *emitOpenMPCopyConstructor(const Expr *IPriv);
  llvm::Value *emitOpenMPCopyAssign(QualType Ty, const Expr *SrcExpr,
                                    const Expr *DstExpr, const Expr *AssignOp);

  bool isIgnoredImplicit(const VarDecl *);
  bool isImplicit(const VarDecl *);
  bool isExplicitForDirective(const VarDecl *, OpenMPDirectiveKind);
  bool alreadyHandled(llvm::Value *);
  void addImplicitClauses();
  void addRefsToOuter();

  bool needsVLAExprEmission();

  void HandleImplicitVar(const Expr *E, ImplicitClauseKind ICK);
  llvm::MapVector<const VarDecl *, ImplicitClauseKind> ImplicitMap;

  llvm::DenseMap<const VarDecl *, SmallVector<OpenMPClauseKind, 2>>
      ExplicitRefs;

  llvm::DenseSet<const VarDecl *> VarDefs;

  // Host and target may use different ABIs, which in turn may cause
  // different processing order for expressions (e.g. MS C++ ABI
  // implies right to left processing of call parameters).
  // We have to guarantee that the order of implicitly FIRSTPRIVATE
  // variables for "omp target" regions is the same for host and target.
  // This comparator uses the source locations and the variable names
  // to set up this order.
  struct VarCompareTy {
    bool operator()(const VarDecl *const &V1, const VarDecl *const &V2) const {
      const auto &SL1 = V1->getBeginLoc();
      const auto &SL2 = V2->getBeginLoc();
      if (SL1 < SL2)
        return true;
      if (SL1 != SL2)
        return false;

      if (V1->getName() < V2->getName())
        return true;
      if (V1->getName() != V2->getName())
        return false;

      assert(V1 == V2 && "Cannot order variables.");
      return false;
    }
  };
  std::set<const VarDecl *, VarCompareTy> VarRefs;
  llvm::DenseSet<const VarDecl *> DispatchExplicitVars;
  llvm::DenseSet<const VarDecl *> DependIteratorVars;
  llvm::SmallVector<std::pair<llvm::Value *, const VarDecl *>, 8> MapTemps;
#if INTEL_CUSTOMIZATION
  llvm::MapVector<const VarDecl *, std::string> OptRepFPMapInfos;
#endif  // INTEL_CUSTOMIZATION

  std::vector<std::pair<llvm::WeakTrackingVH, llvm::Type *>> DefinedValues;
  std::vector<std::pair<llvm::WeakTrackingVH, llvm::Type *>> ReferencedValues;
  llvm::DenseSet<llvm::Value *> HandledValues;

  bool UseTypedClauses = false;

public:
  static const VarDecl *getExplicitVarDecl(const Expr *E);
  static const DeclRefExpr *getExplicitDeclRefOrNull(const Expr *E);
  static const Expr *getArraySectionBase(const Expr *E,
                                         CodeGenFunction *CGF = nullptr,
                                         ArraySectionTy *AS = nullptr);
  OpenMPLateOutliner(CodeGenFunction &CGF, const OMPExecutableDirective &D,
                     OpenMPDirectiveKind Kind);
  ~OpenMPLateOutliner();
  bool isImplicitLastPrivate(const VarDecl *VD) {
   return isImplicit(VD) && (ImplicitMap[VD] == ICK_lastprivate ||
                             ImplicitMap[VD] == ICK_linear_lastprivate);
  }
  void privatizeMappedPointers(CodeGenFunction::OMPPrivateScope &PrivateScope) {
    for (auto MT : MapTemps) {
      llvm::Type *Ty = MT.first->getType();
      Address A = CGF.CreateDefaultAlignTempAlloca(Ty, MT.second->getName() +
                                                           ".map.ptr.tmp");
      if (MT.second->getType()->isReferenceType())
        A.setRemovedReference();
      if (auto *DI = CGF.getDebugInfo())
        if (CGF.CGM.getCodeGenOpts().hasReducedDebugInfo())
          (void)DI->EmitDeclareOfAutoVariable(MT.second, A.getPointer(),
                                              CGF.Builder);
      CGF.Builder.CreateStore(MT.first, A);
      PrivateScope.addPrivateNoTemps(MT.second, [A]() -> Address { return A; });
      CGF.addMappedTemp(MT.second, MT.second->getType()->isReferenceType());
    }
    PrivateScope.Privatize();
  }
#if INTEL_CUSTOMIZATION
  void emitRemark(std::string Str);
#endif // INTEL_CUSTOMIZATION
  bool isImplicitTask(OpenMPDirectiveKind K);
  bool isImplicitTaskgroup(OpenMPDirectiveKind K);
  bool shouldSkipExplicitClause(OpenMPClauseKind K);
  void emitOMPParallelDirective();
  void emitOMPParallelForDirective();
  void emitOMPSIMDDirective();
  void emitOMPForDirective();
  void emitOMPForSimdDirective();
  void emitOMPParallelForSimdDirective();
  void emitOMPAtomicDirective(OMPAtomicClause ClauseKind);
  void emitOMPSingleDirective();
  void emitOMPMasterDirective();
  void emitOMPMaskedDirective();
  void emitOMPCriticalDirective(const StringRef Name);
  void emitOMPOrderedDirective();
  void emitOMPTargetDirective(int OffloadEntryIndex);
  void emitOMPTargetDataDirective();
  void emitOMPTargetUpdateDirective();
  void emitOMPTargetEnterDataDirective();
  void emitOMPTargetExitDataDirective();
  void emitOMPTaskLoopDirective();
  void emitOMPTaskLoopSimdDirective();
  void emitOMPTaskDirective();
  void emitOMPTaskGroupDirective();
  void emitOMPTaskWaitDirective();
  void emitOMPTaskYieldDirective();
  void emitOMPBarrierDirective();
  void emitOMPFlushDirective();
  void emitOMPTeamsDirective();
  void emitOMPDistributeDirective();
  void emitOMPDistributeSimdDirective();
  void emitOMPDistributeParallelForDirective();
  void emitOMPDistributeParallelForSimdDirective();
  void emitOMPSectionsDirective();
  void emitOMPSectionDirective();
  void emitOMPParallelSectionsDirective();
  void emitOMPCancelDirective(OpenMPDirectiveKind Kind);
  void emitOMPCancellationPointDirective(OpenMPDirectiveKind Kind);
  void emitOMPTargetVariantDispatchDirective();
  void emitOMPDispatchDirective();
  void emitOMPGenericLoopDirective();
  void emitOMPInteropDirective();
  void emitOMPPrefetchDirective();
  void emitOMPScopeDirective();
  void emitOMPScanDirective();
  void emitVLAExpressions() {
    if (needsVLAExprEmission())
      CGF.VLASizeMapHandler->EmitVLASizeExpressions();
  }

  OpenMPLateOutliner &operator<<(ArrayRef<OMPClause *> Clauses);

  void emitImplicitLoopBounds(const OMPLoopDirective *LD);
  void emitImplicit(Expr *E, ImplicitClauseKind K);
  void emitImplicit(const VarDecl *VD, ImplicitClauseKind K);
  void emitImplicit(llvm::Value *V, llvm::Type *ElementType,
                    ImplicitClauseKind K, bool Handled = false);
  void addVariableDef(const VarDecl *VD) { VarDefs.insert(VD); }
  void addVariableRef(const VarDecl *VD) { VarRefs.insert(VD); }
  void addDispatchExplicitVar(const VarDecl *VD) {
    DispatchExplicitVars.insert(VD);
  }
  void addDependIteratorVar(const VarDecl *VD) {
    DependIteratorVars.insert(VD);
  }
  bool isDispatchExplicitVar(const VarDecl *VD) {
    return DispatchExplicitVars.find(VD) != DispatchExplicitVars.end();
  }
  void addValueDef(llvm::Value *V, llvm::Type *ElemTy) {
    llvm::WeakTrackingVH VH = V;
    DefinedValues.push_back({VH, ElemTy});
  }
  void addValueRef(llvm::Value *V, llvm::Type *ElemTy) {
    llvm::WeakTrackingVH VH = V;
    ReferencedValues.push_back({VH, ElemTy});
  }
  void addValueSuppress(llvm::Value *V) { HandledValues.insert(V); }
  OpenMPDirectiveKind getCurrentDirectiveKind() { return CurrentDirectiveKind; }
  void addExplicit(const VarDecl *VD, OpenMPClauseKind CK) {
    auto It = ExplicitRefs.find(VD);
    if (It != ExplicitRefs.end()) {
      It->second.push_back(CK);
    } else {
      llvm::SmallVector<OpenMPClauseKind, 2> CKs = {CK};
      ExplicitRefs.insert({VD,CKs});
    }
  }
  bool isExplicitForIsDevicePtr(const VarDecl *V) {
    const auto &It = ExplicitRefs.find(V);
    if (It == ExplicitRefs.end())
      return false;
    for (auto &CK : (*It).second)
      if (CK == llvm::omp::OMPC_is_device_ptr)
        return true;
    return false;
  }
  bool insertPointChangeNeeded() { return MarkerInstruction != nullptr; }
  void setInsertPoint() {
    assert(MarkerInstruction);
    CGF.Builder.SetInsertPoint(MarkerInstruction);
  }
  void insertMarker() {
    // Create a marker call at the start of the region.  The values generated
    // from clauses must be inserted before this point.
    ApplyDebugLocation DL (CGF, Directive.getBeginLoc());
    // Create a marker for each directive.
    for (auto &D : Directives)
      D.CallEntry = CGF.Builder.CreateCall(RegionEntryDirective, {});
    // If there are directives use the first directive as the main marker
    // for clauses.
    MarkerInstruction = Directives.empty()
                            ? CGF.Builder.CreateCall(RegionEntryDirective, {})
                            : Directives[0].CallEntry;
  }
  llvm::Value *emitSpecialSIMDExpression(const Expr* E) {
    if (Directives.empty() ||
        Directives[Directives.size() - 1].DKind != llvm::omp::OMPD_simd)
      return CGF.EmitScalarExpr(E);

    // Emit an expression just above the SIMD directive.
    CGBuilderTy::InsertPoint SavedIP = CGF.Builder.saveIP();
    const DirectiveIntrinsicSet &D = Directives[Directives.size() - 1];
    CGF.Builder.SetInsertPoint(D.CallEntry);
    llvm::Value *V = CGF.EmitScalarExpr(E);
    CGF.Builder.restoreIP(SavedIP);
    return V;
  }
  static bool isFirstDirectiveInSet(const OMPExecutableDirective &S,
                                    OpenMPDirectiveKind Kind);
  bool checkIfModifier(OpenMPDirectiveKind DKind, const OMPIfClause *IC);

  static bool hasCapturedStmt(const OMPExecutableDirective &S) {
    auto Kind = S.getDirectiveKind();
    if (Kind == llvm::omp::OMPD_atomic || Kind == llvm::omp::OMPD_critical ||
        Kind == llvm::omp::OMPD_section || Kind == llvm::omp::OMPD_master ||
        Kind == llvm::omp::OMPD_masked)
      return false;
    return true;
  }
};

class OMPLateOutlineLexicalScope : public CodeGenFunction::LexicalScope {
  CodeGenFunction::OMPPrivateScope Remaps;
public:
  OMPLateOutlineLexicalScope(CodeGenFunction &CGF,
                             const OMPExecutableDirective &S,
                             OpenMPDirectiveKind Kind)
      : CodeGenFunction::LexicalScope(CGF, S.getSourceRange()), Remaps(CGF) {

    // Only declare variables for the PreInit statements on the first
    // directive in a multi-directive set.
    if (!OpenMPLateOutliner::isFirstDirectiveInSet(S, Kind))
      return;

    for (const auto *C : S.clauses()) {
      if (const auto *CPI = OMPClauseWithPreInit::get(C)) {
        if (const auto *PreInit =
                cast_or_null<DeclStmt>(CPI->getPreInitStmt())) {
          for (const auto *I : PreInit->decls()) {
            if (!I->hasAttr<OMPCaptureNoInitAttr>()) {
              CGF.EmitVarDecl(cast<VarDecl>(*I));
            } else {
              CodeGenFunction::AutoVarEmission Emission =
                  CGF.EmitAutoVarAlloca(cast<VarDecl>(*I));
              CGF.EmitAutoVarCleanups(Emission);
            }
          }
        }
      }
    }

    CGF.RemapForLateOutlining(S, Remaps);
    (void)Remaps.Privatize();
  }

  static bool isCapturedVar(CodeGenFunction &CGF, const VarDecl *VD) {
    return CGF.LambdaCaptureFields.lookup(VD) ||
           (CGF.CapturedStmtInfo && CGF.CapturedStmtInfo->lookup(VD)) ||
           (CGF.CurCodeDecl && isa<BlockDecl>(CGF.CurCodeDecl));
  }

};

class CGLateOutlineOpenMPRegionInfo
    : public CodeGenFunction::CGCapturedStmtInfo {
public:
  CGLateOutlineOpenMPRegionInfo(CodeGenFunction::CGCapturedStmtInfo *CSI,
                                OpenMPLateOutliner &O,
                                const OMPExecutableDirective &D)
      : CGCapturedStmtInfo(*cast<CapturedStmt>(D.getAssociatedStmt()),
                           CR_OpenMP),
        OldCSI(CSI), Outliner(O), D(D) {}

  /// Retrieve the value of the context parameter.
  llvm::Value *getContextValue() const override;
  void setContextValue(llvm::Value *V) override;

  /// Lookup the captured field decl for a variable.
  const FieldDecl *lookup(const VarDecl *VD) const override;

  FieldDecl *getThisFieldDecl() const override;

  bool isDispatchTargetCall(SourceLocation Loc) override;

  CodeGenFunction::CGCapturedStmtInfo *getOldCSI() const { return OldCSI; }

  void recordVariableDefinition(const VarDecl *VD) override {
    Outliner.addVariableDef(VD);
    Outliner.addVariableRef(VD);
  }
  void recordVariableReference(const VarDecl *VD) override {
    Outliner.addVariableRef(VD);
  }
  void recordValueDefinition(llvm::Value *V, llvm::Type *ElemTy) override {
    Outliner.addValueDef(V, ElemTy);
    Outliner.addValueRef(V, ElemTy);
  }
  void recordValueReference(llvm::Value *V, llvm::Type *ElemTy) override {
    Outliner.addValueRef(V, ElemTy);
  }
  void recordValueSuppression(llvm::Value *V) override {
    Outliner.addValueSuppress(V);
  }
  void recordDispatchExplicitVar(const VarDecl *VD) override {
    Outliner.addDispatchExplicitVar(VD);
  }
  void recordDependIteratorVar(const VarDecl *VD) override {
    Outliner.addDependIteratorVar(VD);
  }
  bool inTargetVariantDispatchRegion() override {
    return Outliner.getCurrentDirectiveKind() ==
           llvm::omp::OMPD_target_variant_dispatch;
  }
  bool inDispatchRegion() override {
    return Outliner.getCurrentDirectiveKind() == llvm::omp::OMPD_dispatch;
  }

  void enterTryStmt() override { ++TryStmts; }
  void exitTryStmt() override {
    assert(TryStmts > 0);
    --TryStmts;
  }
  bool inTryStmt() override { return TryStmts > 0; }
  bool isLateOutlinedRegion() override { return true; }
  bool isImplicitLastPrivate(const VarDecl *VD) override {
    return Outliner.isImplicitLastPrivate(VD);
  }

private:
  /// CodeGen info about outer OpenMP region.
  CodeGenFunction::CGCapturedStmtInfo *OldCSI;
  OpenMPLateOutliner &Outliner;
  /// Nesting of C++ 'try' statements in the OpenMP region.
  unsigned TryStmts = 0;
  const OMPExecutableDirective &D;
};

/// RAII for emitting code of OpenMP constructs.
class LateOutlineOpenMPRegionRAII {
  CodeGenFunction &CGF;
  OpenMPLateOutliner &Outliner;
  const OMPExecutableDirective &Dir;
public:
  /// Constructs region for combined constructs.
  /// \param CodeGen Code generation sequence for combined directives. Includes
  /// a list of functions used for code generation of implicitly inlined
  /// regions.
  LateOutlineOpenMPRegionRAII(CodeGenFunction &CGF, OpenMPLateOutliner &O,
                              const OMPExecutableDirective &D)
      : CGF(CGF), Outliner(O), Dir(D) {

    if (!OpenMPLateOutliner::hasCapturedStmt(Dir))
      return;

    // Start emission for the construct.
    CGF.CapturedStmtInfo =
        new CGLateOutlineOpenMPRegionInfo(CGF.CapturedStmtInfo, Outliner, D);
  }
  ~LateOutlineOpenMPRegionRAII() {
    if (!OpenMPLateOutliner::hasCapturedStmt(Dir))
      return;

    // Restore original CapturedStmtInfo only if we're done with code emission.
    auto *OldCSI =
        static_cast<CGLateOutlineOpenMPRegionInfo *>(CGF.CapturedStmtInfo)
            ->getOldCSI();
    delete CGF.CapturedStmtInfo;
    CGF.CapturedStmtInfo = OldCSI;
  }
};

} // namespace CodeGen
} // namespace clang

#endif  // LLVM_CLANG_LIB_CODEGEN_CGOPENMPLATEOUTLINE_H
#endif // INTEL_COLLAB
