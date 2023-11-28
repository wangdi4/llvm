//===-- CSAProcedureCalls.cpp - CSA compilation of procedure calls --------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass looks at the list of functions inside a module.
// For each function, the linkage is identified (internal or external)
// For functions, with internal linkage, all the call-sites are identified
// This information will be used to generate code for procedure calls
// where the caller and callee will both be executed on CSA
//
//===----------------------------------------------------------------------===//

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <stack>
#include "CSA.h"
#include "InstPrinter/CSAInstPrinter.h"
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
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "CSAUtils.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/ADT/PostOrderIterator.h"

using namespace llvm;

static cl::opt<int>
ProcCallsPass("csa-proc-calls-pass",
          cl::Hidden, cl::ZeroOrMore,
          cl::desc("CSA Specific: Optimize data-flow to data-flow procedure calls"),
          cl::init(1));

#define DEBUG_TYPE "csa-proc-calls"

typedef struct call_site_info {
  StringRef caller_func;
  unsigned call_site_index;
  SmallVector<unsigned, 4> call_site_args;
  SmallVector<unsigned, 4> return_args;
} CALL_SITE_INFO;

namespace {
class CSAProcCallsPass : public MachineFunctionPass {
public:
  static char ID;
  CSAProcCallsPass() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return "CSA: Procedure Calls Pass"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    AU.addPreserved<MachineModuleInfoWrapperPass>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void addTrampolineCode(MachineInstr *,MachineInstr *);
  MachineInstr *addReturnInstruction(MachineInstr *);
  void processCallAndContinueInstructions(void);
  void changeMovConstToGate(unsigned int);
  void createMovInstAfter(MachineInstr *DefMI, unsigned oldreg, unsigned newreg);
  void createMovInstBefore(MachineInstr *DefMI, unsigned oldreg, unsigned newreg);
private:
  MachineFunction *thisMF;
  CSAMachineFunctionInfo *LMFI;
	MachineRegisterInfo *MRI;
  const CSAInstrInfo *TII;
protected:
  void getCallSiteArgs(CALL_SITE_INFO &, MachineInstr *, bool);
  void getReturnArgs(CALL_SITE_INFO &, MachineInstr *, bool);
  unsigned getParamReg(const std::string &, const std::string &, unsigned, unsigned,
                        const TargetRegisterClass *, bool, bool);
};
} // Close unnamed namespace

MachineFunctionPass *llvm::createCSAProcCallsPass() {
  return new CSAProcCallsPass();
}

char CSAProcCallsPass::ID = 0;

static
RegisterPass<CSAProcCallsPass> CSAOptRegistration("csaproccalls",
                                              "CSA Procedure Calls Pass",
                                              false, false);

static unsigned getGateOpcode(unsigned opcode) {
  switch (opcode) {
    case CSA::MOV0: return CSA::GATE0;
    case CSA::MOV1: return CSA::GATE1;
    case CSA::MOV8: return CSA::GATE8;
    case CSA::MOV16: return CSA::GATE16;
    case CSA::MOV32: return CSA::GATE32;
    case CSA::MOV64: return CSA::GATE64;
    default: return 0;
  }
}  

static unsigned getMoveOpcode(const TargetRegisterClass *RC) {
  switch (RC->getID()) {
  default:
    llvm_unreachable("Unknown Target register class!");
  case CSA::CI0RegClassID:
  case CSA::I0RegClassID:
  case CSA::RI0RegClassID:
    return CSA::MOV0;
  case CSA::CI1RegClassID:
  case CSA::I1RegClassID:
  case CSA::RI1RegClassID:
    return CSA::MOV1;
  case CSA::CI8RegClassID:
  case CSA::I8RegClassID:
  case CSA::RI8RegClassID:
    return CSA::MOV8;
  case CSA::CI16RegClassID:
  case CSA::I16RegClassID:
  case CSA::RI16RegClassID:
    return CSA::MOV16;
  case CSA::CI32RegClassID:
  case CSA::I32RegClassID:
  case CSA::RI32RegClassID:
    return CSA::MOV32;
  case CSA::CI64RegClassID:
  case CSA::I64RegClassID:
  case CSA::RI64RegClassID:
    return CSA::MOV64;
  }
}

