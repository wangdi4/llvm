//====------ Intel_X86CFMA.cpp - Complex FMA optimization -----------------====
//
//      Copyright (c) 2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
//===----------------------------------------------------------------------===//
//
/// \file Pass is to optimize complex fma operation to reduce the data
//   dependency. It collects the list of cfma instructions and perform
///  transform for the list to reduce the register dependency. Below is
///  an example for the transform.
///            sum = +ab+cd+ef+gh+ij+kl;
///            -->
///            sum1 = ab+ef+ij
///            sum2 = cd+gh+kl
///            sum = sum1 + sum2
///
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86InstrInfo.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TileShapeInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "x86-cfma"

static cl::opt<unsigned>
    CFMAMin("x86-cfma-min", cl::desc("Min CFMA instructions for optimizing."),
            cl::init(2), cl::Hidden);

namespace {

struct X86CFMA : public MachineFunctionPass {
  using CFMAInfoT = std::pair<MachineInstr *, std::pair<unsigned, unsigned>>;

  MachineFunction *MF;
  MachineRegisterInfo *MRI;
  const TargetInstrInfo *TII;

  MachineInstr *createZero(MachineBasicBlock &MBB, MachineInstr *MI);

  MachineInstr *createAdd(MachineBasicBlock &MBB, MachineInstr *LMul,
                          MachineInstr *RMul);

  void morphCFMA(MachineBasicBlock &MBB, MachineInstr *CFMA);

  void morphCFMA(MachineInstr *PrevMI, MachineInstr *MI);

  X86CFMA() : MachineFunctionPass(ID) {}

  /// Return the pass name.
  StringRef getPassName() const override { return "Complex FMA Optimization"; }

  /// X86CFMA analysis usage.
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool optimizeBasicBlock(MachineBasicBlock &MBB);

  bool runOnMachineFunction(MachineFunction &MF) override;

  static char ID;
};

} // end anonymous namespace

char X86CFMA::ID = 0;

INITIALIZE_PASS_BEGIN(X86CFMA, DEBUG_TYPE, "Complex FMA Opt", false, false)
INITIALIZE_PASS_END(X86CFMA, DEBUG_TYPE, "Complex FMA Opt", false, false)

static std::pair<unsigned, unsigned> CFMAOps[] = {
    // packed conjugate complex FMA register operand
    {X86::VFCMADDCPHZ128r,    X86::VFCMULCPHZ128rr},
    {X86::VFCMADDCPHZ256r,    X86::VFCMULCPHZ256rr},
    {X86::VFCMADDCPHZr,       X86::VFCMULCPHZrr},
    // packed conjugate complex FMA memory operand
    {X86::VFCMADDCPHZ128m,    X86::VFCMULCPHZ128rm},
    {X86::VFCMADDCPHZ256m,    X86::VFCMULCPHZ256rm},
    {X86::VFCMADDCPHZm,       X86::VFCMULCPHZrm},
    // packed complex FMA register operand
    {X86::VFMADDCPHZ128r,     X86::VFMULCPHZ128rr},
    {X86::VFMADDCPHZ256r,     X86::VFMULCPHZ256rr},
    {X86::VFMADDCPHZr,        X86::VFMULCPHZrr},
    // packed complex FMA memory operand
    {X86::VFMADDCPHZ128m,     X86::VFMULCPHZ128rm},
    {X86::VFMADDCPHZ256m,     X86::VFMULCPHZ256rm},
    {X86::VFMADDCPHZm,        X86::VFMULCPHZrm}};
// TODO scalar complex FMA

static std::pair<unsigned, unsigned> *findCFMA(unsigned Opcode) {
  for (auto &Opc : CFMAOps)
    if (Opc.first == Opcode)
      return &Opc;
  return nullptr;
}

MachineInstr *X86CFMA::createAdd(MachineBasicBlock &MBB, MachineInstr *LMul,
                                 MachineInstr *RMul) {
  unsigned AddOpc = 0;
  auto *RC = MRI->getRegClass(LMul->getOperand(0).getReg());
  switch (RC->getID()) {
  default:
    return nullptr;
  case X86::VR128RegClassID:
  case X86::VR128XRegClassID:
    AddOpc = X86::VADDPHZ128rr;
    break;
  case X86::VR256RegClassID:
  case X86::VR256XRegClassID:
    AddOpc = X86::VADDPHZ256rr;
    break;
  case X86::VR512RegClassID:
  case X86::VR512_0_15RegClassID:
    AddOpc = X86::VADDPHZrr;
    break;
  }

  const DebugLoc &DL = RMul->getDebugLoc();
  Register SrcReg1 = LMul->getOperand(0).getReg();
  Register SrcReg2 = RMul->getOperand(0).getReg();
  Register DstReg = MRI->cloneVirtualRegister(SrcReg1);
  MachineInstr *Add =
      BuildMI(MBB, ++RMul->getIterator(), DL, TII->get(AddOpc), DstReg)
          .addReg(SrcReg1, RegState::Kill)
          .addReg(SrcReg2, RegState::Kill);
  Add->setFlags(RMul->getFlags());

  return Add;
}

