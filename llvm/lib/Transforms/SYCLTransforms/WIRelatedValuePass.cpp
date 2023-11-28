//==--- WIRelatedValue.cpp - Detect values dependent on TIDs - C++ -*-------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/WIRelatedValuePass.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Support/raw_ostream.h"

#include <set>

using namespace llvm;

WIRelatedValue::WIRelatedValue(Module &M) {
  // Initialize barrier utils class with current module.
  Utils.init(&M);

  // Bypass WIRelatedValue analysis since calculateCallingOrder doesn't
  // support recursion and this action makes all values become uniform.
  if (!Utils.getRecursiveFunctionsWithSync().empty())
    return;
  // Calculate the calling order for the functions to be analyzed.
  calculateCallingOrder();

  // Run over the functions according to the calling order
  // i.e. analyze caller function before callee.
  for (Function *Func : OrderedFunctionsToAnalyze) {
    // Update function argument dependency based on passed operands.
    // i.e. if there is one caller that passes non-uniform value to
    // and argument, then consider that argument non-uniform too.
    updateArgumentsDep(Func);
    runOnFunction(*Func);
  }
}

bool WIRelatedValue::runOnFunction(Function &F) {
  Changed.clear();

  // Schedule all instruction for calculation.
  for (Instruction &I : instructions(F))
    calculateDep(&I);

  updateDeps();
  return false;
}

void WIRelatedValue::updateDeps() {
  // As long as we have values to update.
  while (!Changed.empty()) {
    // Move the list aside and clear original list for next iteration.
    const auto ChangedValues = Changed.takeVector();
    // Update all changed values.
    for (Value *V : ChangedValues)
      calculateDep(V);
  }
}

bool WIRelatedValue::getWIRelation(Value *Val) {
  // New instruction to consider.
  // Mark it as not related to WI Id till further update.
  auto It = SpecialValues.insert({Val, false});
  return It.first->second;
}