static bool isMovOpcode(unsigned opcode) {
  switch (opcode) {
    case TargetOpcode::COPY:
    case CSA::MOV0: 
    case CSA::MOV1: 
    case CSA::MOV8: 
    case CSA::MOV16: 
    case CSA::MOV32: 
    case CSA::MOV64: return true;
    default: return false;
  }
}  

static int get_func_order(StringRef &name, const Module *M) {
  int id = 1;
  for (const Function &F : *M) {
    if (F.getName() == name) {
      LLVM_DEBUG(errs() << name << "::" << id << "\n");
      return id;
    }
    ++id;
  }
  return -1;
}

unsigned CSAProcCallsPass::getParamReg(const std::string &caller_func, const std::string &prefix, 
                                         unsigned call_site_index, unsigned param_index, 
                                         const TargetRegisterClass *TRC,
                                         bool isDeclared, bool isGloballyVisible) {
  std::stringstream ss1;
  if (prefix == "param_" || prefix == "result_") 
    ss1 << prefix << param_index << "_" << call_site_index;
  else
    ss1 << prefix << call_site_index;
  std::string str = ss1.str();
  StringRef name(str);
  return LMFI->allocateLIC(TRC, Twine(name), Twine(caller_func), isDeclared, isGloballyVisible);
}

void CSAProcCallsPass::getCallSiteArgs(CALL_SITE_INFO &csi,
                                       MachineInstr *entryMI, bool isDeclared) {
  csi.call_site_args.push_back(
      getParamReg(csi.caller_func.str(), "caller_out_mem_ord_",
                  csi.call_site_index, 0, &CSA::CI0RegClass, isDeclared, true));
  for (unsigned int i = 1; i < entryMI->getNumOperands(); ++i) {
    csi.call_site_args.push_back(
        getParamReg(csi.caller_func.str(), "param_", csi.call_site_index, i,
                    &CSA::CI64RegClass, isDeclared, true));
  }
}

void CSAProcCallsPass::getReturnArgs(CALL_SITE_INFO &csi,
                                     MachineInstr *returnMI, bool isDeclared) {
  csi.return_args.push_back(
      getParamReg(csi.caller_func.str(), "caller_in_mem_ord_",
                  csi.call_site_index, 0, &CSA::CI0RegClass, isDeclared, true));
  for (unsigned int i = 1; i < returnMI->getNumOperands(); ++i) {
    csi.return_args.push_back(
        getParamReg(csi.caller_func.str(), "result_", csi.call_site_index, i,
                    &CSA::CI64RegClass, isDeclared, true));
  }
}

