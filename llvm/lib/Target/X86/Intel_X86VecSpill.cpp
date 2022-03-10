//====-- Intel_X86VecSpill.cpp ----------------====
//
//      Copyright (c) 2021 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file implements optimization vectorizing following scalar spill code:
//
//        movq    $0, 48(%rsp)
//        movq    $0, 40(%rsp)
//        movl    $0, 12(%rsp)
//        movq    $0, 32(%rsp)
//        movl    $0, 4(%rsp)
//        movq    $0, 24(%rsp)
//        movq    $0, 16(%rsp)
//        movl    $0, 8(%rsp)
//
// into following vectorized code:
//
//        vpxorq      %ymm16, %ymm16, %ymm16
//        vmovdqu64   %ymm16, 16(%rsp)
//        vmovdqa64   %xmm16, 48(%rsp)
//        movl        $0,     12(%rsp)
//
// The optimization performs at BB level and consists of 4 steps:
// 1) select candidates of spill instructions.
// 2) allocate unshared stack slots for selected spills so that they can be
//    contiguous to each others.
// 3) allocate contiguous stack objects for selected spills.
// 4) generate vectorized spills and remove orignal scalar spills.
//
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/LiveStacks.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "X86.h"
#include "X86Subtarget.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <vector>

using namespace llvm;
#define DEBUG_TYPE "x86-vec-spill"

static cl::opt<unsigned> VecSpillThreshold(
    "vec-spill-threshold", cl::Hidden, cl::init(16),
    cl::desc("threshold for vectorize spill"));
static cl::opt<unsigned> VecSpillFactor(
    "vec-spill-factor", cl::Hidden, cl::init(2),
    cl::desc("vectorize spill factor"));

namespace {

  class X86VecSpill : public MachineFunctionPass {
    LiveStacks* LS = nullptr;
    MachineFrameInfo *MFI = nullptr;
    const X86InstrInfo *TII = nullptr;
    const X86Subtarget *ST = nullptr;
    const TargetRegisterInfo *TRI = nullptr;
    RegisterClassInfo RegClassInfo;
    bool isVecSpillInst(const MachineInstr &MI) const;
    bool canVecSpill(const SmallVector<int, 8> &FIVec, MCPhysReg &Reg,
        BitVector &Select, const MachineBasicBlock &MBB) const;
  public:
    static char ID; // Pass identification

    X86VecSpill() : MachineFunctionPass(ID) {
      initializeX86VecSpillPass(*PassRegistry::getPassRegistry());
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesCFG();
      AU.addRequired<SlotIndexes>();
      AU.addPreserved<SlotIndexes>();
      AU.addRequired<LiveStacks>();
      AU.addPreserved<LiveStacks>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
  };

} // end anonymous namespace


char X86VecSpill::ID = 0;

INITIALIZE_PASS_BEGIN(X86VecSpill, DEBUG_TYPE,
                "Vectorize Spill", false, false)
INITIALIZE_PASS_DEPENDENCY(SlotIndexes)
INITIALIZE_PASS_DEPENDENCY(LiveStacks)
INITIALIZE_PASS_END(X86VecSpill, DEBUG_TYPE,
                "Vectorize Spill", false, false)

FunctionPass *llvm::createX86VecSpillPass() {
  return new X86VecSpill();
}

bool X86VecSpill::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  LS = &getAnalysis<LiveStacks>();
  unsigned NumSlots = LS->getNumIntervals();
  // Nothing to do!
  if (NumSlots == 0)
    return false;

  // If there are calls to setjmp or sigsetjmp
  if (MF.exposesReturnsTwice())
    return false;

  ST = &MF.getSubtarget<X86Subtarget>();
  if (!ST->hasAVX512())
    return false; // TODO (wxiao3) for AVX2

  LLVM_DEBUG({
    dbgs() << "********** X86 Vectorize Spill **********\n"
           << "********** Function: " << MF.getName() << '\n';
  });

  TRI = ST->getRegisterInfo();
  TII = ST->getInstrInfo();
  MFI = &MF.getFrameInfo();
  RegClassInfo.runOnMachineFunction(MF);
  uint8_t StackID = TargetStackID::Default;

