//===-- CSACreateSelfContainedGraph.cpp - CSA create self contained graphs-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass looks at entire module and creates self contained graphs
// for each offload region
//
//===----------------------------------------------------------------------===//

#include "CSASubtarget.h"
#include "CSA.h"
#include "CSATargetMachine.h"
#include "CSAMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/CallSite.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Analysis/CallGraph.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <sstream>

#include "CSAUtils.h"
using namespace llvm;

#define DEBUG_TYPE "csa-create-scg-pass"
#define REMARK_NAME "csa-create-scg-remark"
#define PASS_NAME "CSA: Create self contained graphs"

namespace {
struct CSACreateSelfContainedGraph : public ModulePass {
  static char ID;

  explicit CSACreateSelfContainedGraph() : ModulePass(ID) {}
  StringRef getPassName() const override { return PASS_NAME; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfo>();
    AU.addRequired<CallGraphWrapperPass>();
    ModulePass::getAnalysisUsage(AU);
  }
  bool runOnModule(Module &M) override;
private:
  MachineModuleInfo *MMI;
  bool IsOpenMPOffload;
  Module *thisMod;
  MachineInstr *mergeTwoDataFlowFunctions(MachineFunction *Base,
    MachineFunction *ToBeMerged);
  DenseSet<MachineFunction *> OffloadRegionRoots;
  DenseMap<MachineInstr *, MachineInstr *> EntryToReturnMap;
  void insertAllCalleesIntoRoot(MachineFunction *Root, Module &M,
    DenseMap<MachineInstr *, MachineFunction *> &CalledFunctions);
  void insertTrampolineCode(
    MachineFunction *TopMF, Module &M,
    DenseMap<MachineInstr *, MachineFunction *> &CalledFunctions);
  void processForOffloadCompile(Module &M);
  void processForManualCompile(Module &M);
  void getCallInstList(
    MachineFunction *TopMF, MachineFunction *CalleeMF, Module &M,
    SmallVectorImpl<MachineInstr *> &CallInstList);
  void insertTrampolineCode(
    MachineFunction *TopMF, MachineFunction *CalleeMF,
    ArrayRef<MachineInstr *> CallInstList, MachineInstr *EntryInst);
  void insertSimpleTrampolineCode(
    MachineFunction *TopMF, MachineInstr *CallInst,
    MachineInstr *EntryInst, MachineInstr *ReturnInst);
  void insertComplexTrampolineCode(
    MachineFunction *TopMF, ArrayRef<MachineInstr *> CallInstList,
    MachineInstr *EntryInst, MachineInstr *ReturnInst, bool HasExtEntry,
    MachineFunction *CalleeMF);
  void mergeAllFunctions(MachineFunction *TopMF, Module &M,
    DenseMap<MachineInstr *, MachineFunction *> &CalledFunctions);
};
} // namespace

char CSACreateSelfContainedGraph::ID = 0;

INITIALIZE_PASS_BEGIN(CSACreateSelfContainedGraph, DEBUG_TYPE, PASS_NAME,
  false, false)
INITIALIZE_PASS_DEPENDENCY(MachineModuleInfo)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(CSACreateSelfContainedGraph, DEBUG_TYPE, PASS_NAME,
  false, false)

Pass *llvm::createCSACreateSelfContainedGraphPass(void) {
  return new CSACreateSelfContainedGraph();
}

// Helper function to get callee IR function from a call instruction
const Function *getCalleeFunction(MachineInstr &MI, Module &M) {
  if (MI.getOpcode() != CSA::CSA_CALL)
    return nullptr;
  const MachineOperand &MO = MI.getOperand(0);
  if (MO.isSymbol() && M.getFunction(MO.getSymbolName()))
    return M.getFunction(MO.getSymbolName());
  return dyn_cast<Function>(MO.getGlobal());
}

// Helper function to get a pointer to callee MF from call instruction
MachineFunction *getCalleeMachineFunction(
  MachineInstr &MI, Module &M, MachineModuleInfo *MMI) {
  auto F = getCalleeFunction(MI, M);
  if (F)
    return MMI->getMachineFunction(*F);
  return nullptr;
}

