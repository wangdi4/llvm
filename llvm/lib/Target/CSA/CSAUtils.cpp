//===- CSAUtils.cpp - Expand loop intrinsics --------===//
//
//===----------------------------------------------------------------===//
//
// This module has code common to multiple passes
//
//===----------------------------------------------------------------===//

#include "CSAUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "CSAMachineFunctionInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#define DEBUG_TYPE "csa-utils"

using namespace llvm;

static cl::opt<int>
AlwaysDataFlowLinkage("csa-df-calls", cl::Hidden, cl::ZeroOrMore,
                    cl::desc("CSA: Always use data flow linkage for passing parameters and results."),
                    cl::init(1));

bool csa_utils::isAlwaysDataFlowLinkageSet(void) {
	return AlwaysDataFlowLinkage;
}

static MachineInstr *getPriorFormedInst(MachineInstr *currMI, MachineBasicBlock *mbb) {
  LLVM_DEBUG(errs() << "currMI = " << *currMI << "\n");
  for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); I++) {
    MachineInstr *MI = &*I;
    if (MI == currMI) continue;
    if (MI->getNumOperands() != currMI->getNumOperands()) continue;
    if (MI->getOpcode() != currMI->getOpcode()) continue;
    bool allOpsSame = true;
    for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
      MachineOperand &op = MI->getOperand(i);
      MachineOperand &curr_op = currMI->getOperand(i);
      if (!op.isReg() || !curr_op.isReg()) {
        LLVM_DEBUG(errs() << "op #" << i << " is not reg\n");
        continue;
      }
      if (op.isDef()) continue;
      if (op.getReg() != curr_op.getReg()) {
        LLVM_DEBUG(errs() << "op #" << i << " is different\n");
        allOpsSame = false;
        break;
      }
    }
    if (allOpsSame) {
      LLVM_DEBUG(errs() << "Match found with MI = " << *MI << "\n");
      return MI;
    }
  }
  return currMI;
}

// Utility function to create a tree of uses. For example, it can create
// ANY/ALL/MERGE trees from a collection of operands. The "unusedReg" value
// defaults to IGN.
unsigned csa_utils::createUseTree(MachineBasicBlock *mbb, MachineBasicBlock::iterator before, unsigned opcode, const SmallVector<unsigned, 4> vals, unsigned unusedReg) {
  const TargetInstrInfo *TII = mbb->getParent()->getSubtarget().getInstrInfo();
  CSAMachineFunctionInfo *LMFI   = mbb->getParent()->getInfo<CSAMachineFunctionInfo>();
  const TargetRegisterInfo *TRI = mbb->getParent()->getSubtarget<CSASubtarget>().getRegisterInfo();
  unsigned n = vals.size();
  assert(n && "Can't combine 0 values");
  MCInstrDesc id = TII->get(opcode);
  // Ensure that this is an op that we can build some kind of 1-to-N tree out
  // of.
  assert(id.getNumDefs() == 1 && id.getNumOperands() > 2 &&
         "Don't know how to build a tree out of this opcode");
  unsigned radix                    = id.getNumOperands() - id.getNumDefs();
  if (opcode == CSA::ANY0) radix = radix - 1;
  
  const TargetRegisterClass *outTRC = LMFI->licRCFromGenRC(
    TII->getRegClass(id, 0, TRI, *mbb->getParent()));

  // If one left, we're done.
  if (n == 1)
    return vals[0];

  // Make our own set of vals to track.
  SmallVector<unsigned, 4> fewerVals;

  // Create a new element to combine some of the others.
  MachineInstrBuilder next =
    BuildMI(*mbb, before,
            before == mbb->end() ? DebugLoc() : before->getDebugLoc(),
            TII->get(opcode), LMFI->allocateLIC(outTRC));
  next.setMIFlag(MachineInstr::NonSequential);

  // vals will be partitioned into those items being combined and those items
  // which remain.
  for (unsigned j = 0; j < n; ++j) {
    if (j < 2)
      next.addReg(vals[j]);
    else
      fewerVals.push_back(vals[j]);
  }

  // Ensure the new instruction has enough ops
  while (next->getNumOperands() < radix + 1)
    next.addReg(unusedReg);
  if (opcode == CSA::ANY0) next.addImm(0); //default mode for any0
  MachineInstr *nextMI = &*next;
  LLVM_DEBUG(errs() << "Use Tree Instruction = " << *nextMI << "\n");
  MachineInstr *newNextMI = getPriorFormedInst(nextMI,mbb);
  if (nextMI == newNextMI) {
    // swap and try
    LLVM_DEBUG(errs() << "swap and try\n");
    unsigned temp = nextMI->getOperand(1).getReg(); 
    nextMI->getOperand(1).setReg(nextMI->getOperand(2).getReg());
    nextMI->getOperand(2).setReg(temp);
    newNextMI = getPriorFormedInst(nextMI,mbb);
  }
  if (nextMI != newNextMI) nextMI->eraseFromParent();
  // Add the new item to be combined. Putting this at the back of the list
  // helps balance.
  fewerVals.push_back(newNextMI->getOperand(0).getReg());

  // Run again on the smaller vector.
  return csa_utils::createUseTree(mbb, before, opcode, fewerVals, unusedReg);
}