// Given a CallInst. this function returns a handle to the callee function
// This function is duplicated in CSAReplaceAllocaWithMalloc pass
// Chnages will need to be synchronized
static const Function *getLoweredFunc(const CallInst *CI, const Module *M) {
  const Function *LowerF = nullptr;
  if (!CI->getCalledFunction()) return nullptr;
  auto F = CI->getCalledFunction();
  auto IID = F->getIntrinsicID();
  if (IID == Intrinsic::not_intrinsic) {
    if (F->isDeclaration()) return nullptr;
    LowerF = F;
  }
  LLVM_DEBUG(errs() << "CI = " << *CI << " and num ops = " << CI->getNumOperands() << "\n");
  if (CI->getNumArgOperands() == 0) return LowerF;
  bool IsFloat = (CI->getArgOperand(0)->getType()->getTypeID() == Type::FloatTyID);
  bool IsDouble = (CI->getArgOperand(0)->getType()->getTypeID() == Type::DoubleTyID);
  if (IsDouble) {
    switch (IID) {
    case Intrinsic::ceil: { LowerF = M->getFunction("ceil"); break; }
    case Intrinsic::cos: { LowerF = M->getFunction("cos"); break; }
    case Intrinsic::exp: { LowerF = M->getFunction("exp"); break; }
    case Intrinsic::exp2: { LowerF = M->getFunction("exp2"); break; }
    case Intrinsic::floor: { LowerF = M->getFunction("floor"); break; }
    case Intrinsic::log: { LowerF = M->getFunction("log"); break; }
    case Intrinsic::log2: { LowerF = M->getFunction("log2"); break; }
    case Intrinsic::log10: { LowerF = M->getFunction("log10"); break; }
    case Intrinsic::pow: { LowerF = M->getFunction("pow"); break; }
    case Intrinsic::round: { LowerF = M->getFunction("round"); break; }
    case Intrinsic::sin: { LowerF = M->getFunction("sin"); break; }
    case Intrinsic::trunc: { LowerF = M->getFunction("trunc"); break; }
    default: { break; }
    }
  } else if (IsFloat) {
    switch (IID) {
    case Intrinsic::ceil: { LowerF = M->getFunction("ceilf"); break; }
    case Intrinsic::cos: { LowerF = M->getFunction("cosf"); break; }
    case Intrinsic::exp: { LowerF = M->getFunction("expf"); break; }
    case Intrinsic::exp2: { LowerF = M->getFunction("exp2f"); break; }
    case Intrinsic::floor: { LowerF = M->getFunction("floorf"); break; }
    case Intrinsic::log: { LowerF = M->getFunction("logf"); break; }
    case Intrinsic::log2: { LowerF = M->getFunction("log2f"); break; }
    case Intrinsic::log10: { LowerF = M->getFunction("log10f"); break; }
    case Intrinsic::pow: { LowerF = M->getFunction("powf"); break; }
    case Intrinsic::round: { LowerF = M->getFunction("roundf"); break; }
    case Intrinsic::sin: { LowerF = M->getFunction("sinf"); break; }
    case Intrinsic::trunc: { LowerF = M->getFunction("truncf"); break; }
    default: { break; }
    }
  }
  return LowerF;
}

static void getCalleeName(StringRef &callee_name, MachineInstr *MI, const Module *M) {
  assert(MI->getOpcode() == CSA::CSA_CALL);
  LLVM_DEBUG(errs() << "Input Call MI = " << *MI << "\n");
  const MachineOperand &MO = MI->getOperand(0);
  // Report external calls here as either WARNINGs or FAILs depending on flag
  if (!MO.isGlobal()) {
    if (MO.isSymbol()) {
      if (M->getFunction(MO.getSymbolName())) {
        callee_name = MO.getSymbolName();
      }
    } else {
      if (csa_utils::reportWarningForExtCalls()) {
        callee_name = "dummy_func";
        errs() << "WARNING: Indirect calls not yet supported! May generate code with incomplete linkage!\n";
      } else
        report_fatal_error("Indirect calls not yet supported! Cannot be run on CSA!");
    }
  } else {
    const Function *F = dyn_cast<Function>(MO.getGlobal());
    if (!F) {
      if (MO.isSymbol()) {
        if (M->getFunction(MO.getSymbolName())) {
          callee_name = MO.getSymbolName();
        }
      } else {
        if (csa_utils::reportWarningForExtCalls()) {
          callee_name = "dummy_func";
          errs() << "WARNING: External calls not yet supported! May generate code with incomplete linkage!\n";
        } else
          report_fatal_error("External calls not yet supported! Cannot be run on CSA!");
      }
    } else
      callee_name = F->getName();
  }
  LLVM_DEBUG(errs() << "Called Function name = " << callee_name << "\n");
}