// Check if this type of call is supported NOW
bool isCallSupported(MachineInstr &MI, Module &M, MachineModuleInfo *MMI) {
  auto CallerMF = MI.getParent()->getParent();
  auto CalleeMF = getCalleeMachineFunction(MI, M, MMI);
  if (!CalleeMF) { //proxy call
    errs() << "WARNING: Proxy calls not yet supported!"
           << "May generate code with incomplete linkage!\n";
    return false;
  }
  if (CallerMF->getSubtarget<CSASubtarget>().isSequential()
    && !CalleeMF->getSubtarget<CSASubtarget>().isSequential()) {
    errs() << "WARNING: calls from SXU code to DF code not supported!"
                 << "May generate code with wrong linkage!\n";
    return false;
  }
  if (!CallerMF->getSubtarget<CSASubtarget>().isSequential()
    && CalleeMF->getSubtarget<CSASubtarget>().isSequential()) {
    errs() << "WARNING: calls from DF code to SXU code not supported!"
                 << "May generate code with wrong linkage!\n";
    return false;
  }
  return true;
}

// Check if this type of call is supported NOW
bool hasUnsupportedCalls(Module &M, MachineModuleInfo *MMI) {
  bool HasUnsupportedCalls = false;
  for (auto &F : M) {
    if (F.isDeclaration()) continue;
    MachineFunction *MF = MMI->getMachineFunction(F);
    if (!MF) continue;
    for (auto &MBB : *MF) {
      for (auto &MI : MBB) {
        if (MI.getOpcode() == CSA::CSA_CALL)
          HasUnsupportedCalls |= !isCallSupported(MI,M,MMI);
      }
    }
  }
  return HasUnsupportedCalls;
}


bool isMallocPresent(MachineFunction *MF, Module &M, MachineModuleInfo *MMI) {
  Function *CSAMemAlloc = M.getFunction("csa_mem_alloc");
  if (CSAMemAlloc == nullptr) CSAMemAlloc = M.getFunction("CsaMemAlloc");
  if (CSAMemAlloc == nullptr) return false;
  MachineFunction *MallocMF = MMI->getMachineFunction(*CSAMemAlloc);
  if (MallocMF == nullptr) return false;
  for (auto &MBB : *MF) {
    for (auto &MI : MBB) {
      if (getCalleeMachineFunction(MI,M,MMI) == MallocMF)
        return true;
    }
  }
  return false;
}

// This function clones one instruction (MIToBeCopied)
// and inserts it at the end of DstMF
MachineInstr *copyMachineInstr(
    MachineFunction *DstMF, MachineInstr *MIToBeCopied) {
  MachineInstr *NewMI = DstMF->CloneMachineInstr(MIToBeCopied);
  MachineBasicBlock *MBB = &*(DstMF->begin());
  MBB->insert(MBB->end(),NewMI);
  NewMI->dropMemRefs(*DstMF);
  return NewMI;
}

