/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "LoopUtils.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {
namespace LoopUtils {

Function *getWIFunc(Module *M, StringRef name, Type *retTy) {
  // First look in the module if it already exists
  Function *func = M->getFunction(name);
  if (func)
    return func;
  // create declaration.
  std::vector<Type *> argTypes(1, Type::getInt32Ty(retTy->getContext()));
  FunctionType *fType = FunctionType::get(retTy, argTypes, false);
  Constant *fConst = M->getOrInsertFunction(name, fType);
  func = dyn_cast<Function>(fConst);
  assert(func && "null WI function");
  return func;
}

CallInst *getWICall(Module *M, StringRef funcName, Type *retTy, unsigned dim,
                    BasicBlock *BB, const Twine &callName) {
  Function *WIFunc = getWIFunc(M, funcName, retTy);
  Constant *dimConst =
      ConstantInt::get(Type::getInt32Ty(retTy->getContext()), dim);
  CallInst *WICall = CallInst::Create(WIFunc, dimConst, callName, BB);
  return WICall;
}

Type *getIndTy(Module *M) {
  const DataLayout* DL = M->getDataLayout();
  assert(DL != NULL && "cannot get pointer size without the data layout");
  unsigned pointerSize = DL->getPointerSize();
  switch (pointerSize) {
  case 32:
  case 64:
    return IntegerType::get(M->getContext(), pointerSize);
  default:
    assert(0 && "pointer size != 32 , 64");
    return NULL;
  }
}

void getAllCallInFunc(StringRef funcName, Function *funcToSearch,
                      SmallVectorImpl<CallInst *> &calls) {
  Function *F = funcToSearch->getParent()->getFunction(funcName);
  if (!F)
    return;

  for (Value::user_iterator useIt = F->user_begin(), useE = F->user_end();
       useIt != useE; ++useIt) {
    CallInst *CI = dyn_cast<CallInst>(*useIt);
    Function *parentFunc = CI->getParent()->getParent();
    if (parentFunc == funcToSearch)
      calls.push_back(CI);
  }
}

bool inSubLoop(Loop *L, BasicBlock *BB) {
  for (Loop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    if ((*I)->contains(BB))
      return true; // In a subloop.
  }
  return false;
}

bool inSubLoop(Loop *L, Instruction *I) {
  BasicBlock *BB = I->getParent();
  return inSubLoop(L, BB);
}

void
getBlocksExecutedExactlyOnce(Loop *L, DominatorTree *DT,
                             SmallVectorImpl<BasicBlock *> &alwaysExecuteOnce) {
  assert(DT && "NULL dominator tree");
  alwaysExecuteOnce.clear();
  // Get the exit blocks for the current loop.
  SmallVector<BasicBlock *, 8> ExitBlocks;
  L->getExitBlocks(ExitBlocks);

  // Verify that the block dominates each of the exit blocks of the loop.
  for (Loop::block_iterator BBI = L->block_begin(), BBE = L->block_end();
       BBI != BBE; ++BBI) {
    // If this block is in sub loop than it might be executed more than once.
    if (inSubLoop(L, *BBI))
      continue;

    // Check that the block dominates all exit blocks.
    bool dominatesAll = true;
    for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
      if (!DT->dominates(*BBI, ExitBlocks[i]))
        dominatesAll = false;
    }
    if (dominatesAll)
      alwaysExecuteOnce.push_back(*BBI);
  }
}

void fillDirectUsers(std::set<Function *> *funcs,
                     std::set<Function *> *userFuncs,
                     std::set<Function *> *newUsers) {
  // Go through all of the funcs.
  SmallVector<Instruction *, 8> userInst;
  for (std::set<Function *>::iterator fit = funcs->begin(), fe = funcs->end();
       fit != fe; ++fit) {
    Function *F = *fit;
    if (!F)
      continue;

    // Get the instruction users of the function, and insert their
    // parent functions to the userFuncs set.
    userInst.clear();
    fillInstructionUsers(F, userInst);
    for (unsigned i = 0, e = userInst.size(); i < e; ++i) {
      Instruction *I = userInst[i];
      Function *userFunc = I->getParent()->getParent();
      assert(userFunc && "NULL parent function ?");
      // If the user is a call add the function contains it to userFuncs
      if (userFuncs->insert(userFunc).second) {
        // If the function is new update the new user set.
        newUsers->insert(userFunc);
      }
    }
  }
}

