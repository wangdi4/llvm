//===------- Intel_AggInline.cpp - Aggressive Inline Analysis -*------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does Aggressive Inline Analysis to expose uses of global
// pointers to malloc'ed memory. It helps data-transformations to easily
// analyze all uses of global pointers in single routine.
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_AggInline.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Module.h"



using namespace llvm;

// Trace option for Aggressive Inline Analysis.  
//
static cl::opt<bool> InlineAggressiveTrace("inline-agg-trace",
                                        cl::init(false), cl::ReallyHidden);

// Maximum number of callsites marked by this analysis. If it exceeds
// this threshold, this analysis is disabled. 
//
static cl::opt<unsigned> InlineAggressiveCSLimit("inline-agg-callsites-limit",
                                        cl::init(25), cl::ReallyHidden);

// Minimum malloc size limit for Aggressive Inline Analysis. A  malloc
// call that allocates more than this limit is considered for this
// analysis.
//
static cl::opt<unsigned> InlineAggressiveMallocLimit("inline-agg-malloc-limit",
                                    cl::init(0x60000000), cl::ReallyHidden);

// If number of instructions in entire application is greater than
// this limit, it will not be considered for aggressive inline analysis.
//
static cl::opt<uint64_t> InlineAggressiveInstLimit("inline-agg-inst-limit",
                                    cl::init(0x3000), cl::ReallyHidden);

#define DEBUG_TYPE  "inlineaggressiveanalysis"


INITIALIZE_PASS_BEGIN(InlineAggressiveWrapperPass, "inlineaggressiveanalysis",
                "inline aggressive analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(InlineAggressiveWrapperPass, "inlineaggressiveanalysis",
                "inline aggressive analysis", false, false)

char InlineAggressiveWrapperPass::ID = 0;

ModulePass *llvm::createInlineAggressiveWrapperPassPass() {
  return new InlineAggressiveWrapperPass();
}

InlineAggressiveWrapperPass::InlineAggressiveWrapperPass() : ModulePass(ID) {
  initializeInlineAggressiveWrapperPassPass(*PassRegistry::getPassRegistry());
}

bool InlineAggressiveWrapperPass::doFinalization(Module &M) {
  Result.reset();
  return false;
}

bool InlineAggressiveWrapperPass::runOnModule(Module &M) {
  auto *WPA = getAnalysisIfAvailable<WholeProgramWrapperPass>();
  Result.reset(new InlineAggressiveInfo(
                InlineAggressiveInfo::runImpl(M,
                            WPA ? &WPA->getResult() : nullptr)));
  return false;
}

InlineAggressiveInfo::InlineAggressiveInfo() {
  AggInlCalls.clear();
}

InlineAggressiveInfo::~InlineAggressiveInfo() {}

// Returns true if Aggressive Inline occured.
//
bool InlineAggressiveInfo::isAggInlineOccured(void) {
  if (AggInlCalls.size())
   return true;
  return false;
}

// Mark 'CS' as Inlined-Call by inserting 'CS' into AggInlCalls if
// it is already not there.
//
void InlineAggressiveInfo::setAggInlInfoForCallSite(CallSite CS) {
  if (isCallInstInAggInlList(CS))
    return;
  AggInlCalls.push_back(CS.getInstruction());
}

// Returns true if 'CS' is marked as inlined-call i.e 'CS' is in
// AggInlCall.
//
bool InlineAggressiveInfo::isCallInstInAggInlList(CallSite CS) {
  for (unsigned i = 0, e = AggInlCalls.size(); i != e; ++i) {
    if (AggInlCalls[i] == CS.getInstruction()) 
      return true;
  }
  return false;
}

// Returns true if there are no calls to user defined routines in
// callee of 'CS'. This function is used to prove that formals of 
// a routine are not escaped to any other user defined routines.
//
static bool noCallsToUserDefinedRoutinesInCallee(CallSite CS) {
  Function *F = CS.getCalledFunction();
  if (!F) 
    return false;
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    CallSite CS(cast<Value>(&*II));
    if (!CS)
      continue;
    Function *Callee = CS.getCalledFunction();
    if (!Callee || !Callee->isDeclaration()) {
      return false;
    }
  }
  return true;
}

