//===- Intel_IndirectCallConv.cpp - Indirect call Conv transformation -===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-------------------------------------------------------------------===//
//
// This pass performs indirect calls to direct calls conversion if possible
// using points-to info.
// Let us assume Points-to analysis finds that possible targets for fp
// are foo, bar and universalSet. This pass converts the below call 
//
//    t = (*fp)(arg1, arg2);
//
//      into
//  
//    if (fp == foo) {
//      t1 = foo(arg1, arg2);
//    } else if (fp == bar) {
//      t2 = bar(arg1, arg2);
//    } {
//      t3 = (*fp)(arg1, arg2);
//    }
//    t = PHI_NODE(t1, t2, t3); 
// 
//  It eliminates indirect call completely if universalSet is not one
//  of possible targets.
//
//===-------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
using namespace llvm;

#define DEBUG_TYPE "intel-ind-call-conv"

// Maximum number of determined targets to specialize indirect call
static cl::opt<unsigned>
IndCallConvMaxTarget("intel-ind-call-conv-max-target",
                      cl::ReallyHidden, cl::init(3));

// Option to trace indirect call conversion transformation
static cl::opt<bool> IndCallConvTrace("print-indirect-call-conv",
                                         cl::ReallyHidden);

// Option to control allowing InvokeInst for IndCallConv transformation
static cl::opt<bool> IndCallConvAllowInvoke("intel-ind-call-conv-allow-invoke",
cl::init(false), cl::ReallyHidden);
//

STATISTIC(NumIndirectCallsConv, "Number of Indirect calls Converted");

namespace {
  // IndirectCallConv pass implementation
  struct IndirectCallConv : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    IndirectCallConv() : FunctionPass(ID) {
      initializeIndirectCallConvPass(*PassRegistry::getPassRegistry());
    }

    bool runOnFunction(Function &F) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<TargetLibraryInfoWrapperPass>();
      AU.addRequired<AndersensAAWrapperPass>();
    }

private:
  // Points-to info that is computed by Andersens-Analysis
  AndersensAAResult *AnderPointsTo;

  bool IsIndCallConvCandidateCallSite(CallSite CS);
  CallSite CreateDirectCallSite(CallSite CS, Value *F, BasicBlock* In_BB);
  bool IndirectCallConvCall(CallSite CS);
  };
}

char IndirectCallConv::ID = 0;
INITIALIZE_PASS_BEGIN(IndirectCallConv, "indirectcallconv",
                "Indirect Call Conv", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AndersensAAWrapperPass)
INITIALIZE_PASS_END(IndirectCallConv, "indirectcallconv",
                "Indirect Call Conv", false, false)

FunctionPass *llvm::createIndirectCallConvPass() {
  return new IndirectCallConv();
}
 
// Return true if CS is not a direct call.
//
bool IndirectCallConv::IsIndCallConvCandidateCallSite(CallSite CS) {
  Value* func = CS.getCalledValue();
  func = func->stripPointerCasts();
  if (isa<Function>(func)) {
    return false;
  }
  return true;
}

// Creates new CallInst/InvokeInst that is exactly same as 'CS' but 
// 'FuncName' is used as function name and inserted it into 'Insert_BB'.   
//
CallSite IndirectCallConv::CreateDirectCallSite(CallSite CS, Value* FuncName,
                                                 BasicBlock* Insert_BB) {
  CallSite New_CS;

  if (isa<CallInst>(CS.getInstruction())) {
    CallInst* New_CI;
    std::string New_Name;
    CallInst *CI = cast<CallInst>(CS.getInstruction());

    std::vector<Value*> Args(CI->op_begin(), CI->op_end() - 1);
    New_Name = CI->hasName() ? CI->getName().str() + ".indconv" : "";
    New_CI = CallInst::Create(FuncName, Args, New_Name, Insert_BB);

    New_CI->setDebugLoc(CI->getDebugLoc());
    New_CI->setCallingConv(CI->getCallingConv());
    New_CI->setAttributes(CI->getAttributes());
    New_CS = CallSite(New_CI);
  } else if (isa<InvokeInst>(CS.getInstruction())) {
    InvokeInst* New_II;
    std::string New_Name;
    InvokeInst *II = cast<InvokeInst>(CS.getInstruction());

    std::vector<Value*> Args(II->op_begin(), II->op_end() - 3);
    New_Name = II->hasName() ? II->getName().str() + ".indconv" : "";
    New_II = InvokeInst::Create(FuncName,  II->getNormalDest(),
                                II->getUnwindDest(), Args, New_Name, Insert_BB);


    New_II->setDebugLoc(II->getDebugLoc());
    New_II->setCallingConv(II->getCallingConv());
    New_II->setAttributes(II->getAttributes());
    New_CS = CallSite(New_II);
  }
  else {
    llvm_unreachable("Expecting call/invoke instruction");
  }
  return New_CS;
}