// Here, instructions in source function needs to be copied to end
// of dst function
// This function prepares the source and destination functions for merging
// First, the number of virtual registers in each MF is made equal
// by creating appropriate number of dummy virtual registers
// Then, we replace all existing virtual registers in srcMF with a new set
// of virtual registers that do not exist in either of the two functions
// thus avoiding any overlaps in virtual register indices when the
// two functions are merged
void prepareForMerging(MachineFunction *DstMF, MachineFunction *SrcMF,
                       Module &M) {
  // create list of address spaces used for scratchpads
  // This will be used to replace constant pools with global variables
  // that will reside in scratchpads
  SmallSet<unsigned,16> ScratchPadAddrSpaces;
  for (const auto &GV: M.globals()) {
    if (isScratchpadAddressSpace(GV.getAddressSpace())) {
      ScratchPadAddrSpaces.insert(GV.getAddressSpace());
    }
  }
  MachineRegisterInfo *SrcMRI = &(SrcMF->getRegInfo());
  MachineRegisterInfo *DstMRI = &(DstMF->getRegInfo());
  const CSAInstrInfo *TII =
    static_cast<const CSAInstrInfo *>(SrcMF->getSubtarget().getInstrInfo());
  unsigned NumVirtRegsInSrc = SrcMRI->getNumVirtRegs();
  unsigned NumVirtRegsInDst = DstMRI->getNumVirtRegs();
  while (NumVirtRegsInSrc < NumVirtRegsInDst) {
    SrcMRI->createVirtualRegister(&CSA::CI0RegClass);
    NumVirtRegsInSrc++;
  }
  while (NumVirtRegsInDst < NumVirtRegsInSrc) {
    DstMRI->createVirtualRegister(&CSA::CI0RegClass);
    NumVirtRegsInDst++;
  }
  auto MBB = SrcMF->begin();
  for (auto &MI : *MBB) {
    if (TII->isInit(&MI)) continue;
    for (MachineOperand &MO : MI.operands()) {
      if (MO.isReg() && Register::isVirtualRegister(MO.getReg())) {
        if (MO.isDef()) {
          auto OldReg = MO.getReg();
          auto NewReg =
            SrcMRI->createVirtualRegister(SrcMRI->getRegClass((OldReg)));
          auto SrcLMFI = SrcMF->getInfo<CSAMachineFunctionInfo>();
          auto DstLMFI = DstMF->getInfo<CSAMachineFunctionInfo>();
          DstLMFI->getLICInfo(NewReg) = SrcLMFI->getLICInfo(OldReg);
          DstMRI->createVirtualRegister(SrcMRI->getRegClass((OldReg)));
          SrcMRI->replaceRegWith(OldReg,NewReg);
          SrcLMFI->getLICInfo(NewReg) = SrcLMFI->getLICInfo(OldReg);
        }
      }
      // MachineOperand points to a Constant Pool
      if (MO.isCPI()) {
        ArrayRef<MachineConstantPoolEntry> Constants =
          SrcMF->getConstantPool()->getConstants();
        const MachineConstantPoolEntry &ConstantEntry = Constants[MO.getIndex()];
        const Constant *C = ConstantEntry.Val.ConstVal;
        unsigned i;
        // Find an address space index not yet used for scratchpads
        for (i = 1024; i < 2047; ++i) {
          if (!ScratchPadAddrSpaces.count(i)) {
            ScratchPadAddrSpaces.insert(i);
            break;
          }
        }
        // Constant pool is being replaced by a global variable residing in
        // scratchpad
        // Name is set to be _spad_<address space index>
        auto *NewGV = new GlobalVariable(M,C->getType(),true,
                                         llvm::Function::InternalLinkage,
                                         const_cast <Constant *> (C),
                                         Twine("_spad_")+Twine(i),nullptr,
                                         llvm::GlobalValue::NotThreadLocal,i);
        MO.ChangeToGA(NewGV,0,0);
      }
    }
  }
  return;
}

// Insert all instructions from ToBeMerged at the end of Base
MachineInstr *CSACreateSelfContainedGraph::mergeTwoDataFlowFunctions(
    MachineFunction *Base, MachineFunction *ToBeMerged) {
  MachineInstr *NewEntryInst = nullptr;
  MachineInstr *NewReturnInst = nullptr;
  assert(Base->size() == 1 && ToBeMerged->size() == 1);
  auto MBB2 = ToBeMerged->begin();
  prepareForMerging(Base,ToBeMerged,*thisMod);
  for (MachineInstr &MI : *MBB2) {
    MachineInstr *NewMI = copyMachineInstr(Base,&MI);
    if (MI.getOpcode() == CSA::CSA_ENTRY)
      NewEntryInst = NewMI;
    if (MI.getOpcode() == CSA::CSA_RETURN)
      NewReturnInst = NewMI;
  }
  EntryToReturnMap[NewEntryInst] = NewReturnInst;
  return NewEntryInst;
}

// This function computes the list of call sites for a given function
// and returns it in CallInstList
void CSACreateSelfContainedGraph::getCallInstList(
    MachineFunction *TopMF, MachineFunction *CalleeMF, Module &M,
    SmallVectorImpl<MachineInstr *> &CallInstList) {
  assert(TopMF);
  for (auto &MBB : *TopMF) {
    for (auto &MI : MBB) {
      MachineFunction *MF = getCalleeMachineFunction(MI,M,MMI);
      if (MF && MF == CalleeMF)
        CallInstList.push_back(&MI);
    }
  }
  return;
}