// Used to create MOV instruction after the CONTINUE instruction
void CSAProcCallsPass::createMovInstAfter(MachineInstr *DefMI, unsigned oldreg, unsigned newreg) {
  const TargetRegisterClass *RC = (oldreg == CSA::IGN) ? &CSA::CI0RegClass : MRI->getRegClass(oldreg);
  unsigned movopcode = TII->makeOpcode(CSA::Generic::MOV, RC);
  MachineInstrBuilder MIB = BuildMI(*(DefMI->getParent()), ++DefMI->getIterator(), DefMI->getDebugLoc(),
            TII->get(movopcode), oldreg)
           .addUse(newreg);
  MachineInstr *newInst = &*MIB;
  newInst->setFlag(MachineInstr::NonSequential);
}

// Used to create and add a MOV instruction before the CALL instruction
void CSAProcCallsPass::createMovInstBefore(MachineInstr *UseMI, unsigned oldreg, unsigned newreg) {
  const TargetRegisterClass *RC = (oldreg == CSA::IGN) ? &CSA::CI0RegClass : MRI->getRegClass(oldreg);
  unsigned movopcode = TII->makeOpcode(CSA::Generic::MOV, RC);
  MachineInstrBuilder MIB = BuildMI(*(UseMI->getParent()), UseMI, UseMI->getDebugLoc(),
            TII->get(movopcode), newreg)
           .addUse(oldreg);
 MachineInstr *newInst = &*MIB;
 newInst->setFlag(MachineInstr::NonSequential);
}

// Here, LICs associated with CALL instructions and CONTINUE instructions
// are given meaningful names. THe same set of names will be used when
// trampoline code is generated when the callee function is being processed
// MOV instructions are used to 'isolate' the name from each other.
// For example, if the same variable is used in two call-sites, we generate two
// different LICs for this variable and use MOV instruction to connect the old variable
// to the new LIC
// Two helper functions - createMovInstBefore and createMovInstAfter are used
void CSAProcCallsPass::processCallAndContinueInstructions(void) {
  int CallSiteIndex = 0;
  MachineModuleInfo &MMI = thisMF->getMMI();
  const Module *M = MMI.getModule();
  StringRef name = thisMF->getFunction().getName();
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(), EI = BB->end(); I != EI; ++I) {
      MachineInstr *CallMI = &*I;
      MachineInstr *ContinueMI;
      if (CallMI->getOpcode() == CSA::CSA_CALL) {
        ++I;
        ContinueMI = &*I;
      } else
        continue;
      CallSiteIndex++;
      StringRef callee_name;
      getCalleeName(callee_name, CallMI, M);
      if (callee_name== name) {
        errs() << "Called Function name = " << callee_name << "\n";
        report_fatal_error("Recursive function call not yet supported! Cannot be run on CSA!");
      }
      // The LIC will be declared in callee code or caller code depending on which comes first in the module
      int callee_id = get_func_order(callee_name, M);
      int caller_id = get_func_order(name, M);
      bool isDeclared = (callee_id > caller_id);
      unsigned newreg =
          getParamReg(name.str(), "caller_out_mem_ord_", CallSiteIndex, 0,
                      &CSA::CI0RegClass, isDeclared, true);
      unsigned oldreg = CallMI->getOperand(1).getReg();
      CallMI->getOperand(1).setReg(newreg);
      createMovInstBefore(CallMI, oldreg, newreg);
      for (unsigned int i = 2; i < CallMI->getNumOperands(); ++i) {
        unsigned oldreg = CallMI->getOperand(i).getReg();
        unsigned newreg =
            getParamReg(name.str(), "param_", CallSiteIndex, i - 1,
                        MRI->getRegClass(oldreg), isDeclared, true);
        CallMI->getOperand(i).setReg(newreg);
        createMovInstBefore(CallMI, oldreg, newreg);
      }
      oldreg = ContinueMI->getOperand(0).getReg();
      newreg = getParamReg(name.str(), "caller_in_mem_ord_", CallSiteIndex, 0,
                           &CSA::CI0RegClass, isDeclared, true);
      ContinueMI->getOperand(0).setReg(newreg);
      createMovInstAfter(ContinueMI, oldreg, newreg);
      for (unsigned int i = 1; i < ContinueMI->getNumOperands(); ++i) {
        unsigned oldreg = ContinueMI->getOperand(i).getReg();
        const TargetRegisterClass *RC =
            (oldreg == CSA::IGN) ? &CSA::CI0RegClass : MRI->getRegClass(oldreg);
        unsigned newreg = getParamReg(name.str(), "result_", CallSiteIndex, 1,
                                      RC, isDeclared, true);
        ContinueMI->getOperand(i).setReg(newreg);
        createMovInstAfter(ContinueMI, oldreg, newreg);
      }
    }
  }
}