// Convert indirect call 'CS' to direct call if possible.
//
//  Ex (Complete Set):
//   Before:  
//     (*fp)(args);
//     ...
//
//   After: (Assume possible targets of fp are foo and bar)
//     if (fp == foo) {
//       foo(args)
//     } else {
//       bar(args);
//     }
//     ...
//
//
//  Ex (Incomplete Set):
//   Before:  
//     (*fp)(args);
//     ...
//
//   After: (Assume possible targets of fp are foo and bar)
//     if (fp == foo) {
//       foo(args)
//     } else if (fp == bar) {
//       bar(args);
//     } else {
//       (*fp)(args);
//     }
//     ...
//
bool IndirectCallConv::IndirectCallConvCall(CallSite CS) {
  bool IsComplete;
  unsigned NumPossibleTargets = 0;

  if (IndCallConvTrace) {
    errs() << "Call-Site:  ";
    errs() << *(CS.getInstruction()) << "\n";
  }

  // Get possible targets for 'CS' using points-to info. 
  std::vector<llvm::Value*> PossibleTargets;
  Value* call_fptr = CS.getCalledValue()->stripPointerCasts();
  IsComplete = AnderPointsTo->GetFuncPointerPossibleTargets(call_fptr,
                                                            PossibleTargets);

  if (!IsComplete) {
    if (IndCallConvTrace) {
      errs() << "    (Incomplete set) \n";
    }
    // If Incomplete, increment NumPossibleTargets by 1 since indirect call
    // will be generated as Fallback case.
    NumPossibleTargets++;
  }
  else {
    if (IndCallConvTrace) {
      errs() << "    (Complete set) \n";
    }
  }
  // No possible targets ... skip them
  if (!PossibleTargets.size()) {
    if (IndCallConvTrace) {
      errs() << "    No possible targets \n";
    }
    return false;
  }

  NumPossibleTargets += PossibleTargets.size();
  
  if (IndCallConvTrace) {
    for (auto F1 = PossibleTargets.begin(), E1 = PossibleTargets.end();
         F1 != E1; ++F1) {
      Function *Fun = dyn_cast<Function>(*F1);
      errs() << "    " << Fun->getName() << "\n";
    }
  }

  // It is not converted if number of possible targets exceeds
  // IndCallConvMaxTarget.
  if (NumPossibleTargets > IndCallConvMaxTarget) {
    if (IndCallConvTrace) {
      errs() << "    Number of possible targets exceeds Limit\n";
    }
    return false;
  }

  NumIndirectCallsConv++;

  assert(NumPossibleTargets > 0 && "Incorrect Number of possible targets");
  // Number of condition stmts required is one less than number of
  // possible targets.
  unsigned NumberCondStmts = NumPossibleTargets - 1;

  if (!NumberCondStmts) {
    // If there is only one possible target and it is complete, just
    // replace function pointer with direct call.
    auto FirstElement = PossibleTargets.front();
    CS.setCalledFunction(FirstElement); 
    if (IndCallConvTrace) {
      errs() << "    Replaced with Direct call" <<
                                  *(CS.getInstruction()) << "\n";
    }
    return true;
  }

  // Get BasicBlock of indirect call
  Instruction *SplitPt = nullptr;
  if (CallInst* CI = dyn_cast<CallInst>(CS.getInstruction())) {
    SplitPt = CI;
  }
  else if (InvokeInst* CI = dyn_cast<InvokeInst>(CS.getInstruction())) {
    SplitPt = CI;
  }
  assert(SplitPt != nullptr && "Expected Call/Invoke Inst");

  BasicBlock* OrigBlock = SplitPt->getParent();

  if (IndCallConvTrace) {
    errs() << " \n BasicBlocks before transformation \n";
    errs() << *OrigBlock;
    errs() << "\n\n";
  }

  std::string BB_Str = ".indconv.sink.";
  // Split BasicBlock that the indirect call lives in.
  // After splitting, the indirect call will be in Tail_BB and new
  // branch instruction will be added at the end of OrigBlock.
  BasicBlock *Tail_BB = OrigBlock->splitBasicBlock(SplitPt->getIterator(), BB_Str);
  assert(Tail_BB != nullptr && "Split BasicBlock Failed");

  // Get rid of branch that was added by splitBasicBlock.
  OrigBlock->getInstList().pop_back();

  // List of newly created BasicBlocks that are created to hold direct calls.
  std::vector<BasicBlock*> NewDirectCallBBs;

  // List of newly created BasicBlocks that are created to hold conditional
  // stmts.
  std::vector<BasicBlock*> NewCondStmtBBs;

  // List of newly created direct call stmts
  std::vector<CallSite> NewDirectCalls;

  // List of newly created conditional stmts
  std::vector<CmpInst*> NewCondStmts;

  unsigned index = 0;
  BasicBlock* Cond_BB;
  CallSite New_CS;
  CmpInst* Comp;

  for (auto F1 = PossibleTargets.begin(), E1 = PossibleTargets.end();
       F1 != E1; ++F1) {


    if (index < NumberCondStmts) {

      // Create cond stmt and a new BasicBlock to insert it
      // Ex:
      // .indconv.cmp.subtract:             ; preds = %.indconv.cmp.add
      //     %.indconv.c9 = icmp eq i32 (i32, i32)* %1, @subtract
      //
 
      // Create basic block to insert cond stmt
      BB_Str = ".indconv.cmp." + (*F1)->getName().str();
      Cond_BB = BasicBlock::Create(call_fptr->getContext(), BB_Str,
                                    OrigBlock->getParent(), Tail_BB);

      // Create condition to test function pointer is equal to the possible
      // target.
      Comp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ,
                                       call_fptr, *F1, ".indconv.c", Cond_BB);
      Comp->setDebugLoc(SplitPt->getDebugLoc());
      // Add new cond stmt and cond BasicBlock to NewCondStmts and
      // NewCondStmtBBs lists that are used to fix CFG later
      NewCondStmts.push_back(Comp);
      NewCondStmtBBs.push_back(Cond_BB);
    }

    // Create direct call and a new BasicBlock to insert it. A new 
    // unconditional branch instruction is added at the end 
    // of the BasicBlock to jump to Tail BasicBlock. 
    // Ex:
    // .indconv.call.subtract:             ; preds = %.indconv.cmp.subtract
    //     %call.i.indconv10 = call i32 @subtract(i32 10, i32 2) #3
    //     br label %.indconv.sw.epilog
    
    // Create basic block to insert direct call
    BB_Str = ".indconv.call." + (*F1)->getName().str();
    BasicBlock* Call_BB = BasicBlock::Create(call_fptr->getContext(), BB_Str,
                                       OrigBlock->getParent(), Tail_BB);

    // Create direct call and insert into Call_BB
    New_CS = CreateDirectCallSite(CS, *F1, Call_BB);

    // Add new call inst and call BasicBlock to NewDirectCalls and
    // NewDirectCallBBs to fix CFG later. 
    NewDirectCalls.push_back(New_CS);
    NewDirectCallBBs.push_back(Call_BB);

    // Create unconditional branch to tail BB
    BranchInst* BI = BranchInst::Create(Tail_BB, Call_BB);
    BI->setDebugLoc(SplitPt->getDebugLoc());
    index++;
  }

  if (index < NumPossibleTargets) {
    // This is fallback case.
    // Ex:
    // .indconv.icall:             ; preds = %.indconv.cmp.multiply
    //     %call.i.indconv12 = call i32 %8(i32 10, i32 2) #3
    //     br label %.indconv.sw.epilog
    
    // Create basic block to insert direct call
    BB_Str = ".indconv.icall." + SplitPt->getName().str();
    BasicBlock* Call_BB = BasicBlock::Create(call_fptr->getContext(), BB_Str,
                                      OrigBlock->getParent(), Tail_BB);

    New_CS = CreateDirectCallSite(CS, CS.getCalledValue(), Call_BB);

    // Add them to NewDirectCallBBs and NewDirectCalls list
    NewDirectCallBBs.push_back(Call_BB);
    NewDirectCalls.push_back(New_CS);

    // Create unconditional branch to tail BB
    BranchInst* BI = BranchInst::Create(Tail_BB, Call_BB);
    BI->setDebugLoc(SplitPt->getDebugLoc());
  }

  // Create new branch instructions to fix CFG for Cond BBs and Call BBs 
  // that are created for specialization.
  BranchInst::Create(NewCondStmtBBs[0], OrigBlock);
  for (index = 0; index < NumberCondStmts; index++) {
    BasicBlock* F_BB;
    BasicBlock* T_BB = NewDirectCallBBs[index];
    BasicBlock* C_BB = NewCondStmtBBs[index];
    CmpInst* C_stmt = NewCondStmts[index];

    if (index + 1 < NumberCondStmts) {
      F_BB = NewCondStmtBBs[index + 1];
    }
    else {
      F_BB = NewDirectCallBBs[index + 1];
    }
    BranchInst* BI = BranchInst::Create(T_BB, F_BB, C_stmt, C_BB);
    BI->setDebugLoc(SplitPt->getDebugLoc());
  }

  // Create PHI node to handle return values of newly created calls.
  // Ex:
  // .indconv.sw.epilog:                 ; preds = %.indconv.call.multiply, 
  //                             ;%.indconv.call.subtract, %.indconv.call.add
  //    %.indconv.ret = phi i32 [ %call.i.indconv, %.indconv.call.add], 
  //                            [ %call.i.indconv10, %.indconv.call.subtract],
  //                            [ %call.i.indconv11, %.indconv.call.multiply]
  //
  if (!CS->getType()->isVoidTy()) {
    PHINode *RPHI = PHINode::Create(CS->getType(), NumPossibleTargets,
                                        ".indconv.ret",
                                        &Tail_BB->front());

    for (unsigned i = 0; i < NumPossibleTargets; i++) {
      CallSite CS2 = NewDirectCalls[i];
      RPHI->addIncoming(CS2.getInstruction(), NewDirectCallBBs[i]);
    }
    RPHI->setDebugLoc(SplitPt->getDebugLoc());
    // Fix UI for newly created PHI_NODE
    SplitPt->replaceAllUsesWith(RPHI);
  }

  // Eliminate original indirect call
  SplitPt->eraseFromParent();

  if (IndCallConvTrace) {
    errs() << " BasicBlocks after transformation \n";
    for (index = 0; index < NumPossibleTargets; index++) {
      if (index < NumberCondStmts) {
        errs() << *NewCondStmtBBs[index];
      }
      errs() << *NewDirectCallBBs[index];
    }
    errs() << *Tail_BB;
    errs() << "\n\n";
  }
  return true;
}
 