// Top level function that copies body of all functions
// that reside inside the call graph tree for which Root is the root
// List of all functions (and their corresponding entry insts)
// are sent back in CalledFunctions
void CSACreateSelfContainedGraph::insertAllCalleesIntoRoot(
    MachineFunction *Root, Module &M,
    DenseMap<MachineInstr *, MachineFunction *> &CalledFunctions) {
  assert(Root);
  DenseSet<MachineFunction *> Funcs;
  Funcs.insert(Root);
  for (auto &MBB : *Root) {
    for (auto &MI : MBB) {
      if (MI.getOpcode() == CSA::CSA_CALL) {
        MachineFunction *CalleeMF = getCalleeMachineFunction(MI,M,MMI);
        if (CalleeMF) {
          if (Funcs.find(CalleeMF) == Funcs.end()) {
            Funcs.insert(CalleeMF);
            MachineInstr *NewEntryInst =
              mergeTwoDataFlowFunctions(Root, CalleeMF);
            CalledFunctions[NewEntryInst] = CalleeMF;
          }
        }
      }
    }
  }
}

// Snippet to add a trampoline start marker
void insertTrampolineStartPseudo(MachineBasicBlock *MBB,
  const CSAInstrInfo *TII) {
  BuildMI(MBB, MBB->getLastNonDebugInstr()->getDebugLoc(),
          TII->get(CSA::TRAMPOLINE_START))
          .setMIFlag(MachineInstr::NonSequential);
}

// Snippet to add a trampoline end marker
void insertTrampolineEndPseudo(MachineBasicBlock *MBB,
  const CSAInstrInfo *TII) {
  BuildMI(MBB, MBB->getLastNonDebugInstr()->getDebugLoc(),
          TII->get(CSA::TRAMPOLINE_END))
          .setMIFlag(MachineInstr::NonSequential);
}

// This function inserts trampoline code for cases where is
// one call site
void CSACreateSelfContainedGraph::insertSimpleTrampolineCode(
    MachineFunction *TopMF, MachineInstr *CallInst,
    MachineInstr *EntryInst, MachineInstr *ReturnInst) {
  MachineRegisterInfo *MRI = &(TopMF->getRegInfo());
  const CSAInstrInfo *TII =
    static_cast<const CSAInstrInfo *>(TopMF->getSubtarget().getInstrInfo());
  MachineInstr *ContinueInst = CallInst->getNextNode();
  MachineBasicBlock *MBB = EntryInst->getParent();
  insertTrampolineStartPseudo(MBB, TII);
  for (unsigned int i = 0; i < EntryInst->getNumOperands(); ++i) {
    assert(EntryInst->getOperand(i).isReg());
    assert(CallInst->getOperand(i+1).isReg());
    unsigned dst = EntryInst->getOperand(i).getReg();
    unsigned src = CallInst->getOperand(i+1).getReg();
    unsigned movOpcode = TII->getMoveOpcode(MRI->getRegClass(src));
    BuildMI(MBB, MBB->getLastNonDebugInstr()->getDebugLoc(),
            TII->get(movOpcode), dst)
            .addReg(src)
            .setMIFlag(MachineInstr::NonSequential);
  }
  for (unsigned int i = 0; i < ReturnInst->getNumOperands(); ++i) {
    unsigned src = ReturnInst->getOperand(i).getReg();
    unsigned dst = ContinueInst->getOperand(i).getReg();
    unsigned movOpcode = TII->getMoveOpcode(MRI->getRegClass(src));
    BuildMI(MBB, MBB->getLastNonDebugInstr()->getDebugLoc(),
            TII->get(movOpcode), dst)
            .addReg(src)
            .setMIFlag(MachineInstr::NonSequential);
  }
  insertTrampolineEndPseudo(MBB, TII);
  return;
}

// This generates a clone of the old instruction
// and, in addition, it also replaces all old registers
// with new registers
MachineInstr *getNewInst(
    MachineInstr *OldInst, MachineRegisterInfo *MRI, const CSAInstrInfo *TII) {
  MachineInstrBuilder MIB =
  BuildMI(*(OldInst->getParent()), OldInst, OldInst->getDebugLoc(),
          TII->get(OldInst->getOpcode()));
  for (unsigned int i = 0; i < OldInst->getNumOperands(); ++i) {
    assert(OldInst->getOperand(i).isReg());
    auto TRC = MRI->getRegClass(OldInst->getOperand(i).getReg());
    unsigned NewReg = MRI->createVirtualRegister(TRC);
    auto LMFI =
      OldInst->getParent()->getParent()->getInfo<CSAMachineFunctionInfo>();
    LMFI->getLICInfo(NewReg) =
      LMFI->getLICInfo(OldInst->getOperand(i).getReg());
    LMFI->setLICName(OldInst->getOperand(i).getReg(),"","");
    if (OldInst->getOperand(i).isDef())
      MIB.addReg(NewReg,RegState::Define);
    else
      MIB.addReg(NewReg);
  }
  MIB.setMIFlag(MachineInstr::NonSequential);
  return &*MIB;
}