void fillFuncUsersSet(std::set<Function *> &roots,
                      std::set<Function *> &userFuncs) {
  std::set<Function *> newUsers1, newUsers2;
  std::set<Function *> *pNewUsers = &newUsers1;
  std::set<Function *> *pRoots = &newUsers2;
  // First Get the direct users of the roots.
  fillDirectUsers(&roots, &userFuncs, pNewUsers);
  while (pNewUsers->size()) {
    // iteratively swap between the new users sets, and use the current
    // as the roots for the new direct users.
    std::swap(pNewUsers, pRoots);
    pNewUsers->clear();
    fillDirectUsers(pRoots, &userFuncs, pNewUsers);
  }
}

void fillInstructionUsers(Function *F,
                          SmallVectorImpl<Instruction *> &userInsts) {
  // Holds values to check.
  SmallVector<Value *, 8> workList(F->user_begin(), F->user_end());
  // Holds values that already been checked, in order to prevent
  // inifinite loops.
  std::set<Value *> visited;
  while (workList.size()) {
    Value *user = workList.back();
    workList.pop_back();
    if (visited.count(user))
      continue;
    visited.insert(user);

    // New value if it is an Instruction add to it to usedInsts Vec,
    // otherwise check all it's users.
    Instruction *I = dyn_cast<Instruction>(user);
    if (I) {
      userInsts.push_back(I);
    } else {
      workList.append(user->user_begin(), user->user_end());
    }
  }
}

void fillInternalFuncUsers(Module &m, const OpenclRuntime *rt,
                           std::set<Function *> &userFuncs) {
  std::set<Function *> internalFuncs;
  for (Module::iterator fit = m.begin(), fe = m.end(); fit != fe; ++fit)
    if (!fit->isDeclaration())
      internalFuncs.insert(fit);
  fillFuncUsersSet(internalFuncs, userFuncs);
}

void collectTIDCallInst(const char *name, IVecVec &tidCalls, Function *F) {
  const unsigned MAX_OCL_NUM_DIM = 3;
  IVec emptyVec;
  tidCalls.assign(MAX_OCL_NUM_DIM, emptyVec);
  SmallVector<CallInst *, 4> allDimTIDCalls;
  getAllCallInFunc(name, F, allDimTIDCalls);

  for (unsigned i = 0, e = allDimTIDCalls.size(); i < e; ++i) {
    CallInst *CI = allDimTIDCalls[i];
    ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
    assert(C && "tid arg must be constant");
    unsigned dim = C->getValue().getZExtValue();
    assert(dim < MAX_OCL_NUM_DIM && "tid not in range");
    tidCalls[dim].push_back(CI);
  }
}

//  void fillVariableWIFuncUsers(Module &m, const OpenclRuntime *rt,
//                                     std::set<Function *> &userFuncs);
// transforms code as follows:
// prehead:
//     br head
// head:
//     indVar = phi (0 preHead, latch incIndVar)
//          :
// latch: //(can be the same as head)
//          :
//     incIndVar = add indVar, 1
//     x = Icmp eq incIndVar ,loopSize
//     br x, exit, oldEntry
// exit:
//
//  all get_local_id \ get_global_id are replaced with indVar
loopRegion createLoop(BasicBlock *head, BasicBlock *latch, Value *begin,
                      Value *increment, Value *end, std::string &name,
                      LLVMContext &C) {
  Type *indTy = begin->getType();
  assert(indTy == increment->getType());
  assert(indTy == end->getType());
  assert(head->getParent() == latch->getParent());
  // Creating Blocks to wrap the code as described above.
  Function *F = head->getParent();
  BasicBlock *preHead = BasicBlock::Create(C, name + "pre_head", F, head);
  BasicBlock *exit = BasicBlock::Create(C, name + "exit", F);
  exit->moveAfter(latch);
  BranchInst::Create(head, preHead);

  // Insert induction variable phi in the head entry.
  PHINode *indVar =
      head->empty()
          ? PHINode::Create(indTy, 2, name + "ind_var", head)
          : PHINode::Create(indTy, 2, name + "ind_var", head->begin());

  // Increment induction variable.
  BinaryOperator *incIndVar = BinaryOperator::Create(
      Instruction::Add, indVar, increment, name + "inc_ind_var", latch);
  incIndVar->setHasNoSignedWrap();
  incIndVar->setHasNoUnsignedWrap();

  // Create compare and conditionally branch out from latch.
  Instruction *compare = new ICmpInst(*latch, CmpInst::ICMP_EQ, incIndVar, end,
                                      name + "cmp.to.max");
  BranchInst::Create(exit, head, compare, latch);

  // Upadte induction variable phi with the incoming values.
  indVar->addIncoming(begin, preHead);
  indVar->addIncoming(incIndVar, latch);
  return loopRegion(preHead, exit);
}
} // LoopUtils
} // namespace intel