// Mark all callsites of 'F' as aggressive-inlined-calls.
//
bool InlineAggressiveInfo::setAggInlineInfoForAllCallSites(Function *F) {
  for (const Use &U : F->uses()) {
    User *UR = U.getUser();
    if (!isa<CallInst>(UR)) {
      return false;
    }
    CallSite CS1(cast<CallInst>(UR));
    setAggInlInfoForCallSite(CS1);
  }
  return true;
}

// Mark 'CS' as aggressive-inlined-calls and all callsites of callee of
// 'CS' as aggressive-inlined-calls.
//
bool InlineAggressiveInfo::setAggInlineInfo(CallSite CS)
{
  setAggInlInfoForCallSite(CS);
  Function * Callee = CS.getCalledFunction();
  if (!Callee) 
    return false;
  return setAggInlineInfoForAllCallSites(Callee);

}

// Propagate AggInfo from callsites to called functions recursively.
//
bool InlineAggressiveInfo::propagateAggInlineInfoCall(CallSite CS) {

  Function *Callee = CS.getCalledFunction();

  if (!Callee || Callee->isDeclaration()) {
    return false;
  }
  bool DoInline = false;
  for (inst_iterator II = inst_begin(Callee), E = inst_end(Callee);
       II != E; ++II) {

    CallSite CS1(cast<Value>(&*II));
    if (!CS1)
      continue;
    if (propagateAggInlineInfoCall(CS1)) {
      setAggInlineInfo(CS1);
      DoInline = true;
    }
  }
  if (isCallInstInAggInlList(CS)) 
   DoInline = true;

  return DoInline;
}

// Set AggInfo to any callsite that eventually calls a callsite, which
// is marked as Inlined-call for Aggressive Analysis.
//
bool InlineAggressiveInfo::propagateAggInlineInfo(Function *F) {
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    CallSite CS(cast<Value>(&*II));
    if (!CS)
      continue;
    if (propagateAggInlineInfoCall(CS))
      setAggInlineInfo(CS);
  }

  // Check a limit on number of callsites that marked as inlined-calls.
  if (AggInlCalls.size() > InlineAggressiveCSLimit) 
    return false; 

  // Disable Aggressive analysis if any callsite is marked as both NoInline
  // and aggressive-inlined-call.
  for (unsigned i = 0, e = AggInlCalls.size(); i != e; ++i) {
    CallSite CS(AggInlCalls[i]);
    if (CS.isNoInline())
      return false;
  }

  return true;
}

// Returns true if 'CI' is a malloc call and it is allocating more 
// than 'InlineAggressiveMallocLimit' bytes.
//
static bool isMallocAllocatingHugeMemory(const CallInst *CI) {
  Function *Callee = CI->getCalledFunction();
  StringRef FnName = Callee->getName();
  if (FnName != "malloc")
    return false;
  Value *MallocArg = CI->getArgOperand(0);
  ConstantInt *ConstInt = dyn_cast<ConstantInt>(MallocArg);
  if (!ConstInt)
    return false;
  if (ConstInt->getValue().getZExtValue() < InlineAggressiveMallocLimit)
    return false;

  return true;
}

// Returns true if 'SI' store instruction is saving 'V' in formal
// argument of current routine.
//
//   Ex:   
//      LBM_allocateGrid(double** %ptr)
//      store i8* %V, i8** %ptr
//
static bool isValueSavedInArg(Value *V, StoreInst *SI) {
  if (V != SI->getOperand(0))
    return false;
  Value* V2 = SI->getOperand(1);
  if (Operator::getOpcode(V2) == Instruction::BitCast)
    V2 = cast<Operator>(V2)->getOperand(0);;
  if (!isa<Argument>(V2))
    return false;
  return true;
}
 
