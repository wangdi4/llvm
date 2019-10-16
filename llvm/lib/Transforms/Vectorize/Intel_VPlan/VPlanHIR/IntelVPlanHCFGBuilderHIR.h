//===-- IntelVPlanHCFGBuilderHIR.h ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanHCFGBuilderHIR class which extends
/// VPlanHCFGBuilderBase with support to build a hierarchical CFG from HIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H

#include "../IntelVPlanHCFGBuilder.h"
#include "IntelVPlanVerifierHIR.h"

using namespace llvm::loopopt;

namespace llvm {

// Forward declarations
namespace loopopt {
class DDGraph;
class HLLoop;
class HIRSafeReductionAnalysis;
class HLInst;
template <class T> class VectorIdioms;
using HIRVectorIdioms = VectorIdioms<HLInst>;
extern void deleteHIRVectorIdioms(HIRVectorIdioms *);
class HIRDDAnalysis;
class RegDDRef;
class DDRef;
class DDRefUtils;
} // namespace loopopt

namespace vpo {

// High-level class to capture and provide loop vectorization legality analysis
// for incoming HIR. Currently various loop entities like reductions, inductions
// and privates are identified and stored within this class.
class HIRVectorizationLegality {
  using RecurrenceKind = RecurrenceDescriptor::RecurrenceKind;
  using MMRecurrenceKind = RecurrenceDescriptor::MinMaxRecurrenceKind;

public:
  struct CompareByDDRefSymbase {
    bool operator()(const DDRef *Ref1, const DDRef *Ref2) const {
      return Ref1->getSymbase() < Ref2->getSymbase();
    }
  };
  // Base class for descriptors which have init/finalize HIR values
  struct DescrValues {
    DescrValues(const DDRef *RefV) : Ref(RefV), InitValue(nullptr) {}
    // Move constructor
    DescrValues(DescrValues &&Other) = default;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void dump(raw_ostream &OS, unsigned Indent = 0) const {
      OS << "Ref: ";
      Ref->dump();
      OS << "\n";
      if (InitValue) {
        OS.indent(Indent + 2) << "InitValue: ";
        InitValue->dump();
        OS << "\n";
      }
      for (auto &V : UpdateInstructions) {
        OS.indent(Indent + 2) << "UpdateInstruction: ";
        V->dump();
      }
    }

    void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    const DDRef *Ref;
    // NOTE: InitValue holds only DDRefs for which VPExternalDefs were created
    // for a descriptor/alias. DDRefs with VPConstants are not accounted for.
    // Each descriptor/alias may have multiple updating HLInsts within the loop.
    const DDRef *InitValue;
    SmallVector<HLInst *, 4> UpdateInstructions;
  };
  // Base class for descriptors which may have alias DDRefs used within the loop
  // of incoming HIR. These descriptors are specific to HIR, so any analysis
  // which requires underyling HIR information must be done with these
  // descriptors. NOTE : Only original descriptors can have aliases and they are
  // always of the form &(%a)[0]
  struct DescrWithAliases : public DescrValues {
    DescrWithAliases(const RegDDRef *RefV) : DescrValues(RefV) {
      assert(RefV->isSelfAddressOf() && "Unexpected clause Ref!");
    }
    // Move constructor
    DescrWithAliases(DescrWithAliases &&Other) = default;

    // Filter out invalid aliases and return the valid one. If no valid alias is
    // found return nullptr.
    DescrValues *getValidAlias() const {
      DescrValues *ValidAlias = nullptr;
      for (auto &AliasItPair : Aliases) {
        DescrValues *Alias = AliasItPair.second.get();
        if (Alias->InitValue && Alias->UpdateInstructions.size() > 0) {
          assert(!ValidAlias &&
                 "HIRLegality descriptor has multiple valid aliases.");
          ValidAlias = Alias;
        }
      }

      return ValidAlias;
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    void dump(raw_ostream &OS) const {
      DescrValues::dump(OS);
      for (const auto &AliasIt : Aliases) {
        OS << "\n";
        OS.indent(2) << "Alias";
        AliasIt.second->dump(OS, 2);
      }
    }

    void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    std::map<const DDRef *, std::unique_ptr<DescrValues>, CompareByDDRefSymbase>
        Aliases;
  };
  // Specialized class to represent reduction descriptors specified explicitly
  // via SIMD reduction clause. The reduction's kind and signed datatype
  // information is also stored within this class.
  struct RedDescr : public DescrWithAliases {
    RedDescr(const RegDDRef *RegV, RecurrenceKind KindV,
             MMRecurrenceKind MMKindV, bool Signed)
        : DescrWithAliases(RegV), Kind(KindV), MMKind(MMKindV),
          IsSigned(Signed) {}
    // Move constructor
    RedDescr(RedDescr &&Other) = default;