// Convert indirect calls to direct calls if possible using points-to info.
//
bool IndirectCallConv::runOnFunction(Function &F) {
  bool Changed = false;

  std::vector<CallSite> IndCallConvList;

  IndCallConvList.clear();

  // Collect Indirect calls in current routine.
  for (inst_iterator II = inst_begin(F), EE = inst_end(F); II != EE; ++II) {
    if (isa<CallInst>(&*II) || 
        (IndCallConvAllowInvoke && isa<InvokeInst>(&*II))) {
      CallSite CS = CallSite(&*II);
      if (IsIndCallConvCandidateCallSite(CS)) {
        IndCallConvList.push_back(CS);
      }
    }
  }
  // No indirect calls found in current routine
  if (IndCallConvList.size() == 0) {
    return Changed;
  }

  // Get points-to info
  AnderPointsTo = &getAnalysis<AndersensAAWrapperPass>().getResult();

  if (IndCallConvTrace) {
    errs() << "IntelIndCallConv for ";
    errs() << F.getName() << "\n";
    errs() << "--------------------------\n";
  }

  // Walk through the IndCallConvList and try to convert each indirect call
  std::vector<CallSite>::const_iterator calllist_itr;
  for (unsigned i = 0, e = IndCallConvList.size(); i != e; ++i) {
    Changed |= IndirectCallConvCall(IndCallConvList[i]);
  }

  if (IndCallConvTrace) {
    errs() << "End IntelIndCallConv\n\n";
  }
  return Changed;
}
