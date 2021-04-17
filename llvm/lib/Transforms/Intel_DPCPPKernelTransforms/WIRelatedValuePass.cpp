//==--- WIRelatedValue.cpp - Detect values dependent on TIDs - C++ -*-------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WIRelatedValuePass.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"

#include <set>

using namespace llvm;
WIRelatedValue::WIRelatedValue(Module &M) {
  //Initialize barrier utils class with current module
  BarrierUtils.init(&M);

  // Calculate the calling order for the functions to be analyzed.
  calculateCallingOrder();

  // Run over the functions according to the calling order
  // i.e. analyze caller function before callee.
  for (Function *F : OrderedFunctionsToAnalyze) {
    // Update function argument dependency based on passed operands,
    // i.e. if there is one caller that passes non-uniform value to
    // and argument, then consider that argument non-uniform too.
    updateArgumentsDep(F);
    runOnFunction(*F);
  }
}

bool WIRelatedValue::runOnFunction(Function &F) {
  ChangedValues.clear();

  // Schedule all instructions for calculation.
  for (auto &I : instructions(F)) {
    ChangedValues.insert(&I);
  }

  updateDeps();

  return false;
}

void WIRelatedValue::updateDeps() {
  // As long as we have values to update...
  while (!ChangedValues.empty()) {
    // Move the list aside and clear original list for next iteration.
    std::vector<Value *> Changed = ChangedValues.takeVector();
    // Update all changed values.
    for (auto *V : Changed) {
      // Remove first instruction,
      // calculate its new dependencey value.
      calculateDep(V);
    }
  }
}

bool WIRelatedValue::getWIRelation(Value *V) {
  // New instruction to consider.
  // Mark it as not related to WI Id till farther update.
  SpecialValues.insert({V, false});

  return SpecialValues[V];
}

