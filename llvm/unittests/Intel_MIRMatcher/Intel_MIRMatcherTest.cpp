//===-- Intel_MIRMatcherTest.cpp ----------------------------------*-C++-*-===//
//
//   Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Test the mirmatcher library.
///
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "../lib/Target/CSA/CSA.h"
#include "../lib/Target/CSA/CSARegisterInfo.h"
#include "../lib/Target/CSA/CSAMatcher.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace llvm;

namespace {

LLVMContext context{};
Module IRModule("TestModule", context);

TargetMachine*     TM      = nullptr;
MachineModuleInfo* MMI     = nullptr;

// Initialize the target machine and create a machine module.
// Note that these are never cleaned up.
bool initTargetMachine() {
  auto TT(Triple::normalize("csa"));
  std::string CPU("");
  std::string FS("");

  LLVMInitializeCSATargetInfo();
  LLVMInitializeCSATarget();
  LLVMInitializeCSATargetMC();

  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);
  if (! TheTarget) return false;

  TM = TheTarget->createTargetMachine(TT, CPU, FS, TargetOptions(), None,
                                      CodeModel::Large, CodeGenOpt::Default);
  if (! TM) return false;

  IRModule.setDataLayout(TM->createDataLayout());

  static MachineModuleInfo info(TM);
  info.doInitialization(IRModule);
  MMI = &info;

  return true;
}

// Construct a MachineFunction containing a basic block. Instructions
// are added to the end of the block.
class MIFuncBuilder {

  Function*              m_func;
  MachineFunction&       m_MIFunc;
  TargetInstrInfo const* m_TII;
  MachineBasicBlock*     m_BB;

  const MachineInstrBuilder& addOpnds(const MachineInstrBuilder& builder,
                                      unsigned numdefs)
    { return builder; }

  template <typename... Opnds>
  const MachineInstrBuilder& addOpnds(const MachineInstrBuilder& builder,
                                      unsigned numdefs,
                                      unsigned Reg1, const Opnds&... opnds) {
    if (numdefs > 0)
      return addOpnds(builder.addDef(Reg1), --numdefs, opnds...);
    else
      return addOpnds(builder.addUse(Reg1), 0, opnds...);
  }

  template <typename... Opnds>
  const MachineInstrBuilder& addOpnds(const MachineInstrBuilder& builder,
                                      unsigned numdefs,
                                      const MachineOperand& opnd1,
                                      const Opnds&... opnds) {
    if (numdefs > 0) --numdefs;
    return addOpnds(builder.add(opnd1), numdefs, opnds...);
  }

public:
  MIFuncBuilder(const char* name)
    : m_func(cast<Function>(
               IRModule.getOrInsertFunction(name,
                                            Type::getVoidTy(context))))
    , m_MIFunc(MMI->getOrCreateMachineFunction(*m_func))
    , m_TII(m_MIFunc.getSubtarget().getInstrInfo())
    , m_BB(m_MIFunc.CreateMachineBasicBlock()) { }

  ~MIFuncBuilder() {
    MMI->deleteMachineFunctionFor(*m_func);
  }

  Function& func()             const { return *m_func; }
  MachineFunction& MIFunc()    const { return m_MIFunc; }
  TargetInstrInfo const* TII() const { return m_TII; }
  MachineBasicBlock* BB()      const { return m_BB; }
  MachineRegisterInfo& MRI()   const { return m_MIFunc.getRegInfo(); }
  void dump()                  const { return m_BB->dump(); }

  int createVirtualRegister(const TargetRegisterClass *regClass) {
    return MRI().createVirtualRegister(regClass);
  }

  template <typename... Opnds>
  MachineInstr* addInstr(unsigned OpCode, const Opnds&... opnds) {
    const MCInstrDesc& instrDesc = m_TII->get(OpCode); // Capture1 reference!!
    MachineInstrBuilder bld = BuildMI(m_BB, DebugLoc{}, instrDesc);
    MachineInstr* ret = addOpnds(bld, instrDesc.getNumDefs(), opnds...);
    return ret;
  }
};

// For debugging. `PrintType<T>();` will cause the compilation to fail,
// printing `T` in the error message.
template <class T> struct PrintType;

} // Close anonymous namespace

