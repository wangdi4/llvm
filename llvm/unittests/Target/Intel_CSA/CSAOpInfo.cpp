//===-- CSAOpInfo.cpp ---------------------------------------------*-C++-*-===//
//
//   Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
#include "CSAInstrInfo.h"
#include "CSASubtarget.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "gtest/gtest.h"

using namespace llvm;

namespace {
std::unique_ptr<TargetMachine> createTargetMachine() {
  auto TT(Triple::normalize("csa--"));

  LLVMInitializeCSATargetInfo();
  LLVMInitializeCSATarget();
  LLVMInitializeCSATargetMC();

  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);

  return std::unique_ptr<TargetMachine>(
      TheTarget->createTargetMachine(TT, "", "", TargetOptions(), None,
                                     CodeModel::Large, CodeGenOpt::Default));
}

std::unique_ptr<CSAInstrInfo> createInstrInfo(TargetMachine *TM) {
  CSASubtarget ST(TM->getTargetTriple(), TM->getTargetCPU(),
                  TM->getTargetFeatureString(), *TM);
  return std::make_unique<CSAInstrInfo>(ST);
}

} // anonymous namespace

TEST(CSAOpInfo, GENERIC_INVERTIBLE) {
  std::unique_ptr<TargetMachine> TM = createTargetMachine();
  ASSERT_TRUE(TM);
  std::unique_ptr<CSAInstrInfo> II = createInstrInfo(TM.get());
  for (unsigned opcode = 0; opcode < CSA::INSTRUCTION_LIST_END; opcode++) {
    auto generic = II->getGenericOpcode(opcode);
    if (generic == CSA::Generic::INVALID_OP)
      continue;
    // TODO: Until the system is improved to handle conversions better, disable
    // these tests for CVT.
    if (generic == CSA::Generic::CVT)
      continue;
    unsigned reconstituted = II->makeOpcode(generic, II->getLicSize(opcode),
        II->getOpcodeClass(opcode));
    EXPECT_EQ(opcode, reconstituted) << II->getName(opcode) <<
      " was reconstituted as " << II->getName(reconstituted);
  }
}