void WIRelatedValue::calculateDep(Value *V) {

  Instruction *I = dyn_cast<Instruction>(V);

  if (!I) {
    llvm_unreachable("We are runing on instruction list, can we reach here?");
    // Not an instruction, must be a constant or an argument.
    // Could this vector type be of a constant which is not uniform?
  }

  // New instruction to consider.
  // Mark it as not related to WI Id till farther update.
  SpecialValues.insert({I, false});

  // Our initial value
  bool OrigRelation = SpecialValues[I];
  bool NewRelation = OrigRelation;

  // LLVM does not have compile time polymorphisms.
  // TODO: to make things faster we may want to sort the list below according
  // to the order of their probability of appearance.
  if (BinaryOperator *Op = dyn_cast<BinaryOperator>(I)) {
    NewRelation = calculateDep(Op);
  } else if (CallInst *CI = dyn_cast<CallInst>(I)) {
    NewRelation = calculateDep(CI);
  } else if (CmpInst *Cmp = dyn_cast<CmpInst>(I)) {
    NewRelation = calculateDep(Cmp);
  } else if (ExtractElementInst *Inst = dyn_cast<ExtractElementInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (GetElementPtrInst *Inst = dyn_cast<GetElementPtrInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (InsertElementInst *Inst = dyn_cast<InsertElementInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (InsertValueInst *Inst = dyn_cast<InsertValueInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (PHINode *Inst = dyn_cast<PHINode>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (ShuffleVectorInst *Inst = dyn_cast<ShuffleVectorInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (StoreInst *Inst = dyn_cast<StoreInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (I->isTerminator()) {
    NewRelation = calculateDepTerminator(I);
  } else if (SelectInst *Inst = dyn_cast<SelectInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (AllocaInst *Inst = dyn_cast<AllocaInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (CastInst *Inst = dyn_cast<CastInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (ExtractValueInst *Inst = dyn_cast<ExtractValueInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (LoadInst *Inst = dyn_cast<LoadInst>(I)) {
    NewRelation = calculateDep(Inst);
  } else if (VAArgInst *Inst = dyn_cast<VAArgInst>(I)) {
    NewRelation = calculateDep(Inst);
  }

  // If the value was changed in this calculation.
  if (NewRelation != OrigRelation) {
    // Save the new value of this instruction.
    SpecialValues[I] = NewRelation;
    // Register for update all of the dependent values of this updated
    // instruction.
    for (auto *U : I->users()) {
      ChangedValues.insert(U);
    }
  }
}

bool WIRelatedValue::calculateDep(BinaryOperator *Op) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = Op->getOperand(0);
  Value *Op1 = Op->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(CallInst *CI) {
  // TODO: This function requires much more work, to be correct:
  //   2) Some functions (dot_prod, cross_prod) provide "measurable"
  //   behavior (Uniform->strided).
  //   This information should also be obtained from RuntimeServices somehow.

  // Check if the function is in the table of functions.
  Function *OrigFunc = CI->getCalledFunction();
  if (!OrigFunc) {
    assert(false && "Unexpected indirect call!");
    return true;
  }

  // Check if call is TID-generator.
  if (OrigFunc->getName() == DPCPPKernelCompilationUtils::mangledGetLID() ||
      OrigFunc->getName() == DPCPPKernelCompilationUtils::mangledGetGID()) {
    // These functions return WI Id, they are indeed WI Id related.
    return true;
  }

  // TODO: port analysis of work group functions,
  // such as all, any, broadcast, etc., together with associated LIT tests.

  // Check if function is not declared inside "this" module
  if (!OrigFunc->isDeclaration()) {
    // For functions defined (not declared) in this module - it is unsafe to
    // assume anything.
    // TODO: can we check the function and assure it is not related on WI-Id?
    return true;
  }

  // Iterate over all input dependencies. If all are not WI Id related -
  // propagate it. Otherwise - return WI Id related.
  unsigned int NumParams = CI->getNumArgOperands();

  bool IsWIRelated = false;
  for (unsigned int i = 0; i < NumParams; ++i) {
    // Operand 0 is the function's name.
    Value *Op = CI->getArgOperand(i);
    IsWIRelated = IsWIRelated || getWIRelation(Op);
    if (IsWIRelated) {
      break; // Non related check failed. No need to continue.
    }
  }
  return IsWIRelated;
}

bool WIRelatedValue::calculateDep(CmpInst *I) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(ExtractElementInst *I) {
  // Return the WI relation of the only one operand.
  Value *Op0 = I->getOperand(0);

  bool Dep0 = getWIRelation(Op0);

  return Dep0;
}

bool WIRelatedValue::calculateDep(GetElementPtrInst *I) {
  // Calculate the WI relation for each of the operands.
  unsigned int Num = I->getNumIndices();
  Value *Op0 = I->getPointerOperand();

  bool Dep = getWIRelation(Op0);

  for (unsigned int i = 0; i < Num; ++i) {
    Dep = Dep || getWIRelation(I->getOperand(i + 1));
  }

  return Dep;
}

bool WIRelatedValue::calculateDep(InsertElementInst *I) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(InsertValueInst *I) {
  // TODO: why should we always return related?
  return true;
}

bool WIRelatedValue::calculateDep(PHINode *Phi) {
  // Calculate the WI relation for each of the operands.
  // unsigned int Num = Phi->getNumIncomingValues();
  // bool dep = false;

  // for ( unsigned int i=0; i < Num; ++i ) {
  //  Value *op = Phi->getIncomingValue(i);
  //  dep = dep || getWIRelation(op);
  //}

  // return dep;
  // TODO: CSSD100007559 (should fix the following case and then remove the
  // always return true!): %isOk.0 = phi i1 [ false, %4 ], [ true, %0 ]
  return true;
}

bool WIRelatedValue::calculateDep(ShuffleVectorInst *I) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(StoreInst *I) {
  // No need to handle store instructions as alloca is handled separately.
  return false;
}

bool WIRelatedValue::calculateDepTerminator(Instruction *I) {
  assert(I->isTerminator() && "Expect a terminator instruction!");
  // Instruction has no return value.
  // Just need to know if this inst is uniform or not,
  // because we may want to avoid predication if the control flows
  // in the function are uniform...
  switch (I->getOpcode()) {
  case Instruction::Br: {
    BranchInst *BrInst = cast<BranchInst>(I);
    if (BrInst->isConditional()) {
      // Conditional branch is uniform, if its condition is uniform.
      Value *Op = BrInst->getCondition();
      return getWIRelation(Op);
    }
    // Unconditional branch is non TID-dependent.
    return false;
  }
  case Instruction::IndirectBr:
    // TODO: Define the dependency requirements of indirectBr.
  default:
    return true;
  }
}

bool WIRelatedValue::calculateDep(SelectInst *I) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);
  Value *Op2 = I->getOperand(2);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);
  bool Dep2 = getWIRelation(Op2);

  return (Dep0 || Dep1 || Dep2);
}

bool WIRelatedValue::calculateDep(AllocaInst *Alloca) {
  // Alloca instruction is assumed to be non-uniform.
  // In fact, It is stored in special buffer always in the current design!
  return true;
}

bool WIRelatedValue::calculateDep(CastInst *I) {
  // Return the WI relation of the only one operand.
  Value *Op0 = I->getOperand(0);

  bool Dep0 = getWIRelation(Op0);

  return Dep0;
}

bool WIRelatedValue::calculateDep(ExtractValueInst *I) {
  // TODO: why should we always return related?
  return true;
}

bool WIRelatedValue::calculateDep(LoadInst *Load) {
  // No need to handle load instructions as alloca is handled separately.
  // Return the WI relation of the only one operand.
  Value *Op0 = Load->getOperand(0);

  bool Dep0 = getWIRelation(Op0);

  return Dep0;
}

bool WIRelatedValue::calculateDep(VAArgInst *I) {
  assert(false && "Are we supporting this ??");
  return false;
}

void WIRelatedValue::updateArgumentsDep(Function *F) {
  for (auto IdxArgPair : enumerate(F->args())) {
    Argument *Arg = &IdxArgPair.value();
    for (User *U : F->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      // Usage of F can be a global variable!
      if (!CI)
        continue;

      if (getWIRelation(CI->getOperand(IdxArgPair.index()))) {
        SpecialValues[Arg] = true;
      }
    }
  }
}

/// FuncNameComp - compare two Function's by their names.
struct FuncNameComp {
  bool operator()(const Function *A, const Function *B) const {
    return A->getName() < B->getName();
  }
};

void WIRelatedValue::calculateCallingOrder() {
  // SetStableIterFunc sorts functions in alphabetical order and not by the
  // pointer to the function, which gurantees a stable iterator. This relies
  // on the assumption that there are no two functions in the module with the
  // same name.
  using SetStableIterFunc = std::set<Function *, FuncNameComp>;
  SetStableIterFunc FuncsToHandle;

  // Initialize functionToHandle container with functions that need to be
  // analyzed Find all functions that call synchronize instructions
  FuncSet& FuncsWithSync = BarrierUtils.getAllFunctionsWithSynchronization();
  // Collect data for each function with synchronize instruction
  for (Function *F : FuncsWithSync) {
    FuncsToHandle.insert(F);
  }
  while (!FuncsToHandle.empty()) {
    for (Function *F : FuncsToHandle) {
      bool IsRoot = true;
      for (auto *U : F->users()) {
        if (!isa<CallInst>(U)) {
          // Something other than CallInst is using function!
          continue;
        }
        CallInst *CI = cast<CallInst>(U);
        Function *CallerFunc = CI->getFunction();
        if (FuncsToHandle.count(CallerFunc)) {
          IsRoot = false;
          break;
        }
      }
      if (IsRoot) {
        OrderedFunctionsToAnalyze.push_back(F);
        FuncsToHandle.erase(F);
        break;
      }
    }
  }
}

void WIRelatedValue::print(raw_ostream &OS, const Module *M) const {
  if (!M) {
    OS << "No Module!\n";
    return;
  }
  // Print Module.
  OS << *M;

  // Run on all WI related values.
  OS << "\nWI related Values\n";
  for (const auto &F : *M) {
    for (const auto &I : instructions(F)) {
      // Store and Return instructions has no value (i.e. no name) don't print
      // them!
      if (isa<StoreInst>(&I) || isa<ReturnInst>(&I))
        continue;
      Value *V = const_cast<Value *>((const Value *)&I);
      bool IsWIRelated =
          SpecialValues.count(V) ? SpecialValues.find(V)->second : false;
      // Print vale name is (not) WI related!
      OS << V->getName().str();
      OS << ((IsWIRelated) ? " is WI related" : " is not WI related");
      OS << "\n";
    }
  }
}

INITIALIZE_PASS(WIRelatedValueWrapper, "dpcpp-kernel-barrier-wi-analysis",
                "Intel DPCPP Barrier Pass - Calculate WI relation per Value",
                false, true)

char WIRelatedValueWrapper::ID = 0;

WIRelatedValueWrapper::WIRelatedValueWrapper() : ModulePass(ID) {
  initializeWIRelatedValueWrapperPass(*PassRegistry::getPassRegistry());
}

bool WIRelatedValueWrapper::runOnModule(Module &M) {
  WRV.reset(new WIRelatedValue{M});
  return false;
}

AnalysisKey WIRelatedValueAnalysis::Key;

WIRelatedValue WIRelatedValueAnalysis::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  return WIRelatedValue{M};
}

PreservedAnalyses WIRelatedValuePrinter::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  MAM.getResult<WIRelatedValueAnalysis>(M).print(OS, &M);
  return PreservedAnalyses::all();
}

ModulePass *llvm::createWIRelatedValueWrapperPass() {
  return new WIRelatedValueWrapper();
}