// For a given function, this function identifies all its call-sites and connects the arguments
// in its entryMI instruction with the parameters from various call-sites via pick-tree.
// Also, all the results in the returnMI instruction are sent back to the call-sites via a switch tree
void CSAProcCallsPass::addTrampolineCode(MachineInstr *entryMI, MachineInstr *returnMI) {
  // Check for all call-sites of current function in the module.
  MachineInstrBuilder MIB;
  MachineModuleInfo &MMI = thisMF->getMMI();
  const Module *M = MMI.getModule();
  int num_call_sites = 0;
  StringRef name = thisMF->getFunction().getName();
  std::vector<CALL_SITE_INFO> csilist;
  for (const Function &F : *M) {
    LLVM_DEBUG(errs() << "function = " << F.getName() << "\n");
    if (F.isDeclaration()) continue;
    int index = 1;
    typedef po_iterator<const BasicBlock *> po_cfg_iterator;
    const BasicBlock *root = &*F.begin();
    std::stack<const BasicBlock *> postk;
    // In DF conversion, the BBs are reordered this way. It is required that we maintain a 1-1 parsing
    // of BBs in addTrampolineCode (parses in IR level; pre DF conversion) and addCallAndContinueInstructions
    // (parses in Machine IR level; post DF conversion)
    for (po_cfg_iterator iterbb = po_cfg_iterator::begin(root),
                       END     = po_cfg_iterator::end(root);
            iterbb != END; ++iterbb) {
      const BasicBlock *bb = *iterbb;
      postk.push(bb);
    }
    while (!postk.empty()) {
      const BasicBlock &BB = *(postk.top());
      postk.pop();
      for (const Instruction &I : BB) {
        // call-site parameters are named based on their caller function names and the call-site index
        // This indexing accounts for only internal calls;
        if (const CallInst *CI = dyn_cast<const CallInst>(&I)) {
          LLVM_DEBUG(errs() << "In AddTrampolineCode: CallInst = " << I << "\n");
          auto callee_func = getLoweredFunc(CI,M);
          if (callee_func == nullptr) continue;
          if (name == callee_func->getName()) {
            CALL_SITE_INFO csi;
            csi.caller_func = F.getName();
            bool isDeclared = (get_func_order(name,M) < get_func_order(csi.caller_func,M));
            csi.call_site_index = index;
            getCallSiteArgs(csi, entryMI, isDeclared);
            getReturnArgs(csi, returnMI, isDeclared);
            csilist.push_back(csi);
            num_call_sites++;
            LLVM_DEBUG(errs() << "In AddTrampolineCode: " << name << " is called by " << csi.caller_func << " at index " << csi.call_site_index << "\n");
          }
          index++;
        }
      }
    }
  }
  LLVM_DEBUG(errs() << "Number of call sites for " << name << " is " << num_call_sites << "\n");
  LMFI->setNumCallSites(num_call_sites);
  LMFI->setDoNotEmitAsm(false);
  if (num_call_sites == 0) {
    LMFI->addCSAEntryPoint(thisMF, entryMI, returnMI);
  }
  if (!num_call_sites) return;
  const TargetInstrInfo *TII = thisMF->getSubtarget().getInstrInfo();
  // Special case: need only movs
  if (num_call_sites == 1) {
    MIB = BuildMI(*(entryMI->getParent()), entryMI, entryMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_START))
                                   .setMIFlag(MachineInstr::NonSequential);
    auto csi = csilist[0];
    assert(csi.call_site_args.size() == entryMI->getNumOperands());
    for (unsigned int i = 0; i < entryMI->getNumOperands(); ++i) {
      unsigned dst = entryMI->getOperand(i).getReg();
      unsigned src = csi.call_site_args[i];
      unsigned movOpcode = getMoveOpcode(MRI->getRegClass(src));
      MIB = BuildMI(*(entryMI->getParent()), entryMI, entryMI->getDebugLoc(), TII->get(movOpcode))
                                    .addReg(dst,RegState::Define)
                                    .addReg(src)
                                    .setMIFlag(MachineInstr::NonSequential);
    }
    MIB = BuildMI(*(entryMI->getParent()), entryMI, entryMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_END))
                 .setMIFlag(MachineInstr::NonSequential);
    MIB = BuildMI(*(returnMI->getParent()), returnMI, returnMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_START))
                 .setMIFlag(MachineInstr::NonSequential);
    assert(csi.return_args.size() == returnMI->getNumOperands());
    for (unsigned int i = 0; i < returnMI->getNumOperands(); ++i) {
      unsigned src = returnMI->getOperand(i).getReg();
      unsigned dst = csi.return_args[i];
      unsigned movOpcode = getMoveOpcode(MRI->getRegClass(src));
      MIB = BuildMI(*(returnMI->getParent()), returnMI, returnMI->getDebugLoc(), TII->get(movOpcode))
                                    .addReg(dst,RegState::Define)
                                    .addReg(src)
                                    .setMIFlag(MachineInstr::NonSequential);                      
    }
    MIB = BuildMI(*(returnMI->getParent()), returnMI, returnMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_END))
               .setMIFlag(MachineInstr::NonSequential);
    return;
  }
  MIB = BuildMI(*(entryMI->getParent()), entryMI, entryMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_START))
                                   .setMIFlag(MachineInstr::NonSequential);
  SmallVector<unsigned, 4> select_signals;
  for (auto csi : csilist) {
    SmallVector<unsigned, 4> args;
    for (unsigned arg : csi.call_site_args)
      args.push_back(arg);
    MachineBasicBlock *mbb = entryMI->getParent();
    select_signals.push_back(csa_utils::createUseTree(mbb, entryMI, CSA::ALL0, args));
  }
  for (unsigned i = 0; i < csilist[0].call_site_args.size(); ++i) {
    unsigned dst = entryMI->getOperand(i).getReg();
    const TargetRegisterClass *TRC = MRI->getRegClass(dst);
    SmallVector<unsigned, 4> vals;
    for (auto csi : csilist)
      vals.push_back(csi.call_site_args[i]);
    MachineBasicBlock *mbb = entryMI->getParent();
    unsigned src = csa_utils::createPickTree(mbb, entryMI, TRC, select_signals, vals, CSA::NA);
    unsigned movOpcode = getMoveOpcode(TRC);
    MIB = BuildMI(*(entryMI->getParent()), entryMI, entryMI->getDebugLoc(), TII->get(movOpcode))
                                    .addReg(dst,RegState::Define)
                                    .addReg(src)
                                    .setMIFlag(MachineInstr::NonSequential);
    MachineInstr *MI = &*MIB;
    (void) MI;
  }
  MIB = BuildMI(*(entryMI->getParent()), entryMI, entryMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_END))
                 .setMIFlag(MachineInstr::NonSequential);
  MIB = BuildMI(*(returnMI->getParent()), returnMI, returnMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_START))
                 .setMIFlag(MachineInstr::NonSequential);
  for (unsigned i = 0; i < csilist[0].return_args.size(); ++i) {
    unsigned src = returnMI->getOperand(i).getReg();
    const TargetRegisterClass *TRC = MRI->getRegClass(src);
    SmallVector<unsigned, 4> outvals;
    for (auto csi : csilist)
      outvals.push_back(csi.return_args[i]);
    MachineBasicBlock *mbb = returnMI->getParent();
    csa_utils::createSwitchTree(mbb, returnMI, TRC, select_signals, outvals, src, 0, CSA::NA);
  }
  MIB = BuildMI(*(returnMI->getParent()), returnMI, returnMI->getDebugLoc(), TII->get(CSA::TRAMPOLINE_END))
               .setMIFlag(MachineInstr::NonSequential);
}