void WIRelatedValue::calculateDep(Value *Val) {

  Instruction *Inst = dyn_cast<Instruction>(Val);

  if (!Inst) {
    llvm_unreachable("we are runing on instruction list, can we reach here?");
    // Not an instruction, must be a constant or an argument
    // Could this vector type be of a constant which is not uniform?
  }

  // New instruction to consider.
  // Mark it as not related to WI Id till further update.
  auto It = SpecialValues.insert({Val, false});

  bool OrigRelation = It.first->second;
  bool NewRelation = OrigRelation;

  // TODO:
  // For most of instructions, we can conservatively assume:
  //   NewRelation = any_of(getWIRelation(Operand))
  // e.g. BinaryOperator, CmpInst, ExtractElementInst, FreezeInst, ...
  // So that we can only explicitly handle instructions that should be treated
  // specially.
  // e.g. CallInst, PHINode, AllocaInst, ...
  if (BinaryOperator *I = dyn_cast<BinaryOperator>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (CallInst *I = dyn_cast<CallInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (CmpInst *I = dyn_cast<CmpInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (ExtractElementInst *I = dyn_cast<ExtractElementInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (GetElementPtrInst *I = dyn_cast<GetElementPtrInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (InsertElementInst *I = dyn_cast<InsertElementInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (InsertValueInst *I = dyn_cast<InsertValueInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (PHINode *I = dyn_cast<PHINode>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (ShuffleVectorInst *I = dyn_cast<ShuffleVectorInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (StoreInst *I = dyn_cast<StoreInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (Inst->isTerminator()) {
    NewRelation = calculateDepTerminator(Inst);
  } else if (SelectInst *I = dyn_cast<SelectInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (AllocaInst *I = dyn_cast<AllocaInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (CastInst *I = dyn_cast<CastInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (ExtractValueInst *I = dyn_cast<ExtractValueInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (LoadInst *I = dyn_cast<LoadInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (VAArgInst *I = dyn_cast<VAArgInst>(Inst)) {
    NewRelation = calculateDep(I);
  } else if (FreezeInst *I = dyn_cast<FreezeInst>(Inst)) {
    NewRelation = calculateDep(I);
  }

  // If the value was changed in this calculation.
  if (NewRelation != OrigRelation) {
    // Save the new value of this instruction.
    SpecialValues[Inst] = NewRelation;
    // Register for update all of the dependent values of this updated
    // instruction.
    for (Value *V : Inst->users()) {
      Changed.insert(V);
    }
  }
}

bool WIRelatedValue::calculateDep(BinaryOperator *Inst) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = Inst->getOperand(0);
  Value *Op1 = Inst->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(CallInst *Inst) {
  // TODO: This function requires much more work, to be correct:
  //   2) Some functions (dot_prod, cross_prod) provide "measurable"
  //   behavior (Uniform->strided).
  //   This information should also be obtained from RuntimeServices somehow.

  auto hasWIRelatedArgs = [=]() {
    return any_of(Inst->operands(), [=](Value *V) { return getWIRelation(V); });
  };

  // Check if the function is in the table of functions
  Function *OrigFunc = Inst->getCalledFunction();
  if (!OrigFunc)
    return !Inst->doesNotAccessMemory() || hasWIRelatedArgs();

  StringRef OrigFuncName = OrigFunc->getName();

  // Check if call is TID-generator.
  if (CompilationUtils::isGetGlobalId(OrigFuncName) ||
      CompilationUtils::isGetLocalId(OrigFuncName)) {
    // These functions return WI Id, they are indeed WI Id related.
    return true;
  }

  std::string OrigWGFuncName = OrigFuncName.str();
  if (CompilationUtils::hasWorkGroupFinalizePrefix(OrigFuncName)) {
    // Remove the finalize prefix from work group function to
    // get the Original work group function name to check against below.
    OrigWGFuncName =
        CompilationUtils::removeWorkGroupFinalizePrefix(OrigFuncName);
  }

  if (CompilationUtils::isWorkGroupScan(OrigWGFuncName)) {
    // WG scan functions are WI Id related.
    return true;
  } else if (CompilationUtils::isWorkGroupUniform(OrigWGFuncName)) {
    // WG uniform functions are WI Id unrelated.
    return false;
  }

  if (CompilationUtils::isWorkGroupReserveReadPipe(OrigWGFuncName) ||
      CompilationUtils::isWorkGroupReserveWritePipe(OrigWGFuncName)) {
    // WG reserve pipe built-ins are WI unrelated.
    return false;
  }

  if (CompilationUtils::isAtomicBuiltin(OrigFuncName) ||
      CompilationUtils::isWorkItemPipeBuiltin(OrigFuncName)) {
    // Atomic and pipe built-ins are WI Id related.
    return true;
  }

  // We assume all other declarations (built-in functions) are constant.
  // TODO:
  // 1) Can we remove OrigFunc->isDeclaration()? I.e., do all other built-ins
  // have readnone attribute?
  // 2) Can we further remove all assertion on function names above? I.e., Do
  // all WI-related built-ins have readnone attribute?
  if ((OrigFunc->isDeclaration() || Inst->doesNotAccessMemory()) &&
      !hasWIRelatedArgs()) {
    return false;
  }

  return true;
}

bool WIRelatedValue::calculateDep(CmpInst *Inst) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = Inst->getOperand(0);
  Value *Op1 = Inst->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(ExtractElementInst *Inst) {
  // Return the WI relation of the only one operand.
  Value *Op0 = Inst->getOperand(0);

  bool Dep0 = getWIRelation(Op0);

  return Dep0;
}

bool WIRelatedValue::calculateDep(GetElementPtrInst *Inst) {
  // Calculate the WI relation for each of the operands.
  unsigned int Num = Inst->getNumIndices();
  Value *Op0 = Inst->getPointerOperand();

  bool Dep = getWIRelation(Op0);

  for (unsigned int i = 0; i < Num; ++i) {
    Dep = Dep || getWIRelation(Inst->getOperand(i + 1));
  }

  return Dep;
}

bool WIRelatedValue::calculateDep(InsertElementInst *Inst) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = Inst->getOperand(0);
  Value *Op1 = Inst->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(InsertValueInst * /*Inst*/) {
  // TODO: why should we always return related?
  return true;
}

bool WIRelatedValue::calculateDep(PHINode * /*Inst*/) {
  // Calculate the WI relation for each of the operands
  // unsigned int Num = Inst->getNumIncomingValues();
  // bool Dep = false;

  // for ( unsigned int i=0; i < Num; ++i ) {
  //   Value *op = Inst->getIncomingValue(i);
  //   Dep = Dep || getWIRelation(op);
  // }

  // return Dep;
  // TODO: CSSD100007559 (should fix the following case and then remove the
  // always return true!): %isOk.0 = phi i1 [ false, %4 ], [ true, %0 ]
  return true;
}

bool WIRelatedValue::calculateDep(ShuffleVectorInst *Inst) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = Inst->getOperand(0);
  Value *Op1 = Inst->getOperand(1);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);

  return (Dep0 || Dep1);
}

bool WIRelatedValue::calculateDep(StoreInst * /*Inst*/) {
  // No need to handle store instructions as alloca is handled separately.
  return false;
}

bool WIRelatedValue::calculateDepTerminator(Instruction *Inst) {
  assert(Inst->isTerminator() && "Expect a terminator instruction!");
  // Instruction has no return value.
  // Just need to know if this inst is uniform or not,
  // because we may want to avoid predication if the control flows
  // in the function are uniform...
  switch (Inst->getOpcode()) {
  case Instruction::Br: {
    BranchInst *BrInst = cast<BranchInst>(Inst);
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

bool WIRelatedValue::calculateDep(SelectInst *Inst) {
  // Calculate the WI relation for each of the operands.
  Value *Op0 = Inst->getOperand(0);
  Value *Op1 = Inst->getOperand(1);
  Value *Op2 = Inst->getOperand(2);

  bool Dep0 = getWIRelation(Op0);
  bool Dep1 = getWIRelation(Op1);
  bool Dep2 = getWIRelation(Op2);

  return (Dep0 || Dep1 || Dep2);
}

bool WIRelatedValue::calculateDep(AllocaInst * /*Inst*/) {
  // Alloca instruction is assumed to be non-uniform. It is stored in special
  // buffer always in the current design.
  return true;
}

bool WIRelatedValue::calculateDep(CastInst *Inst) {
  // Return the WI relation of the only one operand.
  Value *Op0 = Inst->getOperand(0);

  bool Dep0 = getWIRelation(Op0);

  return Dep0;
}

bool WIRelatedValue::calculateDep(ExtractValueInst * /*Inst*/) {
  // TODO: why should we always return related?
  return true;
}

bool WIRelatedValue::calculateDep(LoadInst *Inst) {
  // Return the WI relation of the only one operand.
  Value *Op0 = Inst->getOperand(0);

  bool Dep0 = getWIRelation(Op0);

  return Dep0;
}

bool WIRelatedValue::calculateDep(VAArgInst * /*Inst*/) {
  llvm_unreachable("Are we supporting this ?");
  return false;
}

bool WIRelatedValue::calculateDep(FreezeInst *Inst) {
  return getWIRelation(Inst->getOperand(0));
}

void WIRelatedValue::updateArgumentsDep(Function *Func) {
  for (const auto &ArgItPair : enumerate(Func->args())) {
    Argument *Arg = &ArgItPair.value();
    unsigned Idx = ArgItPair.index();
    for (User *U : Func->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      // Usage of Func can be a global variable!
      if (!CI)
        continue;

      if (getWIRelation(CI->getOperand(Idx)))
        SpecialValues[Arg] = true;
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
  typedef std::set<Function *, FuncNameComp> SetStableIterFunc;
  SetStableIterFunc FunctionsToHandle;

  // Initialize functionToHandle container with functions that need to be
  // analyzed Find all functions that call synchronize instructions.
  CompilationUtils::FuncSet FunctionsWithSync =
      Utils.getAllFunctionsWithSynchronization();
  // Collect data for each function with synchronize instruction.
  for (Function *Func : FunctionsWithSync)
    FunctionsToHandle.insert(Func);

  while (!FunctionsToHandle.empty()) {
    for (Function *Func : FunctionsToHandle) {
      bool IsRoot = true;
      for (User *U : Func->users()) {
        if (!isa<CallInst>(U)) {
          // Something other than CallInst is using function!
          continue;
        }
        CallInst *CI = cast<CallInst>(U);
        Function *CallerFunc = CI->getCaller();
        if (FunctionsToHandle.count(CallerFunc)) {
          IsRoot = false;
          break;
        }
      }
      if (IsRoot) {
        OrderedFunctionsToAnalyze.push_back(Func);
        FunctionsToHandle.erase(Func);
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
  ModuleSlotTracker MST(M);
  for (const auto &F : *M) {
    MST.incorporateFunction(F);
    for (const auto &I : instructions(F)) {
      // Store and Return instructions has no value (i.e. no name) don't print
      // them!
      if (isa<StoreInst>(&I) || isa<ReturnInst>(&I))
        continue;
      Value *V = const_cast<Value *>((const Value *)&I);
      bool IsWIRelated =
          SpecialValues.count(V) ? SpecialValues.find(V)->second : false;
      // Print vale name is (not) WI related!
      if (V->hasName() || MST.getLocalSlot(V) != -1)
        V->printAsOperand(OS, /*PrintType*/ false, MST);
      else
        OS << '"' << *V << '"';
      OS << ((IsWIRelated) ? " is WI related" : " is not WI related");
      OS << "\n";
    }
  }
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