    RecurrenceKind Kind;
    MMRecurrenceKind MMKind;
    bool IsSigned;
  };
  typedef SmallVector<RedDescr, 8> ReductionListTy;
  // Specialized class to represent private descriptors specified explicitly via
  // SIMD private clause.
  struct PrivDescr : public DescrWithAliases {
    PrivDescr(const RegDDRef *RegV, bool IsLastV, bool IsCondV)
        : DescrWithAliases(RegV), IsLast(IsLastV), IsCond(IsCondV) {}
    // Move constructor
    PrivDescr(PrivDescr &&Other) = default;

    bool IsLast;
    bool IsCond;
  };
  typedef SmallVector<PrivDescr, 8> PrivatesListTy;
  // Specialized class to represent linear descriptors specified explicitly via
  // SIMD linear clause. The linear's Step value is also stored within this
  // class.
  struct LinearDescr : public DescrWithAliases {
    LinearDescr(const RegDDRef *RegV, const RegDDRef *StepV)
        : DescrWithAliases(RegV), Step(StepV) {}
    // Move constructor
    LinearDescr(LinearDescr &&Other) = default;

    const DDRef *Step;
  };
  typedef SmallVector<LinearDescr, 8> LinearListTy;

  // Delete copy/assignment/move operations
  HIRVectorizationLegality(const HIRVectorizationLegality &) = delete;
  HIRVectorizationLegality &
  operator=(const HIRVectorizationLegality &) = delete;
  HIRVectorizationLegality(HIRVectorizationLegality &&) = delete;
  HIRVectorizationLegality &operator=(HIRVectorizationLegality &&) = delete;

  HIRVectorizationLegality(HIRSafeReductionAnalysis *SafeReds,
                           HIRDDAnalysis *DDA)
      : SRA(SafeReds), DDAnalysis(DDA) {}

  // Add explicit private.
  void addLoopPrivate(RegDDRef *PrivVal, bool IsLast = false,
                      bool IsConditional = false) {
    assert(PrivVal->isAddressOf() && "Private ref is not address of type.");
    PrivatesList.emplace_back(PrivVal, IsLast, IsConditional);
  }