// Returns true if return address of malloc is saved in formal argument
// of current routine and not escaped to other places.
// Example for allowed case: 
//
//      LBM_allocateGrid(double** %ptr)
//      %call = call noalias i8* @malloc()
//      %0 = bitcast double** %ptr to i8**
//      store i8* %call, i8** %0
//      %tobool = icmp eq i8* %call, null
//      %add.ptr = getelementptr inbounds i8, i8* %call, i64 
//      %1 = bitcast double** %ptr to i8**
//      store i8* %add.ptr, i8** %1
//
static bool isMallocAddressSavedInArg(Function &F, CallSite CS) {

  // Just limit to single formal pointer parameter for now 
  FunctionType *FTy = F.getFunctionType();
  if (!FTy->getReturnType()->isVoidTy())
    return false;
  if (FTy->getNumParams() != 1)
    return false;
  if (!FTy->getParamType(0)->isPointerTy())
    return false;

  Value* V = CS.getInstruction();

  bool malloc_saved_in_arg = false;
  for (Use &U : V->uses()) {
    User *I = U.getUser();
    if (isa<ICmpInst>(I)) {
      // Ignore use in ICmp
      continue;
    }
    if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(I)) {
      // %add.ptr = getelementptr inbounds i8, i8* %call, i64 0
      // %1 = bitcast double** %ptr to i8**
      // store i8* %add.ptr, i8** %1
      if (!GEPI->hasOneUse()) {
        return false;
      }
      if (StoreInst *SI = dyn_cast<StoreInst>(*GEPI->user_begin())) {
        if (isValueSavedInArg(GEPI, SI)) 
          malloc_saved_in_arg = true;
        else
          return false;
      }
      else
        return false;
    } else  if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
      // store i8* %call, i8** %0
      if (isValueSavedInArg(V, SI)) 
        malloc_saved_in_arg = true;
      else
        return false;
    }
    else {
      return false;
    }
  }
  return malloc_saved_in_arg;
}

// Collect global variable pointers that allocated memory using
// AllocRtn.
//   Ex:
//      LBM_allocateGrid(@srcGrid to double**))
//      LBM_allocateGrid(@dstGrid to double**))
//
// Returns false if AllocRtn is used to allocate memory other than global
// variables. All collected variables are inserted into 'Globals'.
//
static bool collectMemoryAllocatedGlobVarsUsingAllocRtn(
             Function * MallocRtn, std::vector<GlobalVariable*> &Globals) {

  for (Use &U : MallocRtn->uses()) {
    User *UR = U.getUser();
    if (!isa<CallInst>(UR)) {
      return false;
    }
    CallSite CS1(cast<Instruction>(UR));
    if (!CS1.isCallee(&U))
      return false;

    Value *glob = CS1.getArgument(0);
    if (Operator::getOpcode(glob) == Instruction::BitCast)
      glob = cast<Operator>(glob)->getOperand(0);;

    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(glob)) {
      Globals.push_back(GV);
    }
    else {
      return false;
    }
  }
  return true;
}

