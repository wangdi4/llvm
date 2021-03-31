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

HIRSpecifics::HIRSpecifics(const VPInstruction &Inst)
    : Inst(const_cast<VPInstruction &>(Inst)) {
  assert(
      (!HIRData().MasterData.isNull() || HIRData().MasterData.is<void *>()) &&
      "Defined state can't contain nullptr!");
}

const HIRSpecificsData &HIRSpecifics::HIRData() const { return Inst.HIRData; }
HIRSpecificsData &HIRSpecifics::HIRData() { return Inst.HIRData; }

MasterVPInstData *HIRSpecifics::getVPInstData() {
  if (isMaster())
    return HIRData().MasterData.get<MasterVPInstData *>();
  if (isDecomposed())
    return getMaster()->HIR().getVPInstData();
  // New VPInstructions don't have VPInstruction data.
  return nullptr;
}

void HIRSpecifics::cloneFrom(const HIRSpecifics HIR, bool CopySymbase) {
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

  if (CopySymbase)
    setSymbase(HIR.getSymbase());
  else
    setFoldIVConvert(HIR.getFoldIVConvert());

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