  /// Register explicit reduction variables provided from outside.
  void addReductionMin(RegDDRef *V, bool IsSigned) {
    addReduction(V, RecurrenceKind::RK_IntegerMinMax, IsSigned,
                 IsSigned ? MMRecurrenceKind::MRK_SIntMin
                          : MMRecurrenceKind::MRK_UIntMin);
  }
  void addReductionMax(RegDDRef *V, bool IsSigned) {
    addReduction(V, RecurrenceKind::RK_IntegerMinMax, IsSigned,
                 IsSigned ? MMRecurrenceKind::MRK_SIntMax
                          : MMRecurrenceKind::MRK_UIntMax);
  }
  void addReductionAdd(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerAdd);
  }
  void addReductionMult(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerMult);
  }
  void addReductionAnd(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerAnd);
  }
  void addReductionXor(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerXor);
  }
  void addReductionOr(RegDDRef *V) {
    addReduction(V, RecurrenceKind::RK_IntegerOr);
  }

  // Add explicit linear.
  void addLinear(RegDDRef *LinearVal, RegDDRef *Step) {
    assert(LinearVal->isAddressOf() && "Linear ref is not address of type.");
    LinearList.emplace_back(LinearVal, Step);
  }

  HIRSafeReductionAnalysis *getSRA() const { return SRA; }
  const HIRVectorIdioms *getVectorIdioms(HLLoop *Loop) const;

  const PrivatesListTy &getPrivates() const { return PrivatesList; }
  const LinearListTy &getLinears() const { return LinearList; }
  const ReductionListTy &getReductions() const { return ReductionList; }

  PrivDescr *isPrivate(const DDRef *Ref) const {
    return findDescr<PrivDescr>(PrivatesList, Ref);
  }
  LinearDescr *isLinear(const DDRef *Ref) const {
    return findDescr<LinearDescr>(LinearList, Ref);
  }
  RedDescr *isReduction(const DDRef *Ref) const {
    return findDescr<RedDescr>(ReductionList, Ref);
  }

  /// Check if the given temp ref \p Ref is part of minmax+index idiom
  /// recognized for a loop \p HLoop.
  bool isMinMaxIdiomTemp(const DDRef *Ref, HLLoop *HLoop) const;

  /// Identify any DDRefs in the \p HLoop's pre-loop nodes which alias the OMP
  /// SIMD clause descriptor DDRefs
  void findAliasDDRefs(HLNode *ClauseNode, HLLoop *HLoop);

  /// Check if the given DDRef \p Ref corresponds to any linear/reduction
  /// HIRLegality descriptors. If found, then update the corresponding
  /// descriptor with \p Ref as its initialization value since it is directly
  /// used inside loop. NOTE: The default InitValue for all descriptors/aliases
  /// is nullptr since it may never be actually used within the loop.
  void recordPotentialSIMDDescrUse(DDRef *Ref);

  /// Check if the given HLInst \p UpdateInst writes in an LVal DDRef that
  /// potentially corresponds to any linear/reduction HIRLegality descriptors.
  /// If found, then update the descriptor with \p UpdateInst as its updating
  /// instruction.
  void recordPotentialSIMDDescrUpdate(HLInst *UpdateInst);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Debug print utility to display contents of the descriptor lists
  void dump(raw_ostream &OS) const;
  void dump() const { dump(errs()); }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  void addReduction(RegDDRef *V, RecurrenceKind Kind, bool IsSigned = false,
                    MMRecurrenceKind MMKind = MMRecurrenceKind::MRK_Invalid) {
    assert(V->isAddressOf() && "Reduction ref is not an address-of type.");
    ReductionList.emplace_back(V, Kind, MMKind, IsSigned);
  }

  /// Check if the given \p Ref is an explicit SIMD descriptor variable of type
  /// \p DescrType in the list \p List, if yes then return the descriptor object
  /// corresponding to it, else nullptr
  template <typename DescrType>
  DescrType *findDescr(ArrayRef<DescrType> List, const DDRef *Ref) const;

  /// Return the descriptor object corresponding to the input \p Ref, if it
  /// represents a reduction or linear SIMD variable (original or aliases). If
  /// \p Ref is not a SIMD descriptor variable nullptr is returned.
  DescrWithAliases *getLinearRednDescriptors(DDRef *Ref) {
    // Check if Ref is a linear descriptor
    DescrWithAliases *Descr = isLinear(Ref);

    // If Ref is not linear, check if it is a reduction variable
    if (!Descr)
      Descr = isReduction(Ref);

    return Descr;
  }

  HIRSafeReductionAnalysis *SRA;
  HIRDDAnalysis *DDAnalysis;
  PrivatesListTy PrivatesList;
  LinearListTy LinearList;
  ReductionListTy ReductionList;
  struct HIRVectorIdiomDeleter {
    void operator()(HIRVectorIdioms *p) { deleteHIRVectorIdioms(p); }
  };
  using IdiomListTy = std::unique_ptr<HIRVectorIdioms, HIRVectorIdiomDeleter>;
  // List of idioms recognized for each corresponding HLLoop.
  // NOTE: Map is made mutable since the const getter method repopulates the
  // list of idioms on the fly if no entry is found for a given loop. Check
  // getVectorIdioms(HLLoop*).
  mutable std::map<HLLoop *, IdiomListTy> VecIdioms;
};

class VPlanHCFGBuilderHIR : public VPlanHCFGBuilder {

private:
  /// The outermost loop to be vectorized.
  HLLoop *TheLoop;

  /// HIR DDGraph that contains DD information for the incoming loop nest.
  const DDGraph &DDG;

  HIRVectorizationLegality *HIRLegality;

  /// Loop header VPBasicBlock to HLLoop map. To be used when building loop
  /// regions.
  SmallDenseMap<VPBasicBlock *, HLLoop *, 4> Header2HLLoop;

  std::unique_ptr<VPRegionBlock>
  buildPlainCFG(VPLoopEntityConverterList &CvtVec) override;

  void populateVPLoopMetadata(VPLoopInfo *VPLInfo) override;

  void passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) override;

  void collectUniforms(VPRegionBlock *Region) override {
    // Do nothing for now
  }

public:
  VPlanHCFGBuilderHIR(const WRNVecLoopNode *WRL, HLLoop *Lp, VPlan *Plan,
                      HIRVectorizationLegality *Legality, const DDGraph &DDG);

  VPLoopRegion *createLoopRegion(VPLoop *VPLp) override;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANHCFGBUILDER_HIR_H
