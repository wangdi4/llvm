//===- IntelVPlanExternals.h - Represent VPlan external values storage ---===//
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

#include "IntelVPlanValue.h"

namespace llvm {
namespace vpo {

/// Class to hold external data used in VPlan. We can have several VPlans that
/// model different vectozation scenarios for one loop. Those VPlans are in most
/// cases clones of one created at the beginning of vectorization. All the
/// external definitions in these case are common and not needed to be cloned.
class VPExternalValues {
  // HIRDecomposer fixes VPExternalUses with multiple operands after building
  // initial plain CFG. Friendship is needed to get non-const iterator range to
  // VPExternalUsesHIR container.
  friend class VPDecomposerHIR;

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
    return map_range(
        make_range(VPExternalDefsHIR.begin(), VPExternalDefsHIR.end()),
        [](const VPExternalDef &Def) { return &Def; });
  }

  /// Holds all the external uses in this VPlan representing an underlying
  /// Value. The key is the underlying Value that uniquely identifies each
  /// external use.
  DenseMap<Value *, std::unique_ptr<VPExternalUse>> VPExternalUses;

  /// Holds all the external uses representing an HIR underlying entity
  /// in this VPlan. The key is the underlying HIR information that uniquely
  /// identifies each external use.
  FoldingSet<VPExternalUse> VPExternalUsesHIR;

  /// Return non-const iterator range for external uses in VPExternalUsesHIR.
  decltype(auto) getVPExternalUsesHIR() {
    return map_range(
        make_range(VPExternalUsesHIR.begin(), VPExternalUsesHIR.end()),
        [](VPExternalUse &Use) { return &Use; });
  }

  /// Holds all the VPMetadataAsValues created for this VPlan.
  DenseMap<MetadataAsValue *, std::unique_ptr<VPMetadataAsValue>>
      VPMetadataAsValues;

public:
  VPExternalValues(LLVMContext *Ctx, const DataLayout *L) : DL(L), Context(Ctx) {}
  VPExternalValues(const VPExternalValues &X) = delete;

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
    return getExternalItemForDDRef(VPExternalDefsHIR, DDR);
  }

  /// Create or retrieve a VPExternalDef for the blob with index \p BlobIndex in
  /// \p DDR.
  VPExternalDef *getVPExternalDefForBlob(const loopopt::RegDDRef *DDR,
                                         unsigned BlobIndex) {
    return getExternalItemForBlob(VPExternalDefsHIR, DDR, BlobIndex);
  }

  /// Create or retrieve a VPExternalDef for the given canon expression \p CE.
  VPExternalDef *getVPExternalDefForCanonExpr(const loopopt::CanonExpr *CE,
                                              const loopopt::RegDDRef *DDR) {
    return getExternalItemForCanonExpr(VPExternalDefsHIR, CE, DDR);
  }

  /// Retrieve the VPExternalDef for given HIR symbase \p Symbase. If no
  /// external definition exists then a nullptr is returned.
  VPExternalDef *getVPExternalDefForSymbase(unsigned Symbase) {
    return getExternalItemForSymbase(VPExternalDefsHIR, Symbase);
  }

  /// Create or retrieve a VPExternalDef for an HIR IV identified by its \p
  /// IVLevel.
  VPExternalDef *getVPExternalDefForIV(unsigned IVLevel, Type *BaseTy) {
    return getExternalItemForIV(VPExternalDefsHIR, IVLevel, BaseTy);
  }

  /// Create or retrieve a VPExternalUse for a given Value \p ExtVal.
  VPExternalUse *getVPExternalUse(PHINode *ExtDef) {
    return getExternalItem(VPExternalUses, ExtDef);
  }

  /// Create or retrieve a VPExternalUse for a given non-decomposable DDRef \p
  /// DDR.
  VPExternalUse *getVPExternalUseForDDRef(const loopopt::DDRef *DDR) {
    return getExternalItemForDDRef(VPExternalUsesHIR, DDR);
  }