// It tracks all uses of global variables in 'Globals' and it basically
// does two main things:
//
//   1. Mark callsites as aggressive-inlined-calls if dereference or
//      address of global variables in 'Globals' are passed as arguments.
//        Ex:
//             LBM_allocateGrid(@srcGrid))
//
//             %0 = load @srcGrid, 
//             %arraydecay = getelementptr %0, i64 0,
//             LBM_initializeGrid(double* %arraydecay)
//
//   2. Mark all callsites of a routine if it has any references to the
//      globals variables in 'Globals'
//      Ex:
//             LBM_swapGrids()  {
//                 ...
//                 store i64 %1, @srcGrid to i64*
//                 ...
//             }
//
bool InlineAggressiveInfo::trackUsesofAllocatedGlobalVariables(
                             std::vector<GlobalVariable*> &Globals) {
  for (unsigned i = 0, e = Globals.size(); i != e; ++i) {
    GlobalVariable *GV = Globals[i];
    for (User *U1 : GV->users()) {

      while (!isa<CallInst>(U1)) {
        //   %3 = load [208000000 x double]*, [208000000 x double]** @srcGrid
        //   %arraydecay2 = getelementptr  %3, i64 0, i64 0
        //   @LBM_loadObstacleFile(double* %arraydecay2, i8* nonnull %2) #5
        //
        if (!U1->hasOneUse()) {
          break;
        }
        if (Operator::getOpcode(U1) == Instruction::BitCast ||
            Operator::getOpcode(U1) == Instruction::Load ||
            Operator::getOpcode(U1) == Instruction::GetElementPtr) {
          U1 = *U1->user_begin();
         } else {
           if (InlineAggressiveTrace) {
             errs() << " Skipped AggInl ... unexpeced use of global\n";
             errs() << "      " << *U1  << "\n";
           }
           return false;
         }
      }

      assert(U1 && "Expecting use");

      if (CallInst *CI2 = dyn_cast<CallInst>(U1)) {
        CallSite CS1(CI2);
        // Mark callsite as aggressive-inlined-call only if callee
        // doesn't have any calls to user defined routines so that
        // we can ignore propagating formals to other calls in Callee.
        //
        if (!noCallsToUserDefinedRoutinesInCallee(CS1)) {
          if (InlineAggressiveTrace) {
            errs() << " Skipped AggInl ... global may be escaped in callee\n";
            errs() << "      " << *CS1.getInstruction() << "\n";
          }
          return false; 
        }
        if (InlineAggressiveTrace) {
          errs() << "AggInl:  Marking callsite for inline  \n";
          errs() << "      " << *CS1.getInstruction() << "\n";
        }
        setAggInlInfoForCallSite(CS1);
        continue;
      }

      // Process all uses if user has multiple uses.
      for (User *U2 : U1->users()) {
        if (Operator::getOpcode(U2) == Instruction::Store) {
          //  LBM_swapGrids()  {
          //     ...
          //     store i64 %1, @srcGrid to i64*)
          Function *F1 = cast<Instruction>(U2)->getParent()->getParent();
          if (InlineAggressiveTrace) {
            errs() << "AggInl:  Marking all callsites of ";
            errs() << F1->getName() << "\n";
          }
          setAggInlineInfoForAllCallSites(F1);
        } else if (CallInst *CI2 = dyn_cast<CallInst>(U2)) {
          // %7 = load @srcGrid,
          // %arraydecay8 = getelementptr %7, i64 0
          // LBM_initializeSpecialCellsForChannel(double* %arraydecay8)
          // LBM_initializeSpecialCellsForLDC(double* %arraydecay8)
          CallSite CS1(CI2);
          if (!noCallsToUserDefinedRoutinesInCallee(CS1)) {
            if (InlineAggressiveTrace) {
              errs() << " Skipped AggInl ...global may be escaped in callee\n";
              errs() << "      " << *CS1.getInstruction() << "\n";
            }
            return false;
          }
          if (InlineAggressiveTrace) {
            errs() << "AggInl:  Marking callsite for inline  \n";
            errs() << "      " << *CS1.getInstruction() << "\n";
          }
          setAggInlInfoForCallSite(CS1);
        } else if (Operator::getOpcode(U2) == Instruction::Load) {
          for (User *U3 : U2->users()) {
            if (Operator::getOpcode(U3) != Instruction::Store) {
              if (InlineAggressiveTrace) {
                errs() << " Skipped AggInl ... unexpected use of global\n";
                errs() << "      " << *U3  << "\n";
              }
              return false;
            }
            Function *F1 = cast<Instruction>(U3)->getParent()->getParent();
            if (InlineAggressiveTrace) {
              errs() << "AggInl:  Marking all callsites of ";
              errs() << F1->getName() << "\n";
            }
            setAggInlineInfoForAllCallSites(F1);
          }
        } else {
           if (InlineAggressiveTrace) {
             errs() << " Skipped AggInl ... unexpected use of global\n";
             errs() << "      " << *U2  << "\n";
           }
          return false;
        }
      }
    }
  }
  return true;
}

InlineAggressiveInfo  InlineAggressiveInfo::runImpl(Module &M,
                                              WholeProgramInfo *WPI) {
  InlineAggressiveInfo Result;

  //auto *WPA = getAnalysisIfAvailable<WholeProgramWrapperPass>();
  if (!WPI || !WPI->isWholeProgramSafe()) {
    if (InlineAggressiveTrace) {
      errs() << " Skipped AggInl ... Whole Program NOT safe \n";
    }
    return Result;
  }
  Result.analyzeModule(M); 
  return Result;
}
 
