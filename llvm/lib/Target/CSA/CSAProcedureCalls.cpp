//===-- CSAProcedureCalls.cpp - CSA compilation of procedure calls ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

static cl::opt<bool>
ReportWarningForExtCalls("csa-report-ext-calls-as-warning",
          cl::Hidden, cl::ZeroOrMore,
          cl::desc("CSA Specific: Report external calls as warnings"),
          cl::init(false));
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
    AU.addRequired<MachineModuleInfo>();
    AU.addPreserved<MachineModuleInfo>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void addTrampolineCode(MachineInstr *,MachineInstr *);
  MachineInstr *addEntryInstruction(void);
  MachineInstr *addReturnInstruction(MachineInstr *);
  void addCallAndContinueInstructions(void);
  void changeMovConstToGate(unsigned int);
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

void CSAProcCallsPass::getCallSiteArgs(CALL_SITE_INFO &csi, MachineInstr *entryMI, bool isDeclared) {
  csi.call_site_args.push_back(getParamReg(csi.caller_func, "caller_out_mem_ord_", csi.call_site_index, 0,
                                            &CSA::CI0RegClass, isDeclared, true));
  for (unsigned int i = 1; i < entryMI->getNumOperands(); ++i) {
    csi.call_site_args.push_back(getParamReg(csi.caller_func, "param_", csi.call_site_index, i,
                                            &CSA::CI64RegClass, isDeclared, true));
  }
}