MachineInstr* CSAProcCallsPass::addReturnInstruction(MachineInstr *entryMI) {
  MachineInstr *lastMI = LMFI->getReturnMI();
  assert(lastMI && "Dataflow conversion did not set the return instruction");
  assert(lastMI->getOpcode() == CSA::CSA_RETURN &&
      "Instruction selection did not use the right function lowering");
  LMFI->setReturnMI(lastMI);

  // Check to make sure we don't have any use from the entry instruction.
  for (auto &Op : lastMI->operands()) {
    assert(Op.isReg() && "CSA_RETURN must have register operands");
    unsigned Reg = Op.getReg();
    if (MRI->getUniqueVRegDef(Reg) == entryMI) {
      unsigned NewReg = LMFI->allocateLIC(MRI->getRegClass(Reg));
      // Insert a move.
      BuildMI(*lastMI->getParent(), lastMI, lastMI->getDebugLoc(),
          TII->get(TII->makeOpcode(CSA::Generic::MOV, MRI->getRegClass(Reg))),
          NewReg)
        .addReg(Reg)
        .setMIFlag(MachineInstr::NonSequential);
      Op.setReg(NewReg);
    }
  }
  return lastMI;
}
void CSAProcCallsPass::changeMovConstToGate(unsigned entry_mem_ord_lic) {
  // SXU Move instr to DF
  for (MachineFunction::iterator MBB = thisMF->begin(), E = thisMF->end(); MBB != E; ++MBB) {
    MachineBasicBlock::iterator nextMI = MBB->begin();
    for (MachineBasicBlock::iterator I = MBB->begin(); nextMI != MBB->end(); I = nextMI) {
      MachineInstr *MI = &*I;
      ++I;
      nextMI = I;
      if (!isMovOpcode(MI->getOpcode())) continue;
      if (MI->getFlag(MachineInstr::NonSequential)) continue;
      if (MI->getOperand(1).isImm() || MI->getOperand(1).isFPImm() || MI->getOperand(1).isGlobal()) {
         MachineInstrBuilder MIB = BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(getGateOpcode(MI->getOpcode())))
                                    .addReg(MI->getOperand(0).getReg(),RegState::Define)
                                    .addReg(entry_mem_ord_lic);
        if (MI->getOperand(1).isImm())
          MIB.addImm(MI->getOperand(1).getImm());
        else if (MI->getOperand(1).isFPImm())
          MIB.addFPImm(MI->getOperand(1).getFPImm());
        else
          MIB.addGlobalAddress(MI->getOperand(1).getGlobal());
        MIB.setMIFlag(MachineInstr::NonSequential);
        MI->eraseFromParent();
      }
    }
  }
  return;
}

