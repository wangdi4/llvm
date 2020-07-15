//===- IntelVPlanExternals.cpp - VPlan externals storage   ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the VPlan externals storage implementation.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanExternals.h"
#include "IntelVPlan.h"
#include "llvm/ADT/SmallSet.h"

using namespace llvm;
using namespace llvm::vpo;

void VPExternalValues::verifyVPConstants() const {
  SmallPtrSet<const Constant *, 16> ConstantSet;
  for (const auto &Pair : VPConstants) {
    const Constant *KeyConst = Pair.first;
    assert(KeyConst == Pair.second->getConstant() &&
           "Value key and VPConstant's underlying Constant must be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!ConstantSet.count(KeyConst) && "Repeated VPConstant!");
    ConstantSet.insert(KeyConst);
  }
}

void VPExternalValues::verifyVPExternalDefs() const {
  SmallPtrSet<const Value *, 16> ValueSet;
  for (const auto &Def : VPExternalDefs) {
    const Value *KeyVal = Def->getUnderlyingValue();
    assert(!ValueSet.count(KeyVal) && "Repeated VPExternalDef!");
    ValueSet.insert(KeyVal);
  }
}

void VPExternalValues::verifyVPExternalDefsHIR() const {
  SmallSet<unsigned, 16> SymbaseSet;
  SmallSet<unsigned, 16> IVLevelSet;
  for (const auto &ExtDef : VPExternalDefsHIR) {
    const VPOperandHIR *HIROperand = ExtDef.getOperandHIR();

    // Deeper verification depending on the kind of the underlying HIR operand.
    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      // For blobs we check that the symbases are unique.
      unsigned Symbase = Blob->getBlob()->getSymbase();
      assert(!SymbaseSet.count(Symbase) && "Repeated blob VPExternalDef!");
      SymbaseSet.insert(Symbase);
    } else if (isa<VPCanonExpr>(HIROperand)) {
      assert(
          llvm::count_if(VPExternalDefsHIR,
                         [HIROperand](const VPExternalDef &ExtDef) {
                           return ExtDef.getOperandHIR()->isStructurallyEqual(
                               HIROperand);
                         }) == 1 &&
          "Repeated CanonExpr VPExternalDef!");
    } else {
      // For IVs we check that the IV levels are unique.
      const auto *IV = cast<VPIndVar>(HIROperand);
      unsigned IVLevel = IV->getIVLevel();
      assert(!IVLevelSet.count(IVLevel) && "Repeated IV VPExternalDef!");
      IVLevelSet.insert(IVLevel);
    }
  }
}

void VPExternalValues::verifyVPMetadataAsValues() const {
  SmallPtrSet<const MetadataAsValue *, 16> MDAsValueSet;
  for (const auto &Pair : VPMetadataAsValues) {
    const MetadataAsValue *KeyMD = Pair.first;
    assert(KeyMD == Pair.second->getMetadataAsValue() &&
           "Value key and VPMetadataAsValue's underlying MetadataAsValue must "
           "be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!MDAsValueSet.count(KeyMD) && "Repeated MetadataAsValue!");
    MDAsValueSet.insert(KeyMD);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPExternalValues::dumpExternalDefs(raw_ostream &FOS) {
  if (VPExternalDefsHIR.size() == 0)
    return;
  FOS << "External Defs Start:\n";
  for (auto &Def : VPExternalDefsHIR)
    Def.printDetail(FOS);
  FOS << "External Defs End:\n";
}

void VPExternalValues::dumpExternalUses(raw_ostream &FOS) {
  if (VPExternalUses.empty())
    return;
  FOS << "External Uses:\n";
  for (auto &ExtUse : VPExternalUses) {
    ExtUse.second->dump(FOS);
    FOS << "\n";
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