// This function inserts trampoline code for cases where are more than
// one call site
void CSACreateSelfContainedGraph::insertComplexTrampolineCode(
    MachineFunction *TopMF, ArrayRef<MachineInstr *> CallInstList,
    MachineInstr *EntryInst, MachineInstr *ReturnInst, bool HasExtEntry,
    MachineFunction *CalleeMF) {
  MachineRegisterInfo *MRI = &(TopMF->getRegInfo());
  const CSAInstrInfo *TII =
    static_cast<const CSAInstrInfo *>(TopMF->getSubtarget().getInstrInfo());
  MachineBasicBlock *MBB = EntryInst->getParent();
  insertTrampolineStartPseudo(MBB, TII);
  SmallVector<unsigned, 4> select_signals;
  for (auto CallInst : CallInstList) {
    SmallVector<unsigned, 4> args;
    for (unsigned int i = 1; i < CallInst->getNumOperands(); ++i) {
      assert(CallInst->getOperand(i).isReg());
      unsigned arg = CallInst->getOperand(i).getReg();
      args.push_back(arg);
    }
    select_signals.push_back(
      csa_utils::createUseTree(
        MBB, MBB->end(), CSA::ALL0, args, CSA::IGN));
  }
  MachineInstr *NewEntryInst, *NewReturnInst;
  if (HasExtEntry) {
    // Add one more site for external call
    SmallVector<unsigned, 4> args;
    NewEntryInst = getNewInst(EntryInst, MRI, TII);
    NewReturnInst = getNewInst(ReturnInst, MRI, TII);
    for (MachineOperand &MO : NewEntryInst->operands()) {
      assert(MO.isReg());
      unsigned arg = MO.getReg();
      args.push_back(arg);
    }
    select_signals.push_back(
      csa_utils::createUseTree(
        MBB, MBB->end(), CSA::ALL0, args, CSA::IGN));
  }
  for (unsigned i = 0; i < EntryInst->getNumOperands(); ++i) {
    assert(EntryInst->getOperand(i).isReg());
    unsigned dst = EntryInst->getOperand(i).getReg();
    const TargetRegisterClass *TRC = MRI->getRegClass(dst);
    SmallVector<unsigned, 4> vals;
    for (auto CallInst : CallInstList)
      vals.push_back(CallInst->getOperand(i+1).getReg());
    if (HasExtEntry)
      vals.push_back(NewEntryInst->getOperand(i).getReg());
    unsigned src =
      csa_utils::createPickTree(
        MBB, MBB->end(), TRC, select_signals, vals, CSA::NA);
    unsigned movOpcode = TII->getMoveOpcode(TRC);
    BuildMI(MBB, MBB->getLastNonDebugInstr()->getDebugLoc(),
            TII->get(movOpcode), dst)
            .addReg(src)
            .setMIFlag(MachineInstr::NonSequential);
  }
  for (unsigned i = 0; i < ReturnInst->getNumOperands(); ++i) {
    unsigned src = ReturnInst->getOperand(i).getReg();
    const TargetRegisterClass *TRC = MRI->getRegClass(src);
    SmallVector<unsigned, 4> outvals;
    for (auto CallInst : CallInstList) {
      MachineBasicBlock::iterator I(CallInst);
      ++I;
      MachineInstr *ContinueInst = &*I;
      outvals.push_back(ContinueInst->getOperand(i).getReg());
    }
    if (HasExtEntry)
      outvals.push_back(NewReturnInst->getOperand(i).getReg());
    csa_utils::createSwitchTree(MBB, MBB->end(), TRC, select_signals, outvals,
                                  src, 0, CSA::NA);
  }
  insertTrampolineEndPseudo(MBB, TII);
  if (HasExtEntry) {
    auto LMFI = TopMF->getInfo<CSAMachineFunctionInfo>();
    LMFI->addCSAEntryPoint(CalleeMF, NewEntryInst, NewReturnInst);
  }
  return;
}