// Cloned from CSACreateSelfContainedGraph.cpp
/********************************************************************
 *  * For initializer functions, memory ordering edges (param and result)
 *   * are handled in a special way.
 *    * If there are parameters other the memory ordering edge parameter,
 *     * Then the memory ordering edge is removed from parameter list and
 *      * its use inside the function is replaced by the narrow copy of the 
 *       * first non memory ordering edge parameter.
 *        * If there are results other the memory odering edge, then each result
 *         * is gated through the output memory ordering edge and the output memory
 *          *  ordering edge is removed from the list os results
 *           ********************************************************************/
void cleanupInitializerFunctions(const Module &M, MachineModuleInfo *MMI) {
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

bool CSAProcCallsPass::runOnMachineFunction(MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;

  thisMF = &MF;
  
  LLVM_DEBUG(errs() << "Entering into CSA Procedure Calls Pass for " << MF.getFunction().getName() << "\n");
  if (ProcCallsPass == 0) return false;
  LMFI   = MF.getInfo<CSAMachineFunctionInfo>();
  MRI = &(MF.getRegInfo());
  TII = static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());
  MachineInstr *entryMI = LMFI->getEntryMI();
  if (!entryMI) report_fatal_error("Entry instruction could not be created! Cannot be run on CSA!");

  // In this loop, we identify the parameters that are not used in the function body
  // Currently such parameters are marked as CSA::IGN
  // However, to be linked with call-sites, the parameters need to be LICs
  // In this loop, the unused params are identified and replaced by 'dummy' LICs
  // MOV instructions to consume these LICs are also added
  for (unsigned i = 0; i < entryMI->getNumOperands(); ++i) {
    if (!entryMI->getOperand(i).isReg()) continue;
    unsigned reg = entryMI->getOperand(i).getReg();
    int dummyid = 0;
    if (reg == CSA::IGN) {
      unsigned vReg = LMFI->allocateLIC(&CSA::CI64RegClass, Twine("_dummy")+Twine(dummyid++), Twine(""), true);
      unsigned movopcode = TII->makeOpcode(CSA::Generic::MOV, &CSA::CI64RegClass);
      BuildMI(*(entryMI->getParent()), ++entryMI->getIterator(), entryMI->getDebugLoc(),
              TII->get(movopcode), CSA::IGN)
        .addReg(vReg)
        .setMIFlag(MachineInstr::NonSequential);
      entryMI->getOperand(i).setReg(vReg);
    }
  }
  MachineInstr *returnMI = addReturnInstruction(entryMI);
  if (!returnMI) report_fatal_error("Return instruction could not be found! Cannot be run on CSA!");
  
  MachineModuleInfo &MMI = thisMF->getMMI();
  const Module *M = MMI.getModule();
  cleanupInitializerFunctions(*M, &MMI);
  
  addTrampolineCode(entryMI,returnMI);
  processCallAndContinueInstructions();

  // At this stage there should be no sequential instructions
  for (auto MBB = thisMF->begin(), E = thisMF->end(); MBB != E; ++MBB) {
    for (auto I = MBB->begin(); I != MBB->end(); ++I) {
      MachineInstr *MI = &*I;
      if (MI->getOpcode() == CSA::UNIT) continue;
      if (MI->getOpcode() == TargetOpcode::DBG_VALUE) continue;
      if (MI->getNumOperands() && MI->getOperand(0).isReg() && MI->getOperand(0).isDef() && MI->getOperand(0).isDead())
        continue;
      if (!MI->getFlag(MachineInstr::NonSequential)) {
        errs() << "MI = " << *MI << "\n";
        report_fatal_error("Sequential instruction found! Cannot be run on CSA!");
      }
    }
  }

  // delete all UNIT SXU ops
  for (auto MBB = thisMF->begin(), E = thisMF->end(); MBB != E; ++MBB) {
    auto I1 = MBB->begin();
    for (auto I = MBB->begin(); I != MBB->end(); I = I1) {
      MachineInstr *MI = &*I;
      I1 = I;
      ++I1;
      if (MI->getOpcode() == CSA::UNIT) {
	if (MI->getOperand(0).getImm() == CSA::FUNCUNIT::SXU)
	  MI->eraseFromParent();
      }
    }
  }
  return true;
}
