//===- IntelVPlanInstructionDataHIR.cpp -----------------------------------===//
//
//   Copyright (C) Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanInstructionDataHIR.h"
#include "../IntelVPlan.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

static bool hasSymbase(const VPInstruction &Inst) {
  auto Opcode = Inst.getOpcode();
  switch (Opcode) {
  default:
    return false;
  case Instruction::Store:
  case Instruction::Load:
  case VPInstruction::VLSLoad:
  case VPInstruction::VLSStore:
  case VPInstruction::Subscript:
    return true;
  }
}

static bool hasFakeSymbases(const VPInstruction &Inst) {
  auto Opcode = Inst.getOpcode();
  switch (Opcode) {
  default:
    return false;
  case VPInstruction::VLSLoad:
  case VPInstruction::VLSStore:
    return true;
  }
}

static bool canHaveFoldIVConvert(const VPInstruction &Inst) {
  auto Opcode = Inst.getOpcode();
  switch (Opcode) {
  default:
    return false;
  case Instruction::SExt:
  case Instruction::ZExt:
  case Instruction::Trunc:
    assert(!hasSymbase(Inst) &&
           "Can't have Symbase and FoldIVConvert simultaneously!");
    return true;
  }
}

HIRSpecificsData::HIRSpecificsData(const VPInstruction &Inst) {
  if (!hasSymbase(Inst))
    FoldIVConvert = false;
}

HIRSpecifics::HIRSpecifics(const VPInstruction &Inst)
    : Inst(const_cast<VPInstruction &>(Inst)) {
  assert(
      (!HIRData().ExtraData.isNull() || HIRData().ExtraData.is<void *>()) &&
      "Defined state can't contain nullptr!");
}

const HIRSpecificsData &HIRSpecifics::HIRData() const { return Inst.HIRData; }
HIRSpecificsData &HIRSpecifics::HIRData() { return Inst.HIRData; }

MasterVPInstData *HIRSpecifics::getVPInstData() {
  if (isMaster())
    return HIRData().ExtraData.get<MasterVPInstData *>();
  if (isDecomposed())
    return getMaster()->HIR().getVPInstData();
  // New VPInstructions don't have VPInstruction data.
  return nullptr;
}

void HIRSpecifics::setSymbase(unsigned SB) {
  assert(hasSymbase(Inst) && "This VPInstruction can't have a symbase!");
  HIRData().Symbase = SB;
}

unsigned HIRSpecifics::getSymbase() const {
  assert(hasSymbase(Inst) && "This VPInstruction can't have a symbase!");
  auto Symbase = HIRData().Symbase;
  return Symbase;
}

void HIRSpecifics::setFoldIVConvert(bool Fold) {
  assert((!Fold || canHaveFoldIVConvert(Inst)) &&
         "Unexpected call to setFoldIVConvert!");
  HIRData().FoldIVConvert = Fold;
}

bool HIRSpecifics::getFoldIVConvert() const {
  assert(canHaveFoldIVConvert(Inst) && "Unexpected call to getFoldIVConvert!");
  return HIRData().FoldIVConvert;
}

void HIRSpecifics::addFakeSymbase(unsigned Symbase) {
  if (Symbase == getSymbase())
    /// Skip if it's the main symbase.
    return;
  assert(hasFakeSymbases(Inst) &&
         "Can't add fake symbase to that kind of VPInstruction!");
  if (HIRData().ExtraData.isNull()) {
    HIRData().ExtraData = new HIRSpecificsData::FakeSymbases();
  }
  HIRData().ExtraData.get<HIRSpecificsData::FakeSymbases *>()->insert(Symbase);
}

ArrayRef<unsigned> HIRSpecifics::fakeSymbases() const {
  if (HIRData().ExtraData.isNull())
    return {};

  return HIRData()
      .ExtraData.get<HIRSpecificsData::FakeSymbases *>()
      ->getArrayRef();
}

void HIRSpecifics::cloneFrom(const HIRSpecifics HIR) {
  if (HIR.isMaster()) {
    setUnderlyingNode(HIR.getUnderlyingNode());
    if (HIR.isValid())
      setValid();
  } else if (HIR.isDecomposed())
    setMaster(HIR.getMaster());

  // Copy the operand.
  if (VPOperandHIR *HIROperand = HIR.getOperandHIR()) {
    if (VPBlob *Blob = dyn_cast<VPBlob>(HIROperand))
      setOperandDDR(Blob->getBlob());
    else {
      VPIndVar *IV = cast<VPIndVar>(HIROperand);
      setOperandIV(IV->getIVLevel());
    }
  }

  // Don't use getters/setters as Symbases might be invalid for LLVM IR path.
  if (hasSymbase(Inst))
    HIRData().Symbase = HIR.HIRData().Symbase;
  else
    HIRData().FoldIVConvert = HIR.HIRData().FoldIVConvert;

  // Verify correctness of the cloned HIR.
  assert(isMaster() == HIR.isMaster() &&
         "Cloned isMaster() value should be equal to the original one");
  assert(isDecomposed() == HIR.isDecomposed() &&
         "Cloned isDecomposed() value should be equal to the original one");
  assert(isSet() == HIR.isSet() &&
         "Cloned isSet() value should be equal to the original one");
  if (isSet())
    assert(HIR.isValid() == isValid() &&
           "Cloned isValid() value should be equal to the original one");
}

const VPOperandHIR *HIROperandSpecifics::getOperand() const {
  if (auto ExtUse = ExtObj.dyn_cast<const VPExternalUse *>())
    return ExtUse->getOperandHIR();
  return ExtObj.get<const VPExternalDef *>()->getOperandHIR();
}