MachineInstr *X86CFMA::createZero(MachineBasicBlock &MBB, MachineInstr *MI) {
  unsigned ZeroOpc = 0;
  auto *RC = MRI->getRegClass(MI->getOperand(0).getReg());
  switch (RC->getID()) {
  default:
    return nullptr;
  case X86::VR128RegClassID:
  case X86::VR128XRegClassID:
    ZeroOpc = X86::AVX512_128_SET0;
    break;
  case X86::VR256RegClassID:
  case X86::VR256XRegClassID:
    ZeroOpc = X86::AVX512_256_SET0;
    break;
  case X86::VR512RegClassID:
  case X86::VR512_0_15RegClassID:
    ZeroOpc = X86::AVX512_512_SET0;
    break;
  }
  const DebugLoc &DL = MI->getDebugLoc();
  Register ZeroReg = MRI->cloneVirtualRegister(MI->getOperand(0).getReg());

  MachineInstr *Zero =
      BuildMI(MBB, MI->getIterator(), DL, TII->get(ZeroOpc), ZeroReg);
  return Zero;
}

void X86CFMA::morphCFMA(MachineInstr *PrevMI, MachineInstr *MI) {
  MI->getOperand(1).setReg(PrevMI->getOperand(0).getReg());
}

void X86CFMA::morphCFMA(MachineBasicBlock &MBB, MachineInstr *CFMA) {
  MachineInstr *Zero = createZero(MBB, CFMA);
  morphCFMA(Zero, CFMA);
}

bool X86CFMA::optimizeBasicBlock(MachineBasicBlock &MBB) {
  bool Change = false;
  SmallVector<std::list<CFMAInfoT>, 4> CFMALists;
  SmallSet<MachineInstr *, 8> Visited;
  // step 1. Collect CFMA instructions.
  for (MachineInstr &MI : MBB) {
    auto *CFMAOp = findCFMA(MI.getOpcode());
    if (!CFMAOp)
      continue;
    // Already enqueued.
    if (Visited.count(&MI))
      continue;
    Visited.insert(&MI);
    // Create a new queue.
    std::list<CFMAInfoT> CFMAList;
    CFMAList.push_back(std::make_pair(&MI, *CFMAOp));
    auto &DestMO = MI.getOperand(0);
    Register DestReg = DestMO.getReg();
    // walk the use-def chain to find all the CFMA instructions in the MBB.
    while (MRI->hasOneNonDBGUse(DestReg)) {
      auto UI = MRI->use_nodbg_begin(DestReg);
      auto *UseMI = UI->getParent();
      if (!UseMI->isFast() || UseMI->getParent() != &MBB ||
          // TODO: the complex FMA, conjugate FMA, register version FMA,
          // memory version FMA, mask version FMA can be mixed in a chain.
          // We can fine tune it late.
          UseMI->getOpcode() != MI.getOpcode() ||
          UseMI->getOperand(1).getReg() != DestReg)
        break;
      CFMAList.push_back({UseMI, *CFMAOp});
      Visited.insert(UseMI);
      DestReg = UseMI->getOperand(0).getReg();
    }
    if (CFMAList.size() >= CFMAMin)
      CFMALists.push_back(CFMAList);
  }

  // If there are already several fma list, don't split it further.
  if (CFMALists.size() > 2)
    return false;

  if (!CFMALists.empty())
    Change = true;
  // step 2. Perform transform for each CFMA list.
  // sum = +ab+cd+ef+gh+ij+kl;
  // -->
  // sum1 = ab+ef+ij
  // sum2 = cd+gh+kl
  // sum = sum1 + sum2
  while (!CFMALists.empty()) {
    auto CFMAList = CFMALists.pop_back_val();
    // Remember the destination register, so that we can replace its user.
    Register DestReg = CFMAList.back().first->getOperand(0).getReg();
    MachineInstr *PrevMI[2] = { nullptr, nullptr };
    int I = 0;

    while (!CFMAList.empty()) {
      CFMAInfoT &CFMAI = CFMAList.front();
      MachineInstr *MI = CFMAI.first;
      if (I == 1)
        morphCFMA(MBB, MI);
      int Idx = I % 2;
      if (PrevMI[Idx])
        morphCFMA(PrevMI[Idx], MI);
      PrevMI[Idx] = MI;

      CFMAList.pop_front();
      I++;
    }
    assert(PrevMI[0] && PrevMI[1]);
    // RHS should be after LHS.
    MachineInstr *RHS = PrevMI[(--I) % 2];
    MachineInstr *LHS = PrevMI[(--I) % 2];
    auto *Add = createAdd(MBB, LHS, RHS);

    SmallVector<MachineOperand *> UseMOs;
    for (auto &MO : MRI->use_operands(DestReg)) {
      if (MO.getParent() == Add)
        continue;
      UseMOs.push_back(&MO);
    }
    assert(Add);
    for (auto *MO : UseMOs)
      MO->setReg(Add->getOperand(0).getReg());
  }

  return Change;
}

bool X86CFMA::runOnMachineFunction(MachineFunction &MFunc) {
  if (!MFunc.getTarget().Options.DoFMAOpt || skipFunction(MFunc.getFunction()))
    return false;
  // At least need 2 cfma to do interleave transform.
  if (CFMAMin < 2)
    return false;

  bool Change = false;
  MF = &MFunc;
  const X86Subtarget &ST = MF->getSubtarget<X86Subtarget>();
  TII = ST.getInstrInfo();
  MRI = &MF->getRegInfo();

  for (MachineBasicBlock &MBB : *MF)
    Change |= optimizeBasicBlock(MBB);

  return Change;
}

FunctionPass *llvm::createX86CFMAPass() { return new X86CFMA(); }