// This function inserts trampoline code for a given callee (CalleeMF)
// The code is inserted into the openmp offload root function (TopMF)
void CSACreateSelfContainedGraph::insertTrampolineCode(
    MachineFunction *TopMF, MachineFunction *CalleeMF,
    ArrayRef<MachineInstr *> CallInstList, MachineInstr *EntryInst) {
  assert(EntryInst);
  MachineInstr *ReturnInst = EntryToReturnMap[EntryInst];
  assert(ReturnInst);
  bool HasExtEntry = (!IsOpenMPOffload
    && (!CalleeMF->getFunction().hasLocalLinkage()));
  if (CallInstList.size() == 1 && !HasExtEntry)
    insertSimpleTrampolineCode(TopMF, CallInstList[0], EntryInst, ReturnInst);
  else
    insertComplexTrampolineCode(TopMF, CallInstList, EntryInst, ReturnInst,
                                HasExtEntry, CalleeMF);
  // Code clean up to remove all call site instructions
  // and entry/return instructions
  for (auto MI : CallInstList) {
    MachineBasicBlock::iterator I(MI);
    ++I;
    MachineInstr *ContinueMI = &*I;
    MI->eraseFromParent();
    ContinueMI->eraseFromParent();
  }
  if (CalleeMF->getFunction().hasLocalLinkage() || CallInstList.size()!=0) {
    EntryInst->eraseFromParent();
    ReturnInst->eraseFromParent();
  }
  return;
}

// This is a top level function that looks at an offload region root (TopMF)
// and inserts trampoline code between all caller/callee combinations
void CSACreateSelfContainedGraph::insertTrampolineCode(
    MachineFunction *TopMF, Module &M,
    DenseMap<MachineInstr *, MachineFunction *> &CalledFunctions) {
  for (auto Item: CalledFunctions) {
    MachineFunction *MF = Item.second;
    MachineInstr *EntryInst = Item.first;
    SmallVector<MachineInstr *, 4> CallInstList;
    getCallInstList(TopMF,MF,M,CallInstList);
    if (!CallInstList.empty())
      insertTrampolineCode(TopMF,MF,CallInstList,EntryInst);
    else { // This is function that is only being called externally
      auto LMFI = TopMF->getInfo<CSAMachineFunctionInfo>();
      MachineInstr *ReturnInst = EntryToReturnMap[EntryInst];
      LMFI->addCSAEntryPoint(MF, EntryInst, ReturnInst);
    }
  }
  return;
}