unsigned csa_utils::createPickTree(MachineBasicBlock *mbb, MachineBasicBlock::iterator before, 
                                         const TargetRegisterClass *TRC, const SmallVector<unsigned, 4> select_signals, 
                                         const SmallVector<unsigned, 4> vals,  unsigned unusedReg) {
  const CSAInstrInfo *TII = static_cast<const CSAInstrInfo *>(mbb->getParent()->getSubtarget<CSASubtarget>().getInstrInfo());
  CSAMachineFunctionInfo *LMFI   = mbb->getParent()->getInfo<CSAMachineFunctionInfo>();
  
  const unsigned pickOpcode = TII->makeOpcode(CSA::Generic::PICK, TRC);
  const unsigned anyOpcode = CSA::ANY0;  
  
  unsigned n = vals.size();
  assert(n && "Can't combine 0 values");
  assert(vals.size() == select_signals.size());
  
  if (n == 1) return vals[0];
    
  if (n == 2) {
    unsigned pickedVal = LMFI->allocateLIC(TRC,"","",true,true);
    unsigned index = LMFI->allocateLIC(&CSA::CI1RegClass,"","",true,false);
    MachineInstr *anyInst =
      BuildMI(*mbb, before, DebugLoc(),TII->get(anyOpcode), index)
        .addReg(select_signals[0])
        .addReg(select_signals[1])
        .addReg(unusedReg)
        .addReg(unusedReg)
        .addImm(0);
    anyInst->setFlag(MachineInstr::NonSequential);
    LLVM_DEBUG(errs() << "anyInst = " << *anyInst << "\n");
    MachineInstr *newAnyInst = getPriorFormedInst(anyInst,mbb);
    unsigned val0 = vals[0]; unsigned val1 = vals[1];
    if (anyInst == newAnyInst) {
      // swap and try
      LLVM_DEBUG(errs() << "swap and try\n");
      unsigned temp = val0; val0 = val1; val1 = temp;
      temp = anyInst->getOperand(1).getReg(); 
      anyInst->getOperand(1).setReg(anyInst->getOperand(2).getReg());
      anyInst->getOperand(2).setReg(temp);
      newAnyInst = getPriorFormedInst(anyInst,mbb);
    }
    if (anyInst != newAnyInst) anyInst->eraseFromParent();
    MachineInstr *pickInst =
      BuildMI(*mbb, before, DebugLoc(),TII->get(pickOpcode), pickedVal)
        .addReg(newAnyInst->getOperand(0).getReg())
        .addReg(val0)
        .addReg(val1);
    pickInst->setFlag(MachineInstr::NonSequential);
    LLVM_DEBUG(errs() << "pickInst = " << *pickInst << "\n");
    return pickedVal;
  }
  
  SmallVector<unsigned, 4> select_signals_first_half;
  SmallVector<unsigned, 4> select_signals_second_half;
  SmallVector<unsigned, 4> vals_first_half;
  SmallVector<unsigned, 4> vals_second_half;
  for (unsigned i = 0; i < (vals.size()+1)/2; i++) {
    select_signals_first_half.push_back(select_signals[i]);
    vals_first_half.push_back(vals[i]);
    if (i+(vals.size()+1)/2 < vals.size()) {
      select_signals_second_half.push_back(select_signals[i+(vals.size()+1)/2]);
      vals_second_half.push_back(vals[i+(vals.size()+1)/2]);
    }
  }
  SmallVector<unsigned, 4> select_signals_pair;
  SmallVector<unsigned, 4> vals_pair;
  
  select_signals_pair.push_back(csa_utils::createUseTree(mbb, before, anyOpcode, select_signals_first_half, unusedReg));
  select_signals_pair.push_back(csa_utils::createUseTree(mbb, before, anyOpcode, select_signals_second_half, unusedReg));
  
  unsigned val0 = createPickTree(mbb,before,TRC,select_signals_first_half,vals_first_half,unusedReg);
  unsigned val1 = createPickTree(mbb,before,TRC,select_signals_second_half,vals_second_half,unusedReg);
  vals_pair.push_back(val0);
  vals_pair.push_back(val1);
  return csa_utils::createPickTree(mbb,before,TRC,select_signals_pair,vals_pair,unusedReg);
}