  /// Create or retrieve a VPExternalUse for an HIR IV identified by its \p
  /// IVLevel.
  VPExternalUse *getVPExternalUseForIV(unsigned IVLevel, Type *BaseTy) {
    return getExternalItemForIV(VPExternalUsesHIR, IVLevel, BaseTy);
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
    // TODO: implement this method when needed.
    llvm_unreachable("Not implemented yet!");
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
  void dumpExternalDefs(raw_ostream &FOS);
  void dumpExternalUses(raw_ostream &FOS);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

private:
  // Create or retrieve an external item from \p Table for a given Value \p
  // ExtVal.
  template <typename T>
  typename T::mapped_type::element_type *getExternalItem(T &Table,
                                                         PHINode *ExtVal) {
    using Def = typename T::mapped_type::element_type;
    typename T::mapped_type &UPtr = Table[ExtVal];
    if (!UPtr)
      // Def is a new external item to be inserted in the map.
      UPtr.reset(new Def(ExtVal));
    return UPtr.get();
  }

  // Retrieve an external item from \p Table for given HIR symbase \p Symbase.
  // If no external item is found, then a nullptr is returned
  template <typename Def>
  Def *getExternalItemForSymbase(FoldingSet<Def> &Table, unsigned Symbase) {
    FoldingSetNodeID ID;
    ID.AddPointer(nullptr);
    ID.AddInteger(Symbase);
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (Def *ExtDef = Table.FindNodeOrInsertPos(ID, IP))
      return ExtDef;
    // No Def found in table
    return nullptr;
  }

  /// Create or retrieve an external item from \p Table for the blob with index
  /// \p BlobIndex in \p DDR.
  template <typename Def>
  Def *getExternalItemForBlob(FoldingSet<Def> &Table,
                              const loopopt::RegDDRef *DDR,
                              unsigned BlobIndex) {
    FoldingSetNodeID ID;

    // The blob(SCEV expression) with index BlobIndex is the folding set key.
    const loopopt::BlobTy BlobExpr = DDR->getBlobUtils().getBlob(BlobIndex);
    ID.AddPointer(BlobExpr);
    ID.AddInteger(0 /*SymBase*/);
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (Def *ExtDef = Table.FindNodeOrInsertPos(ID, IP))
      return ExtDef;

    Def *ExtDef = new Def(DDR, BlobIndex, BlobExpr->getType());
    Table.InsertNode(ExtDef);
    return ExtDef;
  }

  // Create or retrieve an external item from \p Table for given HIR canon
  // expression \p CE.
  template <typename Def>
  Def *getExternalItemForCanonExpr(FoldingSet<Def> &Table,
                                   const loopopt::CanonExpr *CE,
                                   const loopopt::RegDDRef *DDR) {
    // Search through the table for an external def equivalent to CE and return
    // the same if found. DDR is used to set the definedatlevel of blobs in CE
    // during vector code generation. The canon expression that we are dealing
    // with here is invariant at the loop level that is being vectorized.
    // The blobs within such canon expressions will have the same definedatlevel
    // independent of the DDR. When searching for an external def that is
    // equivalent we do not need any DDR specific checks as a result.
    auto Iter = llvm::find_if(Table, [CE](const Def &ExtDef) {
      return ExtDef.getOperandHIR()->isEqual(CE);
    });
    if (Iter != Table.end())
      return &*Iter;

    Def *ExtDef = new Def(CE, DDR);
    Table.InsertNode(ExtDef);
    return ExtDef;
  }

  // Create or retrieve an external item from \p Table for given HIR unitary
  // DDRef \p DDR.
  template <typename Def>
  Def *getExternalItemForDDRef(FoldingSet<Def> &Table,
                               const loopopt::DDRef *DDR) {
    assert(DDR->isNonDecomposable() && "Expected non-decomposable DDRef!");
    FoldingSetNodeID ID;
    ID.AddPointer(nullptr);
    ID.AddInteger(DDR->getSymbase());
    ID.AddInteger(0 /*IVLevel*/);
    void *IP = nullptr;
    if (Def *ExtDef = Table.FindNodeOrInsertPos(ID, IP))
      return ExtDef;
    Def *ExtDef = new Def(DDR);
    Table.InsertNode(ExtDef, IP);
    return ExtDef;
  }

  // Create or retrieve an external item from \p Table for an HIR IV identified
  // by its \p IVLevel.
  template <typename Def>
  Def *getExternalItemForIV(FoldingSet<Def> &Table, unsigned IVLevel,
                            Type *BaseTy) {
    FoldingSetNodeID ID;
    ID.AddPointer(nullptr);
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(IVLevel);
    void *IP = nullptr;
    if (Def *ExtDef = Table.FindNodeOrInsertPos(ID, IP))
      return ExtDef;
    Def *ExtDef = new Def(IVLevel, BaseTy);
    Table.InsertNode(ExtDef, IP);
    return ExtDef;
  }
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_EXTERNALS_H