  for (MachineBasicBlock &MBB : MF) {
    SmallVector<int, 8> FIVec;
    SmallSet<int, 8> BBVisitedFISet;
    for (MachineInstr &MI : MBB) {
      for (unsigned I = 0, E = MI.getNumOperands(); I != E; ++I) {
        MachineOperand &MO = MI.getOperand(I);
        if (!MO.isFI())
          continue;
        int FI = MO.getIndex();
        if (FI < 0)
          continue;
        if (!LS->hasInterval(FI))
          continue;
        if (MI.isDebugValue())
          continue;

        if (TII->isVecSpillInst(MI) &&
            !MFI->VecSpillSet.contains(FI) &&
            !BBVisitedFISet.contains(FI))
                FIVec.push_back(FI);

        if (BBVisitedFISet.contains(FI)) {
          // We can't take it as candidate since the FI is visited more
          // than once in this BB
          auto IT = std::find(FIVec.begin(), FIVec.end(), FI);
          if (IT != FIVec.end())
            FIVec.erase(IT);
        } else {
          BBVisitedFISet.insert(FI);
        }
      }
    }
    if (FIVec.begin() != FIVec.end()) {
      BitVector Select;
      MCPhysReg AllocReg = 0;
      if (canVecSpill(FIVec, AllocReg, Select, MBB)) {
        assert(AllocReg);
        int Sz = FIVec.size();
        if (StackID + Sz > TargetStackID::NoAlloc)
          continue; // Avoid StackID overflow
        StackID += Sz;
        MFI->VecSpillPhysRegMap[&MBB] = AllocReg;
        SmallVector<int, 8> &Vec = MFI->VecSpillMap[&MBB];
        for (int I = 0; I < Sz; I++)
          if (Select[I]) {
            int FI = FIVec[I];
            Vec.push_back(FI);
            MFI->VecSpillSet.insert(FI);
          }
      }
    }
  }

  if (StackID) {
    StackID = 0;
    using Pair = std::iterator_traits<LiveStacks::iterator>::value_type;
    SmallVector<Pair *, 16> Intervals;
    Intervals.reserve(LS->getNumIntervals());
    for (auto &I : *LS)
      Intervals.push_back(&I);
    llvm::sort(Intervals,
               [](Pair *LHS, Pair *RHS) { return LHS->first < RHS->first; });

    // Gather all spill slots into a list.
    LLVM_DEBUG(dbgs() << "Create Stack ID for vectorized spill slots:\n");
    for (auto *I : Intervals) {
      LiveInterval &LI = I->second;
      int FI = Register::stackSlot2Index(LI.reg());
      if (MFI->isDeadObjectIndex(FI))
        continue;

      if (MFI->VecSpillSet.contains(FI)) {
        MFI->setStackID(FI, ++StackID);
        LLVM_DEBUG(dbgs() << "StackID: " << (int)StackID << "\t"; LI.dump(););
      }
    }
    LLVM_DEBUG(dbgs() << '\n');
  }

  return false;
}