bool InlineAggressiveInfo::analyzeModule(Module &M) {

  Function *AllocRtn = nullptr;
  Function *MainRtn = nullptr;

  uint64_t TotalInstCount = 0;

  AggInlCalls.clear();

  for (Function &F : M) {

    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    if (!F.doesNotRecurse()) {
      if (InlineAggressiveTrace) {
        errs() << " Skipped AggInl ..." << F.getName() << " is recursive \n";
      }
      return false;
    }

    if (F.getName() == "main")
      MainRtn = &F;

    for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {

      TotalInstCount++;
      if (isa<InvokeInst>(&*II)) {
        if (InlineAggressiveTrace) {
          errs() << " Skipped AggInl ... InvokeInst is seen";
        }
        return false;
      }
      const CallInst *CI = dyn_cast<CallInst>(&*II);
      if (!CI || !isa<CallInst>(CI))
        continue;
      Function *Callee = CI->getCalledFunction();

      if (Callee == nullptr) {
        if (InlineAggressiveTrace) {
          errs() << " Skipped AggInl ... Indirect call is seen";
        }
        return false;
      }
      if (!isMallocAllocatingHugeMemory(CI)) {
        continue;
      }

      CallSite CS = CallSite(&*II);
      if (isMallocAddressSavedInArg(F, CS)) {
        if (AllocRtn != nullptr) {
          if (InlineAggressiveTrace) {
            errs() << " Skipped AggInl ... Found more than 1 malloc routine";
          }
          return false;
        }
        AllocRtn = &F;
      }
    }

    if (TotalInstCount > InlineAggressiveInstLimit) {
      if (InlineAggressiveTrace) {
        errs() << " Skipped AggInl ... too many instructions";
      }
      return false;
    }

  }
  if (InlineAggressiveTrace)
    errs() << " Total inst: " << TotalInstCount << "\n";
    
  if (AllocRtn == nullptr) {
    if (InlineAggressiveTrace) {
      errs() << " Skipped AggInl ... No malloc routine found";
    }
    return false;
  }

  if (InlineAggressiveTrace) {
    errs() << "AggInl: " << AllocRtn->getName() << " malloc routine found\n";
  }

  std::vector<GlobalVariable*> AllocatedGlobals;
  AllocatedGlobals.clear();

  if (!collectMemoryAllocatedGlobVarsUsingAllocRtn(AllocRtn,
                                          AllocatedGlobals)) {
    if (InlineAggressiveTrace) {
      errs() << " Skipped AggInl ... Not able to collect Allocated Globals\n";
    }
    return false;
  }

  if (AllocatedGlobals.empty()) {
    if (InlineAggressiveTrace) {
      errs() << " Skipped AggInl ... No Allocated Globals found";
    }
    return false;
  }

  if (InlineAggressiveTrace) {
    errs() << "AggInl:  collected globals \n";
    for (unsigned i = 0, e = AllocatedGlobals.size(); i != e; ++i) {
      errs() << "      " << *AllocatedGlobals[i] << "\n"; 
    }
  }

  if (!trackUsesofAllocatedGlobalVariables(AllocatedGlobals)) {
    AggInlCalls.clear();
    if (InlineAggressiveTrace) {
      errs() << " Skipped AggInl ... can't track uses of Allocated Globals\n";
    }
    return false;
  }

  if (!propagateAggInlineInfo(MainRtn)) {
    AggInlCalls.clear();
    if (InlineAggressiveTrace) {
      errs() << " Skipped AggInl ... can't propagate Agg Inline info\n";
    }
    return false;
  }

  if (InlineAggressiveTrace) {
    errs() << "AggInl:  All CallSites marked for inline after propagation\n";
    for (unsigned i = 0, e = AggInlCalls.size(); i != e; ++i) {
      errs() << "      " << *AggInlCalls[i]  << "\n"; 
    }
  }

  return true;
}

// This analysis depends on WholeProgramAnalysis. Analysis info is not
// modified by any other pass.
//
void InlineAggressiveWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addUsedIfAvailable<WholeProgramWrapperPass>();
}

char InlineAggAnalysis::PassID;

// Provide a definition for the static class member used to identify passes. 
AnalysisKey InlineAggAnalysis::Key;

InlineAggressiveInfo InlineAggAnalysis::run(Module &M,
                                AnalysisManager<Module> &AM) {
  
  return InlineAggressiveInfo::runImpl(M,
                              AM.getCachedResult<WholeProgramAnalysis>(M));
}