// Resolve cases where the same LIC is defined in entry instruction
// and used in result instruction
// This prevents asm printer from emitting the same LIC in .param list
// and result list
void avoidParamsResultsOverlap(MachineFunction *MF) {
  if (!MF) return;
  const CSAMachineFunctionInfo *LMFI = MF->getInfo<CSAMachineFunctionInfo>();
  if (!LMFI) return;
  MachineInstr *EntryMI = LMFI->getEntryMI();
  MachineInstr *ReturnMI = LMFI->getReturnMI();
  const CSAInstrInfo *TII =
    static_cast<const CSAInstrInfo *>(MF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &(MF->getRegInfo());
  for (MachineOperand &MO : EntryMI->operands()) {
    if (MO.isReg() && Register::isVirtualRegister(MO.getReg())) {
      auto Reg = MO.getReg();
      for (auto U = MRI->use_begin(Reg); U != MRI->use_end(); ++U) {
        MachineInstr *UI = U->getParent();
        MachineOperand *UMO = &*U;
        if (UI == ReturnMI) {
          unsigned movOpcode = TII->getMoveOpcode(MRI->getRegClass(Reg));
          auto NewReg = MRI->createVirtualRegister(MRI->getRegClass(Reg));
          auto LMFI = MF->getInfo<CSAMachineFunctionInfo>();
          LMFI->getLICInfo(NewReg) = LMFI->getLICInfo(Reg);
          BuildMI(*(EntryMI->getParent()), EntryMI,
                  EntryMI->getDebugLoc(), TII->get(movOpcode), NewReg)
                 .addReg(Reg)
                 .setMIFlag(MachineInstr::NonSequential);
          UMO->setReg(NewReg);
        }
      }
    }
  }
}

// This function returns the first externally callable function in M
MachineFunction *getManualOffloadFirstRoot(Module &M, MachineModuleInfo *MMI) {
  for (auto &F : M) {
    if (F.isDeclaration()) continue;
    if (F.hasLocalLinkage()) continue;
    MachineFunction *MF = MMI->getMachineFunction(F);
    if (!MF) continue;
    return MF;
  }
  LLVM_DEBUG(errs() << "No external function found\n");
  return nullptr;
}

// Top level function that copies body of all functions
// that reside inside the call graph tree for which MF is the root
// List of all functions (and their corresponding entry insts)
// are sent back in CalledFunctions
// This is used in manual offload model
void CSACreateSelfContainedGraph::mergeAllFunctions(
    MachineFunction *TopMF, Module &M,
    DenseMap<MachineInstr *, MachineFunction *> &CalledFunctions) {
  for (auto &F : M) {
    if (F.isDeclaration()) continue;
    MachineFunction *MF = MMI->getMachineFunction(F);
    if (!MF) continue;
    if (MF == TopMF) continue;
    MachineInstr *NewEntryInst = mergeTwoDataFlowFunctions(TopMF, MF);
    CalledFunctions[NewEntryInst] = MF;
  }
}

void CSACreateSelfContainedGraph::processForManualCompile(Module &M) {
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    MachineFunction *MF = MMI->getMachineFunction(F);
    if (!MF)
      continue;
    avoidParamsResultsOverlap(MF);
  }
  MachineFunction *TopMF = getManualOffloadFirstRoot(M,MMI);
  if (TopMF == nullptr) return;
  // Setting some properties of this MF for use in CSAAsmPrinter
  auto LMFI = TopMF->getInfo<CSAMachineFunctionInfo>();
  LMFI->setDoNotEmitAsm(false);
  LMFI->setNumCallSites(0);
  LMFI->addCSAEntryPoint(TopMF, LMFI->getEntryMI(), LMFI->getReturnMI());

  // Merge all function bodies into TopMF
  DenseMap<MachineInstr *, MachineFunction *> FunctionList;
  mergeAllFunctions(TopMF,M,FunctionList);
  // Insert trampoline code to connect all internal call sites with callees
  insertTrampolineCode(TopMF,M,FunctionList);
}

void CSACreateSelfContainedGraph::processForOffloadCompile(Module &M) {
  for (auto &F : M) {
    StringRef name = F.getName();
    MachineFunction *MF = MMI->getMachineFunction(F);
    avoidParamsResultsOverlap(MF);
    if (name.startswith("__omp_offloading") && !F.hasInternalLinkage())
      OffloadRegionRoots.insert(MF);
  }
  for (auto MF: OffloadRegionRoots) {
    // Setting some properties of this MF for use in CSAAsmPrinter
    auto LMFI = MF->getInfo<CSAMachineFunctionInfo>();
    LMFI->setDoNotEmitAsm(false);
    LMFI->setNumCallSites(0);
    LMFI->addCSAEntryPoint(MF, LMFI->getEntryMI(), LMFI->getReturnMI());
    // merge all function in the call tree of this offload region into root
    DenseMap<MachineInstr *, MachineFunction *> FunctionList;
    insertAllCalleesIntoRoot(MF,M,FunctionList);
    bool IsMallocPresent = isMallocPresent(MF, M, MMI);
    // Insert trampoline code to connect all internal call sites with callees
    insertTrampolineCode(MF,M,FunctionList);
    // Insert CSAMemInitialize if needed
    if (IsMallocPresent) {
      Function *CSAInitialize = M.getFunction("csa_mem_initialize");
      if (!CSAInitialize) CSAInitialize = M.getFunction("CsaMemInitialize");
      assert(CSAInitialize);
      MachineFunction *InitMF = MMI->getMachineFunction(*CSAInitialize);
      MachineInstr *NewEntryInst = mergeTwoDataFlowFunctions(MF, InitMF);
      LMFI->addCSAEntryPoint(InitMF, NewEntryInst,
                             EntryToReturnMap[NewEntryInst]);
    }
  }
}