MIRMATCHER_REGS(LICx_A, LIC1_B, LICx_C, X, LIC1_D, LICx_IN, LICx_OUT, Q1, Q2);
constexpr mirmatch::LiteralMatcher<bool, true>              TRUE_OPND{};
constexpr mirmatch::LiteralMatcher<bool, false>             FALSE_OPND{};

using namespace CSAMatch;

constexpr auto pat =
  mirmatch::graph(LICx_A      = csa_parallel_memdep(LICx_OUT)    ,
                  LIC1_D      = mov1(LIC1_B)                     ,
                  (LICx_C, X) = switch_N(LIC1_B, LICx_A)         ,
                  LIC1_D      = init1(TRUE_OPND)                 ,
                  LICx_IN     = pick_N(LIC1_D, LICx_C, X));

TEST(Intel_MIRMatcher, ComplexPattern) {
  using namespace llvm;

  MIFuncBuilder builder("foo");

  using MO = MachineOperand;

  const int ra   = builder.createVirtualRegister(&CSA::CI0RegClass);
  const int rc   = builder.createVirtualRegister(&CSA::CI0RegClass);
  const int rin  = builder.createVirtualRegister(&CSA::CI0RegClass);
  const int rout = builder.createVirtualRegister(&CSA::CI0RegClass);
  const int rx   = builder.createVirtualRegister(&CSA::CI0RegClass);
  const int rz   = builder.createVirtualRegister(&CSA::CI0RegClass);

  const int rb   = builder.createVirtualRegister(&CSA::CI1RegClass);
  const int rd   = builder.createVirtualRegister(&CSA::CI1RegClass);

  MachineInstr* cpm = builder.addInstr(CSA::CSA_PARALLEL_MEMDEP, ra, rout);
  MachineInstr* swx = builder.addInstr(CSA::SWITCH0, rc, rx, rb, ra);
  MachineInstr* swz = builder.addInstr(CSA::SWITCH0, rc, rx, rb, rz);
  MachineInstr* mv1 = builder.addInstr(CSA::MOV1, rd, rb);
  MachineInstr* ini = builder.addInstr(CSA::INIT1, rd, MO::CreateImm(1));
  MachineInstr* pkx = builder.addInstr(CSA::PICK0, rin, rd, rc, rx);
//  builder.dump();

  auto result = mirmatch::match(pat, cpm);
  ASSERT_TRUE(result);
  EXPECT_EQ(result.reg(LICx_A), ra);
  EXPECT_EQ(result.reg(LICx_OUT), rout);
  EXPECT_EQ(result.reg(LICx_C), rc);
  EXPECT_EQ(result.reg(LIC1_D), rd);
  EXPECT_EQ(result.reg(X), rx);
  EXPECT_EQ(result.reg(LICx_IN), rin);
  EXPECT_EQ(result.reg(LICx_OUT), rout);

  EXPECT_EQ(cpm, result.instr(LICx_A      = csa_parallel_memdep(LICx_OUT)));
  EXPECT_EQ(mv1, result.instr(LIC1_D      = mov1(LIC1_B)                 ));
  EXPECT_EQ(swx, result.instr((LICx_C, X) = switch_N(LIC1_B, LICx_A)     ));
  EXPECT_NE(swz, result.instr((LICx_C, X) = switch_N(LIC1_B, LICx_A)     ));
  EXPECT_EQ(ini, result.instr(LIC1_D      = init1(TRUE_OPND)             ));
  EXPECT_EQ(pkx, result.instr(LICx_IN     = pick_N(LIC1_D, LICx_C, X)    ));
}

TEST(Intel_MIRMatcher, FailMatch) {
  using namespace llvm;

  MIFuncBuilder builder("bar");

  using MO = MachineOperand;
  const int r = builder.createVirtualRegister(&CSA::CI1RegClass);
  MachineInstr* init = builder.addInstr(CSA::INIT1, r, MO::CreateImm(0));
  auto result = mirmatch::match(pat, init);
  ASSERT_FALSE(result);  // Won't match
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  if (initTargetMachine())
    return RUN_ALL_TESTS();
  else
    std::cerr << "FAILED to initialize target machine" << std::endl;
}

/* End MMatcher2.cpp */