void CSAProcCallsPass::getReturnArgs(CALL_SITE_INFO &csi, MachineInstr *returnMI, bool isDeclared) {
  csi.return_args.push_back(getParamReg(csi.caller_func, "caller_in_mem_ord_", csi.call_site_index, 0,
                                            &CSA::CI0RegClass, isDeclared, true));
  for (unsigned int i = 1; i < returnMI->getNumOperands(); ++i) {
    csi.return_args.push_back(getParamReg(csi.caller_func, "result_", csi.call_site_index, i,
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
  if (CI->getNumArgOperands() == 0) return nullptr;
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

// This function returns the first non-debug instruction in a function
// TODO: check if this functionality exists elsewhere
static MachineInstr *getFirstMI(MachineFunction *MF) {
  for (MachineFunction::iterator MBB = MF->begin(); MBB != MF->end(); MBB++) {
    for (MachineBasicBlock::iterator I = MBB->begin(); I != MBB->end(); I++) {
      MachineInstr *MI = &*I;
      if (MI->isDebugValue()) continue;
      return MI;
    }
  }
  return nullptr;
}

// This function generates a pseudo CSA instruction for defining the entry point directive
// It holds a memory ordering operand that will be used to control the ordering between the
// caller and callee memory operations
// It also holds the input function arguments
// Input code:
// Function Live Ins: %P64_2 in %vreg0
// BB#0: derived from LLVM BB %entry
// %vreg4<def> = MOV0 %RA; CI0:%vreg4 (will be deleted)
// %vreg0<def> = COPY %P64_2; CI32:%vreg0 (will be deleted)
// ------->
// Output code:
// Function Live Ins: %P64_2 in %vreg0
// BB#0: derived from LLVM BB %entry
// %vreg6<def>, %vreg0<def> = CSA_ENTRY;
// Here vreg6 is the memory ordering edge coming from caller and will replace vreg4
// vreg0 is the input parameter
MachineInstr* CSAProcCallsPass::addEntryInstruction(void) {
  MachineInstr *copyMI;
  unsigned reg1 = LMFI->allocateLIC(&CSA::CI0RegClass, Twine("callee_in_caller_mem_ord"));
  MachineInstr *firstMI = getFirstMI(thisMF);
  MachineInstrBuilder MIB = BuildMI(*(firstMI->getParent()), firstMI, firstMI->getDebugLoc(), TII->get(CSA::CSA_ENTRY))
                                    .addReg(reg1,RegState::Define);
  // Add entry parameters. If there are some dead parameters, add dummy vars instead
  int dummyid = 0;
  const Function *F = &thisMF->getFunction();
  Function::const_arg_iterator I, E;
  auto LI = MRI->livein_begin();
  unsigned Arg;
  bool flag = false;
  for (I = F->arg_begin(), E = F->arg_end(), Arg= CSA::P64_2; I != E; ++Arg) {
    if (I->getType()->getScalarSizeInBits() == 128) {
      if (flag) ++I;
      flag = !flag;
    } else
      ++I;
    bool ArgIsDead = false;
    if (!MRI->isLiveIn(Arg))
      ArgIsDead = true;
    else {
      unsigned vReg = LI->second;
      ++LI;
      MachineInstr *DefMI = MRI->getVRegDef(vReg);
      if (DefMI) {
        MIB.addReg(vReg,RegState::Define);
        DefMI->eraseFromParent();
      }
      else
        ArgIsDead = true;
    }
    if (ArgIsDead) {
      std::stringstream ss1;
      ss1 << "_dummy" << dummyid++;
      std::string str = ss1.str();
      StringRef name(str);
      unsigned vReg = LMFI->allocateLIC(&CSA::CI64RegClass, Twine(name), Twine(""), true);
      MIB.addReg(vReg,RegState::Define);
      BuildMI(*(firstMI->getParent()), firstMI, firstMI->getDebugLoc(), TII->get(CSA::MOV0), CSA::IGN)
      .addReg(vReg)
      .setMIFlag(MachineInstr::NonSequential);
    }
  }
  MachineInstr *MI = &*MIB;
  MI->setFlag(MachineInstr::NonSequential);
  LLVM_DEBUG(errs() << "Entry instruction = " << *MI << "\n");

  // Deal with output mem ord edge from entry
  // MemOpOrdering introduces a SXU instruction to generate a memory ordering reg
  // That memory ordering reg is replaced by the input LIC here
  bool outerloop = true;
  for (MachineFunction::iterator BB1 = thisMF->begin(); BB1 != thisMF->end() && outerloop; BB1++) {
    MachineBasicBlock::iterator nextI1 = BB1->begin();
    for (MachineBasicBlock::iterator I1 = BB1->begin(); I1 != BB1->end(); I1=nextI1) {
      copyMI = &*I1;
      nextI1 = I1;
      ++nextI1;
      if (copyMI->getOpcode() != CSA::MOV0) continue;
      if (copyMI->getOperand(1).isReg() && (copyMI->getOperand(1).getReg() == CSA::RA)) {
        MRI->replaceRegWith(copyMI->getOperand(0).getReg(),MI->getOperand(0).getReg());
        LLVM_DEBUG(errs() << "copy operation to be deleted = " << *copyMI << "\n");
        copyMI->eraseFromParent();
        outerloop = false;
        break;
      }
    }
  }
  LMFI->setEntryMI(MI);
  return MI;
}

static MachineInstr *getLastMI(MachineFunction *MF) {
  for (MachineFunction::reverse_iterator MBB = MF->rbegin(); MBB != MF->rend(); MBB++) {
    for (MachineBasicBlock::reverse_iterator I = MBB->rbegin(); I != MBB->rend(); I++) {
      MachineInstr *MI = &*I;
      if (MI->isDebugValue()) continue;
      return MI;
    }
  }
  return nullptr;
}

MachineInstr* CSAProcCallsPass::addReturnInstruction(MachineInstr *entryMI) {
  MachineInstr *copyMI;
  MachineInstr *lastMI = getLastMI(thisMF);
  if (!lastMI || !lastMI->isReturn()) return nullptr;
  unsigned reg;
  unsigned reg1 = LMFI->allocateLIC(&CSA::CI0RegClass, Twine("callee_out_caller_mem_ord"));
  MachineInstrBuilder MIB = BuildMI(*(lastMI->getParent()), lastMI, lastMI->getDebugLoc(), TII->get(CSA::CSA_RETURN))
                                    .addReg(reg1);
  MachineInstr *MI = &*MIB;
  // Looking at implicit operands of RETURN instruction
  // THese are values being returned
  for (auto op : lastMI->implicit_operands()) {
    reg = op.getReg();
    bool outerLoop = true;
    // Find the copy instruction that defines this operand in the RETURN instruction
    for (MachineFunction::reverse_iterator MBB = thisMF->rbegin(), E = thisMF->rend(); (MBB != E) && outerLoop; ++MBB) {
      for (MachineBasicBlock::reverse_iterator I = MBB->rbegin(); (I != MBB->rend()) && outerLoop; ++I) {
        MachineInstr *currMI = &*I;
        // if result of callee code is directly returned by the RET instruction
        // i.e 
        // JSRi @tan,<parameter list>
        // (..no def or use of p64_0...)
        // RET p64_0
        // Since this code relies on separate LICs 
        // for caller RET values and Callee RET values
        // We need to introduce copies
        // All these changes should be replaced by performing call
        // lowering to the correct DF instruction at ISelLowering
        if ((currMI->getOpcode() == CSA::JSR) || (currMI->getOpcode() == CSA::JSRi)) {
          unsigned newCallReturnReg = LMFI->allocateLIC(&CSA::CI64RegClass, "call_to_ret_tmp");
          MachineInstrBuilder MIB = BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(CSA::MOV64))
                                      .addReg(reg,RegState::Define)
                                      .addReg(newCallReturnReg)
                                      .setMIFlag(MachineInstr::NonSequential);
          copyMI = &*MIB;
          MIB = BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(CSA::MOV64))
                                      .addReg(newCallReturnReg,RegState::Define)
                                      .addReg(reg)
                                      .setMIFlag(MachineInstr::NonSequential);
          outerLoop = false;
          break;
        } else {
          for (unsigned i = 0; i < currMI->getNumOperands(); ++i) {
            MachineOperand &top = currMI->getOperand(i);
            if (top.isReg()) {
              if (top.isDef() && top.getReg() == reg) {
                copyMI = currMI;
                outerLoop = false;
                break;
              }
            }
          }
        }
      }
    }
    unsigned returnReg;
    if (copyMI->getOpcode() != CSA::RET) {
      returnReg = copyMI->getOperand(1).getReg();
      assert(copyMI && copyMI->getOpcode() == CSA::MOV64);
      // Sometimes return reg is one of the parameters. We need to handle this
      bool isParam = false;
      for (unsigned i = 0; i < entryMI->getNumOperands(); ++i) {
        unsigned reg = entryMI->getOperand(i).getReg();
        if (reg == returnReg) { isParam = true; break; }
      }
      if (isParam) {
        StringRef name = thisMF->getFunction().getName();
        const TargetRegisterClass *TRC = MRI->getRegClass(returnReg);
        unsigned newReturnReg = LMFI->allocateLIC(TRC, "ret_val", Twine(name), true /*isDeclared*/, false/*isGloballyVisible*/);
        BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(CSA::MOV64))
                                      .addReg(newReturnReg,RegState::Define)
                                      .addReg(returnReg)
                                      .setMIFlag(MachineInstr::NonSequential);
        MIB.addReg(newReturnReg);
      }
      else
        MIB.addReg(returnReg);
    }
    copyMI->eraseFromParent();
  }

  if (lastMI) lastMI->eraseFromParent();
  MI->setFlag(MachineInstr::NonSequential);
  LLVM_DEBUG(errs() << "Return instruction = " << *MI << "\n");

  // Deal with input memory ordering edge also being used as returning memory ordering edge
  MachineBasicBlock::reverse_iterator RI_BEGIN(MI);
  for (MachineBasicBlock::reverse_iterator I1 = RI_BEGIN; I1 != (MI->getParent())->rend(); I1++) {
    copyMI = &*I1;
    if (copyMI->getOpcode() != CSA::MOV0) continue;
    if (!copyMI->getOperand(0).isReg()) continue;
    if (copyMI->getOperand(0).getReg() != CSA::R0) continue;
    if (copyMI->getOperand(1).getReg() == entryMI->getOperand(0).getReg()) {
      copyMI->getOperand(0).setReg(MI->getOperand(0).getReg());
      copyMI->setFlag(MachineInstr::NonSequential);
    } else {
      MRI->replaceRegWith(copyMI->getOperand(1).getReg(), MI->getOperand(0).getReg());
      copyMI->eraseFromParent();
    }
    break;
  }
  LMFI->setReturnMI(MI);
  return MI;
}

void CSAProcCallsPass::addCallAndContinueInstructions(void) {
  // Call and continue instructions
  // CALL: (ins calltarget:$callee_func, I64:$call_site_index, I64:$context, MemOrdUse:$mem_ord, variable_ops),
  // CONTINUE: (outs I64:$context, MemOrdUse:$mem_ord, variable_ops), (ins I64:$call_site_index)
  MachineModuleInfo &MMI = thisMF->getMMI();
  const Module *M = MMI.getModule();
  int CallSiteIndex = 1;
  StringRef name = thisMF->getFunction().getName();
  MachineInstr *copyMI;
  for (MachineFunction::iterator MBB = thisMF->begin(), E = thisMF->end(); MBB != E; ++MBB) {
    MachineBasicBlock::iterator nextMI = MBB->begin();
    for (MachineBasicBlock::iterator I = MBB->begin(); nextMI != MBB->end(); I = nextMI) {
      MachineInstr *MI = &*I;
      nextMI = I;
      nextMI++;
      if ((MI->getOpcode() != CSA::JSR) && (MI->getOpcode() != CSA::JSRi)) continue;
      LLVM_DEBUG(errs() << "Input Call MI = " << *MI << "\n");
      const MachineOperand &MO = MI->getOperand(0);
      StringRef callee_name;
      // Report external calls here as either WARNINGs or FAILs depending on flag
      if (!MO.isGlobal()) {
        if (MO.isSymbol()) {
          if (M->getFunction(MO.getSymbolName())) {
            callee_name = MO.getSymbolName();
          }
        } else {
          if (ReportWarningForExtCalls) {
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
            if (ReportWarningForExtCalls) {
              callee_name = "dummy_func";
              errs() << "WARNING: External calls not yet supported! May generate code with incomplete linkage!\n";
            } else
              report_fatal_error("External calls not yet supported! Cannot be run on CSA!");
          }
        } else
          callee_name = F->getName();
      }
      LLVM_DEBUG(errs() << "Called Function name = " << callee_name << "\n");
      if (callee_name== name) {
        errs() << "Called Function name = " << callee_name << "\n";
        report_fatal_error("Recursive function call not yet supported! Cannot be run on CSA!");
      }
      // The LIC will be declared in calle code or caller code depending on which comes first in the module
      int callee_id = get_func_order(callee_name,M);
      int caller_id = get_func_order(name,M);
      bool isDeclared = (callee_id > caller_id);
      unsigned reg1 = getParamReg(name, "caller_out_mem_ord_", CallSiteIndex, 0,
                                            &CSA::CI0RegClass, isDeclared, true);
      MachineInstrBuilder Call_MIB = BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(CSA::CSA_CALL))
                                        .add(MI->getOperand(0))
                                        .addImm(CallSiteIndex)
                                        .addReg(reg1);
      for (unsigned int i = 1; i < MI->getNumExplicitOperands(); ++i) {
        unsigned preg = MI->getOperand(i).getReg();
        MachineBasicBlock *MBB = MI->getParent();
        MachineBasicBlock::reverse_iterator RI_BEGIN(I);
        MachineBasicBlock::reverse_iterator nextI1 = RI_BEGIN;
        for (MachineBasicBlock::reverse_iterator I1 = RI_BEGIN; I1 != MBB->rend(); I1 = nextI1) {
          copyMI = &*I1;
          I1++;
          nextI1 = I1;
          if (!isMovOpcode(copyMI->getOpcode())) continue;
          if (copyMI->getOperand(0).isReg()) {
            if (copyMI->getOperand(0).getReg() == preg) {
              unsigned paramReg = getParamReg(name, "param_", CallSiteIndex, i,
                                            &CSA::CI64RegClass, isDeclared, true);
              Call_MIB.addReg(paramReg);
              copyMI->getOperand(0).setReg(paramReg);
              copyMI->setFlag(MachineInstr::NonSequential);
              break;
            }
          }
        }
      }
      reg1 = getParamReg(name, "caller_in_mem_ord_", CallSiteIndex, 0,
                                            &CSA::CI0RegClass, isDeclared, true);
      MachineInstrBuilder Cont_MIB = BuildMI(*(MI->getParent()), MI, MI->getDebugLoc(), TII->get(CSA::CSA_CONTINUE))
                                            .addReg(reg1, RegState::Define);
      // Get to the instruction after JSR
      // and look for results returned from callee till the next JSR
      // or the end of the basic block
      MachineBasicBlock::iterator I1 = ++I;
      MachineBasicBlock::iterator nextI1 = I1;
      for (; I1 != MBB->end(); I1 = nextI1) {
        copyMI = &*I1;
        if (copyMI->getOpcode() == CSA::JSR || copyMI->getOpcode() == CSA::JSRi) break;
        nextI1 = I1;
        ++nextI1;
        LLVM_DEBUG(errs() << "CopyMI is " << *copyMI << "\n");
        if (copyMI->getOpcode() != CSA::MOV64) continue;
        auto top = copyMI->getOperand(1);
        if (!top.isReg()) continue;
        if (top.getReg() != CSA::P64_0 && top.getReg() != CSA::P64_1) continue;
        LLVM_DEBUG(errs() << "CopyMI for result is " << *copyMI << "\n");
        unsigned resultReg = getParamReg(name, "result_", CallSiteIndex, 1,
                                        &CSA::CI64RegClass, isDeclared, true);
        Cont_MIB.addReg(resultReg,RegState::Define);
        copyMI->getOperand(1).setReg(resultReg);
        copyMI->setFlag(MachineInstr::NonSequential);
      }
      Cont_MIB.addImm(CallSiteIndex++);
      MachineInstr *newCI = &*Call_MIB;
      LLVM_DEBUG(errs() << "new call instruction = " << *newCI << "\n");
      newCI->setFlag(MachineInstr::NonSequential);
      MachineInstr *newCoI = &*Cont_MIB;
      LLVM_DEBUG(errs() << "new continue instruction = " << *newCoI << "\n");
      newCoI->setFlag(MachineInstr::NonSequential);

      // Deal with input memory ordering edge into call
      MachineBasicBlock::reverse_iterator RI_BEGIN(MI);
      for (MachineBasicBlock::reverse_iterator I1 = RI_BEGIN; I1 != MI->getParent()->rend(); ++I1) {
        copyMI = &*I1;
        if (copyMI->getOpcode() != CSA::MOV0) continue;
        if (!copyMI->getOperand(0).isReg()) continue;
        if (copyMI->getOperand(0).getReg() != CSA::R0) continue;
        copyMI->getOperand(0).setReg(newCI->getOperand(2).getReg());
        copyMI->setFlag(MachineInstr::NonSequential);
        LLVM_DEBUG(errs() << "new mem op input instruction = " << *copyMI << "\n");
        break;
      }
      
      // Deal with output mem ord edge from continue
      MachineBasicBlock::iterator I_BEGIN(MI);
      for (MachineBasicBlock::iterator I1 = I_BEGIN; I1 != MI->getParent()->end(); ++I1) {
        copyMI = &*I1;
        if (copyMI->getOpcode() != CSA::MOV0) continue;
        if (copyMI->getOperand(1).isReg() && (copyMI->getOperand(1).getReg() == CSA::RA)) {
          copyMI->getOperand(1).setReg(newCoI->getOperand(0).getReg());
          copyMI->setFlag(MachineInstr::NonSequential);
          LLVM_DEBUG(errs() << "new mem op output instruction = " << *copyMI << "\n");
          break;
        }
      }
      MI->eraseFromParent(); 
    }
  }
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

bool CSAProcCallsPass::runOnMachineFunction(MachineFunction &MF) {
  thisMF = &MF;
  LLVM_DEBUG(errs() << "Entering into CSA Procedure Calls Pass for " << MF.getFunction().getName() << "\n");
  if (ProcCallsPass == 0) return false;
  LMFI   = MF.getInfo<CSAMachineFunctionInfo>();
  MRI = &(MF.getRegInfo());
  TII = static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());
  MachineInstr *entryMI = addEntryInstruction();
  if (!entryMI) report_fatal_error("Entry instruction could not be created! Cannot be run on CSA!");
  MachineInstr *returnMI = addReturnInstruction(entryMI);
  if (!returnMI) report_fatal_error("Return instruction could not be created! Cannot be run on CSA!");
  // Check if input mem order edge is used by any mem op in function
  // if not, then pipe it to the output mem order edge
  bool isInputMemOrdUsed = false;
  unsigned entry_mem_ord = entryMI->getOperand(0).getReg();
  unsigned return_mem_ord = returnMI->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(entry_mem_ord);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO    = *UI;
    ++UI;
    const MachineInstr *UseMI      = UseMO.getParent();
    if (UseMI->getOpcode() != CSA::CSA_ENTRY && UseMI->getOpcode() != CSA::CSA_RETURN 
          && UseMI->getOpcode() != CSA::CSA_CALL && UseMI->getOpcode() != CSA::CSA_CONTINUE) {
      isInputMemOrdUsed = true;
      break;
    }
  }
  if (!isInputMemOrdUsed) {
    MachineInstrBuilder MIB = BuildMI(*(returnMI->getParent()), returnMI, returnMI->getDebugLoc(), TII->get(CSA::MOV0))
                                    .addReg(return_mem_ord,RegState::Define)
                                    .addReg(entry_mem_ord)
                                    .setMIFlag(MachineInstr::NonSequential);
     (void) MIB;
  }
  addTrampolineCode(entryMI,returnMI);
  addCallAndContinueInstructions();
  changeMovConstToGate(entry_mem_ord);

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
