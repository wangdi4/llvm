//===- IntelVPlanExternals.h - Represent VPlan external values storage ---===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains definition of the VPlan externals storage,
// VPExternalDef, VPExternalUse, VPConstant.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_EXTERNALS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_EXTERNALS_H

#include "IntelVPlanOptrpt.h"
#include "IntelVPlanValue.h"
#include "llvm/ADT/MapVector.h"

#include <map>

namespace llvm {
namespace vpo {

class VPlanVector;
class VPLoopEntityList;
class ScalarInOutList;
class VPLoopEntity;
class VPInstruction;

/// Auxiliary class to keep VPInstructions that are removed from CFG. We can't
/// delete them during removal because we might have links to them in the data
/// structures that are not in the def-use chain (e.g. DA). The problem can be
/// solved having a tracking mechanism but that is left for the future.
class VPUnlinkedInstructions {
public:
  // Add a VPInstruction that needs to be erased in UnlinkedVPInsns vector.
  void addUnlinkedVPInst(VPInstruction *I) { UnlinkedVPInsts.emplace_back(I); }

private:
  SmallVector<std::unique_ptr<VPInstruction>, 8> UnlinkedVPInsts;
};

// Auxiliary class to describe live-in/out values for scalar loop.
class ScalarInOutDescr {
  PHINode *Phi;         // the phi node that takes the starting value
  int StartValOpNum;    // Start-value operand num
  const Value *LiveOut; // Live out value
  unsigned MergeId;     // Synchronization key

public:
  ScalarInOutDescr(PHINode *PN, int StartOp, const Value *LiveOutInst,
                   unsigned Id)
      : Phi(PN), StartValOpNum(StartOp), LiveOut(LiveOutInst), MergeId(Id) {}