// Check if this is OpenMP offload compilation
bool isOMPOffloadCompile(Module &M) {
  LLVM_DEBUG(dbgs() << "isOMPOffloadCompile\n");
  for (auto GVI = M.global_begin(), E = M.global_end(); GVI != E;) {
    GlobalVariable *GV = &*GVI++;
    LLVM_DEBUG(dbgs() << "Found GlobalVariable " << GV->getName() << "\n");
    // Presence of .omp_offloading.entries
    // section mens we are doing OMP offload compilation
    if (GV->hasSection()) {
      StringRef sectionName = GV->getSection();
      if (sectionName == ".omp_offloading.entries")
        return true;
    }
  }
  return false;
}

// cloned in CSAProcedureCalls.cpp
/********************************************************************
 * For initializer functions, memory ordering edges (param and result)
 * are handled in a special way.
 * If there are parameters other the memory ordering edge parameter,
 * Then the memory ordering edge is removed from parameter list and
 * its use inside the function is replaced by the narrow copy of the 
 * first non memory ordering edge parameter.
 * If there are results other the memory odering edge, then each result
 * is gated through the output memory ordering edge and the output memory
 *  ordering edge is removed from the list os results
 ********************************************************************/
void cleanupInitializerFunctions(Module &M, MachineModuleInfo *MMI) {
  LLVM_DEBUG(dbgs() << "cleanupInitializerFunctions\n");
  for (auto &F : M) {
    bool IsInitializer = F.hasFnAttribute("__csa_attr_initializer");
    if (!IsInitializer) continue;
    MachineFunction *MF = MMI->getMachineFunction(F);
    if (MF == nullptr) continue;
    MachineRegisterInfo *MRI = &(MF->getRegInfo());
    const CSAInstrInfo *TII =
      static_cast<const CSAInstrInfo *>(MF->getSubtarget().getInstrInfo());
    const CSAMachineFunctionInfo *LMFI = MF->getInfo<CSAMachineFunctionInfo>();
    assert(LMFI);
    MachineInstr *EntryMI = LMFI->getEntryMI();
    MachineInstr *ReturnMI = LMFI->getReturnMI();
    unsigned InMemoryLic = LMFI->getInMemoryLic();
    unsigned OutMemoryLic = LMFI->getOutMemoryLic();
    if (EntryMI->getNumOperands() > 1) { // Are regular params available
      auto Reg = InMemoryLic;
      auto NewReg = MRI->createVirtualRegister(MRI->getRegClass(Reg));
      MachineBasicBlock::iterator MII(EntryMI);
      MII++;
      BuildMI(*(EntryMI->getParent()), MII, EntryMI->getDebugLoc(), TII->get(CSA::MOV0))
             .addReg(NewReg, RegState::Define)
             .addReg(EntryMI->getOperand(1).getReg())
             .setMIFlag(MachineInstr::NonSequential);
      MRI->replaceRegWith(Reg, NewReg);
    }
    for (unsigned i = 1; i < ReturnMI->getNumOperands(); ++i) {
      auto Reg = ReturnMI->getOperand(i).getReg();
      auto NewReg = MRI->createVirtualRegister(MRI->getRegClass(Reg));
      BuildMI(*(ReturnMI->getParent()), ReturnMI, ReturnMI->getDebugLoc(), TII->get(TII->makeOpcode(CSA::Generic::GATE, MRI->getRegClass(Reg))))
             .addReg(NewReg, RegState::Define)
             .addReg(OutMemoryLic)
             .addReg(Reg)
             .setMIFlag(MachineInstr::NonSequential);
      ReturnMI->getOperand(i).setReg(NewReg);
    } 
    EntryMI->RemoveOperand(0);
    if (ReturnMI->getNumOperands() > 1)
      ReturnMI->RemoveOperand(0);
  }
}

bool CSACreateSelfContainedGraph::runOnModule(Module &M) {
  thisMod = &M;
  MMI = &getAnalysis<MachineModuleInfo>();
  OffloadRegionRoots.clear();
  EntryToReturnMap.clear();
  if (hasUnsupportedCalls(M,MMI))
    report_fatal_error("This module has unsupported calls");
  cleanupInitializerFunctions(M, MMI);
  IsOpenMPOffload = isOMPOffloadCompile(M);
  if (IsOpenMPOffload)
    processForOffloadCompile(M);
  else
    processForManualCompile(M);
  return true;
}