void csa_utils::createSwitchTree(MachineBasicBlock *mbb, MachineBasicBlock::iterator before, 
                                         const TargetRegisterClass *TRC, const SmallVector<unsigned, 4> select_signals, 
                                         SmallVector<unsigned, 4> &outvals, unsigned inval, 
                                         unsigned outvals_input_index, unsigned unusedReg) {
  const CSAInstrInfo *TII = static_cast<const CSAInstrInfo *>(mbb->getParent()->getSubtarget<CSASubtarget>().getInstrInfo());
  CSAMachineFunctionInfo *LMFI   = mbb->getParent()->getInfo<CSAMachineFunctionInfo>();
  
  const unsigned switchOpcode = TII->makeOpcode(CSA::Generic::SWITCH, TRC);
  const unsigned anyOpcode = CSA::ANY0;  
  
  unsigned n = select_signals.size();
  assert(n && "Can't combine 0 values");
  LLVM_DEBUG(errs() <<
             "outvals_input_index = " << outvals_input_index << "\n");
  if (n == 1) {
    MachineInstr *MI = BuildMI(*mbb, before, before->getDebugLoc(), TII->get(TII->getMoveOpcode(TRC)))
                                    .addReg(outvals[outvals_input_index],RegState::Define)
                                    .addReg(inval)
                                    .setMIFlag(MachineInstr::NonSequential);
    LLVM_DEBUG(errs() << "final switch mov MI = " << *MI << "\n");
    (void) MI;
    return;
  }
  SmallVector<unsigned, 4> select_signals_first_half;
  SmallVector<unsigned, 4> select_signals_second_half;
  unsigned outvals_first_index = outvals_input_index;
  unsigned outvals_second_index = outvals_input_index + (select_signals.size()+1)/2;
  for (unsigned i = 0; i < (select_signals.size()+1)/2; i++) {
    select_signals_first_half.push_back(select_signals[i]);
    if (i+(select_signals.size()+1)/2 < select_signals.size()) {
      select_signals_second_half.push_back(select_signals[i+(select_signals.size()+1)/2]);
    }
  }
  SmallVector<unsigned, 4> outvals_pair;
  unsigned select_signal_0 = csa_utils::createUseTree(mbb, before, anyOpcode, select_signals_first_half, unusedReg);
  unsigned select_signal_1 = csa_utils::createUseTree(mbb, before, anyOpcode, select_signals_second_half, unusedReg);
  
  unsigned outval0 = LMFI->allocateLIC(TRC,"","",true,true);
  unsigned outval1 = LMFI->allocateLIC(TRC,"","",true,true);
  unsigned index = LMFI->allocateLIC(&CSA::CI1RegClass,"","",true,true);
  
  MachineInstr *anyInst =
    BuildMI(*mbb, before, DebugLoc(),TII->get(anyOpcode), index)
      .addReg(select_signal_0)
      .addReg(select_signal_1)
      .addReg(unusedReg)
      .addReg(unusedReg)
      .addImm(0);
  anyInst->setFlag(MachineInstr::NonSequential);
  LLVM_DEBUG(errs() << "anyInst = " << *anyInst << "\n");
  MachineInstr *newAnyInst = getPriorFormedInst(anyInst,mbb);
  bool swapped = false;
  if (anyInst == newAnyInst) {
    // swap and try
    unsigned temp = outval0; outval0 = outval1; outval1 = temp;
    temp = anyInst->getOperand(1).getReg(); 
    anyInst->getOperand(1).setReg(anyInst->getOperand(2).getReg());
    anyInst->getOperand(2).setReg(temp);
    newAnyInst = getPriorFormedInst(anyInst,mbb);
    swapped = true;
  }
  if (anyInst != newAnyInst) anyInst->eraseFromParent();
  MachineInstr *switchInst =
    BuildMI(*mbb, before, DebugLoc(),TII->get(switchOpcode))
      .addReg(outval0,RegState::Define)
      .addReg(outval1,RegState::Define)
      .addReg(newAnyInst->getOperand(0).getReg())
      .addReg(inval);
  
  switchInst->setFlag(MachineInstr::NonSequential);
  LLVM_DEBUG(errs() << "switchInst = " << *switchInst << "\n");
  if (swapped) {
    createSwitchTree(mbb,before,TRC,select_signals_second_half,outvals,outval0,outvals_second_index,unusedReg);
    createSwitchTree(mbb,before,TRC,select_signals_first_half,outvals,outval1,outvals_first_index,unusedReg);
  } else {
    createSwitchTree(mbb,before,TRC,select_signals_first_half,outvals,outval0,outvals_first_index,unusedReg);
    createSwitchTree(mbb,before,TRC,select_signals_second_half,outvals,outval1,outvals_second_index,unusedReg);
  }
}
                        