  Type *getValueType() const { return LiveOut->getType(); }
  PHINode *getPhi() const { return Phi; }
  int getStartOpNum() const { return StartValOpNum; }
  const Value *getLiveOut() const { return LiveOut; }
  unsigned getId() const { return MergeId; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const;
  void dump() const { dump(outs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Auxiliary class to describe live-in/out values for scalar HLLoop.
class ScalarInOutDescrHIR {
  unsigned MergeId;             // Synchronization key
  const loopopt::DDRef *HIRRef; // Live-in/out temp
  bool IsMainLoopIV;

public:
  ScalarInOutDescrHIR(const loopopt::DDRef *HIRRef, bool IsMainLoopIV,
                      unsigned Id)
      : MergeId(Id), HIRRef(HIRRef), IsMainLoopIV(IsMainLoopIV) {}

  Type *getValueType() const { return HIRRef->getDestType(); }
  unsigned getId() const { return MergeId; }
  const loopopt::DDRef *getHIRRef() const { return HIRRef; }
  bool isMainLoopIV() const { return IsMainLoopIV; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS) const;
  void dump() const { print(outs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// List of descriptors of in/out values for scalar loop.
class ScalarInOutList {
  // The list of descriptors is kept as a map to be able to take
  // descriptors by Id.
  using InOutListTy = MapVector<unsigned, std::unique_ptr<ScalarInOutDescr>>;
  InOutListTy InOutList;

public:
  // Add a descriptor for LLVM-IR input.
  void add(PHINode *PN, int StartOp, const Value *LiveOutInst, unsigned Id) {
    assert(InOutList.count(Id) == 0 && "Second descriptor for an Id");
    InOutList.insert(std::make_pair(
        Id, std::make_unique<ScalarInOutDescr>(PN, StartOp, LiveOutInst, Id)));
  }

  ScalarInOutDescr *getDescr(unsigned Id) const {
    auto Iter = InOutList.find(Id);
    assert(Iter != InOutList.end() && "Invalid Id");
    return Iter->second.get();
  }

  // Return iterator to list of descriptors.
  decltype(auto) list() const {
    return map_range(
        InOutList, [](const InOutListTy::value_type &I) -> ScalarInOutDescr * {
          return I.second.get();
        });
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const {
    OS << "Original loop live-ins/live-outs:";
    if (InOutList.empty())
      OS << " empty\n";
    else {
      OS << "\n";
      for (const auto Descr : list()) {
        Descr->dump(OS);
        OS << "\n";
      }
    }
  }
  void dump() const { dump(dbgs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// List of descriptors of in/out values for scalar HLLoop.
class ScalarInOutListHIR {
  // The list of descriptors is kept as a map to be able to take
  // descriptors by Id.
  using InOutListTy = MapVector<unsigned, std::unique_ptr<ScalarInOutDescrHIR>>;
  InOutListTy InOutList;

public:
  // Add a descriptor for HIR input.
  void add(const loopopt::DDRef *HIRRef, bool IsMainLoopIV, unsigned Id) {
    assert(InOutList.count(Id) == 0 && "Second descriptor for an Id");
    InOutList.insert(std::make_pair(
        Id, std::make_unique<ScalarInOutDescrHIR>(HIRRef, IsMainLoopIV, Id)));
  }

  ScalarInOutDescrHIR *getDescr(unsigned Id) const {
    auto Iter = InOutList.find(Id);
    assert(Iter != InOutList.end() && "Invalid Id");
    return Iter->second.get();
  }

  // Return iterator to list of descriptors.
  decltype(auto) list() const {
    return map_range(
        InOutList,
        [](const InOutListTy::value_type &I) -> ScalarInOutDescrHIR * {
          return I.second.get();
        });
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const {
    OS << "Original loop live-ins/live-outs:";
    if (InOutList.empty())
      OS << " empty\n";
    else {
      OS << "\n";
      for (const auto Descr : list()) {
        Descr->print(OS);
        OS << "\n";
      }
    }
  }
  void dump() const { dump(dbgs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

/// Class to hold external data used in VPlan. We can have several VPlans that
/// model different vectozation scenarios for one loop. Those VPlans are in most
/// cases clones of one created at the beginning of vectorization. All the
/// external definitions in these case are common and not needed to be cloned.
class VPExternalValues {
  // HIRDecomposer fixes VPExternalUses with multiple operands after building
  // initial plain CFG. Friendship is needed to get non-const iterator range to
  // VPExternalUsesHIR container.
  friend class VPDecomposerHIR;
  friend class VPLiveInOutCreator;
  // Codegen looks through the list of external uses and updates their original
  // operands.
  friend class VPOCodeGen;

  const DataLayout *DL = nullptr;

  LLVMContext *Context = nullptr;

  /// Holds all the VPConstants created for this VPlan.
  DenseMap<Constant *, std::unique_ptr<VPConstant>> VPConstants;

  /// Holds all the external definitions representing an underlying Value
  /// in this VPlan. CFGBuilder ensures these are unique.
  SmallVector<std::unique_ptr<VPExternalDef>, 16> VPExternalDefs;

  /// Holds all the external definitions representing an HIR underlying entity
  /// in this VPlan. The hash is based on the underlying HIR information that
  /// uniquely identifies each external definition.
  FoldingSet<VPExternalDef> VPExternalDefsHIR;

  /// Return the iterator range for external defs in VPExternalDefsHIR.
  decltype(auto) getVPExternalDefsHIR() const {
    return map_range(VPExternalDefsHIR,
                     [](const VPExternalDef &Def) { return &Def; });
  }

  /// Holds all the external uses in this VPlan.
  SmallVector<std::unique_ptr<VPExternalUse>, 16> VPExternalUses;

  /// The MergeId counter. Provides unique identifiers of loop entities which
  /// have live out values. The uniqueness is provided by incrementing on
  /// each VPExternalUse creation.
  /// The MergeId is kept in VPExternalUse, VPLiveInValue, and in VPLiveOutValue
  /// so they can be easily identfied as a pair of live-out/live-in values in VPlan.
  /// The MergeId is not changed during cloning and used during CFG merging, to
  /// identify linked in/out values in different VPlan loops (different kinds of
  /// main/peel/remainder loops).
  unsigned getLastMergeId() const { return VPExternalUses.size(); }

  decltype(auto) getVPExternalUsesHIR() {
    return map_range(
        make_filter_range(VPExternalUses,
                          [](std::unique_ptr<VPExternalUse> &It) {
                            return It->getOperandHIR() != nullptr;
                          }),
        [](std::unique_ptr<VPExternalUse> &It) { return It.get(); });
  }

  /// Holds all the VPMetadataAsValues created for this VPlan.
  DenseMap<MetadataAsValue *, std::unique_ptr<VPMetadataAsValue>>
      VPMetadataAsValues;

  // Holds the original incoming values.
  SmallVector<VPValue*, 16> OriginalIncomingValues;

  // Holds live-in/out descriptors of scalar loops.
  std::map<const Loop *, ScalarInOutList> ScalarLoopsInOut;
  std::map<const loopopt::HLLoop *, ScalarInOutListHIR> ScalarLoopsInOutHIR;

  // Holds opt-report stats tracker created for different VPLoops.
  // TODO: Drop "mutable" after implementing VPlan statistics collection pass
  // before CG.
  mutable DenseMap<const VPLoop *, OptReportStatsTracker> OptRptStatsTrackers;

public:
  VPExternalValues(LLVMContext *Ctx, const DataLayout *L) : DL(L), Context(Ctx) {}
  VPExternalValues(const VPExternalValues &X) = delete;

  ~VPExternalValues() {
    // Release memory allocated for VPExternalDefs tracked in VPExternalDefsHIR.
    // Temporary list to collect the pointers is needed to avoid invalid access
    // error while iterating the FoldingSet.
    SmallVector<const VPExternalDef *, 16> TmpExtDefList(
        getVPExternalDefsHIR().begin(), getVPExternalDefsHIR().end());
    VPExternalDefsHIR.clear();
    for (auto *ExtDef : TmpExtDefList)
      delete ExtDef;
  }

  const DataLayout *getDataLayout() const { return DL; }
  LLVMContext *getLLVMContext(void) const { return Context; }

  /// Create a new VPConstant for \p Const if it doesn't exist or retrieve the
  /// existing one.
  VPConstant *getVPConstant(Constant *Const) {
    std::unique_ptr<VPConstant> &UPtr = VPConstants[Const];
    if (!UPtr) {
      // Const is a new VPConstant to be inserted in the map.
      if (isa<ConstantInt>(Const))
        UPtr.reset(new VPConstantInt(cast<ConstantInt>(Const)));
      else
        UPtr.reset(new VPConstant(Const));
    }

    return UPtr.get();
  }

  /// Create or retrieve a VPExternalDef for a given Value \p ExtVal.
  VPExternalDef *getVPExternalDef(Value *ExtDef) {
    VPExternalDefs.emplace_back(new VPExternalDef(ExtDef));
    return VPExternalDefs.back().get();
  }

  /// Create or retrieve a VPExternalDef for a given non-decomposable DDRef \p
  /// DDR.
  VPExternalDef *getVPExternalDefForDDRef(const loopopt::DDRef *DDR) {
    assert(DDR->isNonDecomposable() && "Expected non-decomposable DDRef!");
    FoldingSetNodeID ID;
    ID.AddPointer(nullptr);
    ID.AddInteger(DDR->getSymbase());
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (VPExternalDef *ExtDef = VPExternalDefsHIR.FindNodeOrInsertPos(ID, IP))
      return ExtDef;
    auto *ExtDef = new VPExternalDef(DDR);
    VPExternalDefsHIR.InsertNode(ExtDef, IP);
    return ExtDef;
  }

  /// Create or retrieve a VPExternalDef for the blob with index \p BlobIndex in
  /// \p DDR.
  VPExternalDef *getVPExternalDefForBlob(const loopopt::RegDDRef *DDR,
                                         unsigned BlobIndex) {
    FoldingSetNodeID ID;

    // The blob(SCEV expression) with index BlobIndex is the folding set key.
    const loopopt::BlobTy BlobExpr = DDR->getBlobUtils().getBlob(BlobIndex);
    ID.AddPointer(BlobExpr);
    ID.AddInteger(0 /*SymBase*/);
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (VPExternalDef *ExtDef = VPExternalDefsHIR.FindNodeOrInsertPos(ID, IP))
      return ExtDef;

    auto *ExtDef = new VPExternalDef(DDR, BlobIndex, BlobExpr->getType());
    VPExternalDefsHIR.InsertNode(ExtDef, IP);
    return ExtDef;
  }

  /// Create or retrieve a VPExternalDef for the given canon expression \p CE.
  VPExternalDef *getVPExternalDefForCanonExpr(const loopopt::CanonExpr *CE,
                                              const loopopt::RegDDRef *DDR) {
    // Search through the table for an external def equivalent to CE and return
    // the same if found. DDR is used to set the definedatlevel of blobs in CE
    // during vector code generation. The canon expression that we are dealing
    // with here is invariant at the loop level that is being vectorized.
    // The blobs within such canon expressions will have the same definedatlevel
    // independent of the DDR. When searching for an external def that is
    // equivalent we do not need any DDR specific checks as a result.
    auto Iter =
        llvm::find_if(VPExternalDefsHIR, [CE](const VPExternalDef &ExtDef) {
          return ExtDef.getOperandHIR()->isEqual(CE);
        });
    if (Iter != VPExternalDefsHIR.end())
      return &*Iter;

    auto *ExtDef = new VPExternalDef(CE, DDR);
    VPExternalDefsHIR.InsertNode(ExtDef);
    return ExtDef;
  }

  /// Retrieve the VPExternalDef for given HIR symbase \p Symbase. If no
  /// external definition exists then a nullptr is returned.
  VPExternalDef *getVPExternalDefForSymbase(unsigned Symbase) {
    FoldingSetNodeID ID;
    ID.AddPointer(nullptr);
    ID.AddInteger(Symbase);
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (VPExternalDef *ExtDef = VPExternalDefsHIR.FindNodeOrInsertPos(ID, IP))
      return ExtDef;

    // No Def found in table
    return nullptr;
  }

  /// Create or retrieve a VPExternalDef for an HIR IV identified by its \p
  /// IVLevel.
  VPExternalDef *getVPExternalDefForIV(unsigned IVLevel, Type *BaseTy) {
    FoldingSetNodeID ID;
    ID.AddPointer(nullptr);
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(IVLevel);
    void *IP = nullptr;
    if (VPExternalDef *ExtDef = VPExternalDefsHIR.FindNodeOrInsertPos(ID, IP))
      return ExtDef;
    auto *ExtDef = new VPExternalDef(IVLevel, BaseTy);
    VPExternalDefsHIR.InsertNode(ExtDef, IP);
    return ExtDef;
  }

  VPExternalDef *getVPExternalDefForIfCond(const loopopt::HLIf *If) {
    // Note: it doesn't make much sense to uniqify those in current setup as
    // we're expected to have a single attempt at creation of such a
    // VPExternalDef.
    // TODO: Change the current code to be able to handle rhs of HLInst
    // and condition of HLIf in a generic way, avoiding duplication of
    // VPExternalDefs.
    FoldingSetNodeID ID;
    ID.AddPointer(If);
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (VPExternalDef *ExtDef = VPExternalDefsHIR.FindNodeOrInsertPos(ID, IP))
      return ExtDef;
    auto *ExtDef = new VPExternalDef(If);
    VPExternalDefsHIR.InsertNode(ExtDef, IP);
    return ExtDef;
  }

  /// Return VPExternalUse by its MergeId.
  VPExternalUse *getVPExternalUse(unsigned MergeId) const {
    return VPExternalUses[MergeId].get();
  }

  // Return the iterator-range to the list of ExternalUses.
  inline decltype(auto) externalUses() const {
    return map_range(
        VPExternalUses,
        [](const std::unique_ptr<VPExternalUse> &It) { return It.get(); });
  }

  /// Create a or retrieve VPExternalUse for a given Value \p ExtVal.
  VPExternalUse *getOrCreateVPExternalUse(PHINode *ExtDef) {
    auto It = llvm::find_if(VPExternalUses,
                            [ExtDef](std::unique_ptr<VPExternalUse> &It) {
                              return It->getUnderlyingValue() == ExtDef;
                            });
    if (It != VPExternalUses.end())
      return It->get();
    unsigned Id = getLastMergeId();
    VPExternalUse *ExtUse = new VPExternalUse(ExtDef, Id);
    return insertExternalUse(ExtUse, Id);
  }

  /// Create a or retrieve VPExternalUse for a given non-decomposable DDRef \p
  /// DDR.
  VPExternalUse *getOrCreateVPExternalUseForDDRef(const loopopt::DDRef *DDR) {
    VPBlob Blob(DDR);
    auto It = llvm::find_if(VPExternalUses,
                         [&Blob](const std::unique_ptr<VPExternalUse> &It) {
                           const VPOperandHIR *Op = It->getOperandHIR();
                           if (!Op)
                             return false;
                           return Blob.isStructurallyEqual(Op);
                         });
    if (It != VPExternalUses.end())
      return It->get();
    unsigned Id = getLastMergeId();
    VPExternalUse *ExtUse = new VPExternalUse(DDR, Id);
    return insertExternalUse(ExtUse, Id);
  }

  /// Create or retrieve a VPExternalUse for an HIR IV identified by its \p
  /// IVLevel.
  VPExternalUse *getOrCreateVPExternalUseForIV(unsigned IVLevel, Type *BaseTy) {
    VPIndVar IndVar(IVLevel);
    auto It = llvm::find_if(
        VPExternalUses, [&IndVar](const std::unique_ptr<VPExternalUse> &It) {
          const VPOperandHIR *Op = It->getOperandHIR();
          if (!Op)
            return false;
          return IndVar.isStructurallyEqual(Op);
        });
    if (It != VPExternalUses.end())
      return It->get();
    unsigned Id = getLastMergeId();
    VPExternalUse *ExtUse = new VPExternalUse(IVLevel, BaseTy, Id);
    return insertExternalUse(ExtUse, Id);
  }

  /// Create a VPExternalUse w/o underlying IR. This fake VPExternalUse
  /// is created for non-live-out inductions to have a common scheme for
  /// identification of live-in/out values in such cases.
  VPExternalUse *createVPExternalUseNoIR(Type *BaseTy) {
    unsigned Id = getLastMergeId();
    VPExternalUse *ExtUse = new VPExternalUse(Id, BaseTy);
    return insertExternalUse(ExtUse, Id);
  }

  /// Create a new VPMetadataAsValue for \p MDAsValue if it doesn't exist or
  /// retrieve the existing one.
  VPMetadataAsValue *getVPMetadataAsValue(MetadataAsValue *MDAsValue) {
    std::unique_ptr<VPMetadataAsValue> &UPtr = VPMetadataAsValues[MDAsValue];
    if (!UPtr)
      // MDAsValue is a new VPMetadataAsValue to be inserted in the map.
      UPtr.reset(new VPMetadataAsValue(MDAsValue));

    return UPtr.get();
  }

  /// Create a new VPMetadataAsValue for Metadata \p MD if it doesn't exist or
  /// retrieve the existing one.
  VPMetadataAsValue *getVPMetadataAsValue(Metadata *MD) {
    return getVPMetadataAsValue(MetadataAsValue::get(*Context, MD));
  }

  VPValue *getOriginalIncomingValue(int MergeId) const {
    return OriginalIncomingValues[MergeId];
  }

  void addOriginalIncomingValue(VPValue *V) {
    OriginalIncomingValues.push_back(V);
  }

  void setOriginalIncomingValue(VPValue *V, int MergeId) {
    assert(OriginalIncomingValues[MergeId] == nullptr &&
           "Inconsistent OriginalIncomingValue");
    OriginalIncomingValues[MergeId] = V;
  }

  void allocateOriginalIncomingValues(int Count) {
    assert(OriginalIncomingValues.size() == 0 && "The list is not empty");
    OriginalIncomingValues.resize(Count, nullptr);
  }

  /// Get in/out list for scalar loop.
  const ScalarInOutList *getScalarLoopInOuts(const Loop *OrigLoop) const {
    auto It = ScalarLoopsInOut.find(OrigLoop);
    if (It != ScalarLoopsInOut.end())
      return &It->second;
    return nullptr;
  }
  const ScalarInOutListHIR *
  getScalarLoopInOuts(const loopopt::HLLoop *OrigLoop) const {
    auto It = ScalarLoopsInOutHIR.find(OrigLoop);
    if (It != ScalarLoopsInOutHIR.end())
      return &It->second;
    return nullptr;
  }

  OptReportStatsTracker &
  getOrCreateOptRptStatsForLoop(const VPLoop *VLp) const {
    return OptRptStatsTrackers[VLp];
  }
  void setOptRptStatsForLoop(const VPLoop *VLp, OptReportStatsTracker &ORS) {
    OptRptStatsTrackers[VLp] = ORS;
  }

  // Verify that VPConstants are unique in the pool and that the map keys are
  // consistent with the underlying IR information of each VPConstant.
  void verifyVPConstants() const;

  // Verify that VPExternalDefs are unique in the pool and that the map keys are
  // consistent with the underlying IR information of each VPExternalDef.
  void verifyVPExternalDefs() const;

  // Verify that VPExternalDefs are unique in the pool and that the map keys are
  // consistent with the underlying HIR information of each VPExternalDef.
  void verifyVPExternalDefsHIR() const;

  // Verify that VPMetadataAsValues are unique in the pool and that the map keys
  // are consistent with the underlying IR information of each
  // VPMetadataAsValue.
  void verifyVPMetadataAsValues() const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpExternalDefs(raw_ostream &FOS) const;
  void dumpExternalUses(raw_ostream &FOS, const VPlan *P) const;
  template <class LoopTy>
  void dumpScalarInOuts(raw_ostream &FOS, const LoopTy *L) const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  /// Non const getter for scalar in/out descriptors.
  ScalarInOutList *getOrCreateScalarLoopInOuts(const Loop *OrigLoop) {
    return &ScalarLoopsInOut[OrigLoop];
  }
  ScalarInOutListHIR *
  getOrCreateScalarLoopInOuts(const loopopt::HLLoop *OrigLoop) {
    return &ScalarLoopsInOutHIR[OrigLoop];
  }

  // Insert new ExternalUse into table.
  VPExternalUse *insertExternalUse(VPExternalUse *EUse, unsigned Id) {
    assert((Id == VPExternalUses.size() && EUse->getMergeId() == Id) &&
           "Inconsistent ExternalUse");
    VPExternalUses.emplace_back(EUse);
    return VPExternalUses.back().get();
  }
};

class VPLiveInOutCreator {
  VPlan &Plan;

public:
  using VPExtUseList = SmallVector<VPExternalUse *, 1>;

  VPLiveInOutCreator(VPlan &P) : Plan(P) {}

  /// Create VPLiveInValue and VPLiveOutValue lists for VPlan.
  /// Looking through VPEntities of VPlan create live-in counterparts and
  /// wrappers for all live-out, also adding fake VPExternalUse when needed.
  /// The original incoming vaules are replaced by the newly created
  /// VPLiveInValues and VPExternalUse-users are replaced by VPLiveOutValues.
  /// Also, descriptors of in/out values for scalar loops are created in VPlan.
  template <class LoopTy> void createInOutValues(LoopTy *OrigLoop);

  /// Replace all occurenses of VPLiveInValues with original incoming values.
  /// Temporary, until CFG merge process is not implemented. Then all such
  /// occurences are expected to be replaced during CFG merge process.
  void restoreLiveIns();

  /// Create VPLiveInValue-s list for VPlan that represents
  /// scalar loop. Going through the list of descriptors for a scalar loop,
  /// create live-ins of the appropriate type and update live-in list of VPlan.
  template <typename InOutListTy>
  void createLiveInsForScalarVPlan(const InOutListTy &ScalarInOuts, int Count);

  /// Create VPLiveOutValue-s list for VPlan that represents
  /// scalar loop. Going through the list of descriptors for a scalar loop,
  /// create live-outs with corresponding operands from \p Outgoing and update
  /// the live-out list in VPlan.
  template <typename InOutListTy>
  void createLiveOutsForScalarVPlan(const InOutListTy &ScalarInOuts, int Count,
                                    DenseMap<int, VPValue *> &Outgoing);

private:
  // Create list of VPLiveInValues/VPLiveOutValues for VPlan's inductions.
  template <class LoopTy>
  void createInOutsInductions(const VPLoopEntityList *VPLEntityList,
                              LoopTy *OrigLoop);

  // Create list of VPLiveInValues/VPLiveOutValues for VPlan's reductions.
  template <class LoopTy>
  void createInOutsReductions(const VPLoopEntityList *VPLEntityList,
                              LoopTy *OrigLoop);

  // Create list of VPLiveInValues/VPLiveOutValues for VPlan's privates.
  template <class LoopTy>
  void createInOutsPrivates(const VPLoopEntityList *VPLEntityList,
                            LoopTy *OrigLoop);

  // Create list of VPLiveInValues/VPLiveOutValues for VPlan's compress/expand
  // idioms.
  template <class LoopTy>
  void createInOutsCompressExpandIdioms(const VPLoopEntityList *VPLEntityList,
                                        LoopTy *OrigLoop);

  VPLiveInValue *createLiveInValue(unsigned Id, Type *Ty) {
    VPLiveInValue *LiveIn = new VPLiveInValue(Id, Ty);
    Twine Name = "livein.";
    LiveIn->setName(Name + Twine(Id));
    return LiveIn;
  }
  VPLiveOutValue *createLiveOutValue(unsigned Id, VPValue *Operand) {
    VPLiveOutValue *LiveOut = new VPLiveOutValue(Id, Operand);
    Twine Name = "liveout.";
    LiveOut->setName(Name + Twine(Id));
    return LiveOut;
  }
  template <class InitTy, class FinalTy>
  void addInOutValues(InitTy *Init, FinalTy *Final, VPExtUseList &ExtUse,
                      bool ExtUseAdded, VPValue *StartV);
  // Create and add descriptor of livein/out value in scalar loop.
  void addOriginalLiveInOut(const VPLoopEntityList *VPLEntityList,
                            Loop *OrigLoop, VPLoopEntity *E,
                            VPExtUseList &ExtUse, ScalarInOutList &SList);
  // Create and add descriptor of livein/out value in scalar HLLoop.
  void addOriginalLiveInOut(const VPLoopEntityList *, loopopt::HLLoop *OrigLoop,
                            VPLoopEntity *, VPExtUseList &ExtUse,
                            ScalarInOutListHIR &SList);
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_EXTERNALS_H