bool X86VecSpill::canVecSpill(const SmallVector<int, 8> &FIVec, MCPhysReg &Reg,
    BitVector &Select, const MachineBasicBlock &MBB) const {
  if (FIVec.size() < VecSpillThreshold)
    return false;

  unsigned int ZInsts;
  unsigned int YSize;
  unsigned int YInsts;
  unsigned int XSize;
  unsigned int XInsts;
  unsigned int FISize = FIVec.size();
  unsigned int OrigInstrNum = FISize;
  BitVector CurCandidate(FISize);
  Select.resize(FISize);

  unsigned int TotalSize = 0;
  SmallVector<unsigned int, 8> FISizeArr(FISize);
  for (unsigned int i = 0; i < FISize; i++) {
    FISizeArr[i] = MFI->getObjectSize(FIVec[i]);
    TotalSize += FISizeArr[i];
  }

  unsigned int AlignedTotalSize = alignDown(TotalSize, 16);
  // We can optimize MaxFINum to 1-dim but keep it as 2-dim for better debug
  SmallVector<SmallVector<int, 8>, 8>
    MaxFINum(FISize, SmallVector<int, 8>(AlignedTotalSize+1, -1));
  SmallVector<SmallVector<char, 8>, 8>
    SelFI(FISize, SmallVector<char, 8>(AlignedTotalSize+1, 0));

  // Initialize the first column
  for (unsigned int Row = 0; Row < FISize; ++Row)
    MaxFINum[Row][0] = 0; // Don't need to select anyone

  // Initialize the first row
  if (FISizeArr[0] <= AlignedTotalSize) {
    MaxFINum[0][FISizeArr[0]] = 1;
    SelFI[0][FISizeArr[0]] = 1;
  }

  for (unsigned int Row = 1; Row < FISize; ++Row)
    for (unsigned int Col = 1; Col < AlignedTotalSize+1; ++Col) {
      // Remaining size if select current FI
      int SelectRemSize = Col - FISizeArr[Row];
      // Current FI is not selected
      int UnselectValue = MaxFINum[Row - 1][Col];
      int SelectValue = -1;
      if (SelectRemSize >= 0 && MaxFINum[Row-1][SelectRemSize] != -1) {
        // There is solution if selecting current FI
        SelectValue = MaxFINum[Row-1][SelectRemSize] + 1;
      }
      if (SelectValue > UnselectValue) {
        // Select current FI
        MaxFINum[Row][Col] = SelectValue;
        SelFI[Row][Col] = 1;
      } else {
        // Not select current FI
        MaxFINum[Row][Col] = UnselectValue;
        SelFI[Row][Col] = 0;
      }
    }

  unsigned int SelInstrNum = MaxFINum[FISize-1][AlignedTotalSize];
  assert(SelInstrNum > 0 && SelInstrNum <= OrigInstrNum);
  unsigned int InstrNum = OrigInstrNum - SelInstrNum;
  if (ST->useAVX512Regs()) {
    ZInsts = AlignedTotalSize / 64;
    YSize  = AlignedTotalSize % 64;
  } else {
    ZInsts = 0;
    YSize  = AlignedTotalSize;
  }
  YInsts = YSize / 32;
  XSize = YSize % 32;
  XInsts = XSize / 16;
  assert(XSize % 16 == 0);
  InstrNum += ZInsts;
  InstrNum += YInsts;
  InstrNum += XInsts;

  if (OrigInstrNum < VecSpillFactor*InstrNum)
    return false;

  unsigned int Row = FISize - 1;
  unsigned int Col = AlignedTotalSize;
  while (SelInstrNum) {
      if (SelFI[Row][Col]) {
        Select.set(Row);
        Col -= FISizeArr[Row];
        Row--;
        SelInstrNum--;
      } else {
        Select.reset(Row);
        Row--;
      }
  }

  LLVM_DEBUG({
    dbgs() << "Optimal Vec Spill solution for MBB" << MBB.getNumber() << ":\n";
    for (unsigned int i = 0; i < FISize; i++) {
      dbgs() << i << " Slot: " << FIVec[i] << "\tSize: " <<
          MFI->getObjectSize(FIVec[i]);
      if (Select[i])
        dbgs() << "\t {vec}\n";
      else
        dbgs() << "\n";
    }
    dbgs() << "Spill#: " << OrigInstrNum << "\nVecSpill#: " << InstrNum << "\n";
    MBB.dump();
  });

  unsigned NumRegs = TRI->getNumRegs();
  BitVector PhysRegDefs(NumRegs); // Regs live into this BB.
  for (const auto &LI : MBB.liveins()) {
    for (MCRegAliasIterator AI(LI.PhysReg, TRI, true); AI.isValid(); ++AI)
      PhysRegDefs.set(*AI);
  }
  // Find a free register for zero. We only consider VR128XONLY (XMM16 - XMM31)
  // to avoid generating extra vzeroupper instructions.
  Reg = 0;
  for (MCPhysReg PhysReg : RegClassInfo.getOrder(&X86::VR128XRegClass)) {
    if (X86::VR128RegClass.contains(PhysReg))
      continue;
    if (!PhysRegDefs.test(PhysReg)) {
      Reg = PhysReg;
      break;
    }
  }

  if (!Reg)
    return false;
  return true;
}
