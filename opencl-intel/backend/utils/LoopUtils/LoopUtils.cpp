// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CompilationUtils.h"
#include "LoopUtils.h"
#include "OpenclRuntime.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Transforms/Utils/Cloning.h"

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
  FunctionCallee fConst = M->getOrInsertFunction(name, fType);
  func = dyn_cast<Function>(fConst.getCallee());
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
  unsigned pointerSizeInBits = M->getDataLayout().getPointerSizeInBits(0);
  assert((32 == pointerSizeInBits  || 64 == pointerSizeInBits) &&
         "Unsopported pointer size");
  return IntegerType::get(M->getContext(), pointerSizeInBits);
}

void getAllCallInFunc(StringRef funcName, Function *funcToSearch,
                      SmallVectorImpl<CallInst *> &calls) {
  Function *F = funcToSearch->getParent()->getFunction(funcName);
  if (!F)
    return;

  for (Value::user_iterator useIt = F->user_begin(), useE = F->user_end();
       useIt != useE; ++useIt) {
    CallInst *CI = cast<CallInst>(*useIt);
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
  SmallVector<Value *, 8> workList(F->users());
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
      internalFuncs.insert(&*fit);
  fillFuncUsersSet(internalFuncs, userFuncs);
}

void fillAtomicBuiltinUsers(Module &m, const OpenclRuntime *rt,
                            std::set<Function *> &userFuncs) {
  std::set<Function *> atomicFuncs;
  for (Module::iterator fit = m.begin(), fe = m.end(); fit != fe; ++fit) {
    std::string name = fit->getName().str();
    if (rt->isAtomicBuiltin(name))
      atomicFuncs.insert(&*fit);
  }
  fillFuncUsersSet(atomicFuncs, userFuncs);
}

void fillWorkItemPipeBuiltinUsers(Module &m, const OpenclRuntime *rt,
                                  std::set<Function *> &userFuncs) {
  std::set<Function *> pipeFuncs;
  for (Module::iterator fit = m.begin(), fe = m.end(); fit != fe; ++fit) {
    std::string name = fit->getName().str();
    using namespace Intel::OpenCL::DeviceBackend;
    if (CompilationUtils::isWorkItemPipeBuiltin(name))
      pipeFuncs.insert(&*fit);
  }
  fillFuncUsersSet(pipeFuncs, userFuncs);
}

void fillPrintfs(Module &m, const OpenclRuntime *rt,
                 std::set<Function *> &userFuncs) {
  std::set<Function *> printfFuncs;
  for (Function &f : m) {
    StringRef name = f.getName();
    if (name.equals("printf") || name.equals("opencl_printf"))
      printfFuncs.insert(&f);
  }
  fillFuncUsersSet(printfFuncs, userFuncs);
}

void collectTIDCallInst(const char *name, IVecVec &tidCalls, Function *F) {
  const unsigned MAX_OCL_NUM_DIM = 3;
  IVec emptyVec;
  tidCalls.assign(MAX_OCL_NUM_DIM, emptyVec);
  SmallVector<CallInst *, 4> allDimTIDCalls;
  getAllCallInFunc(name, F, allDimTIDCalls);

  for (unsigned i = 0, e = allDimTIDCalls.size(); i < e; ++i) {
    CallInst *CI = allDimTIDCalls[i];
    unsigned dim;
    if (CI->getNumArgOperands() == 0) {
      // No-operand version - does not really matter what dimension we will be
      // vectorizing over. Some examples:
      //   * <something>_linear_id,
      //   * get_sub_group_local_id.
      //
      // FIXME: Separate index for such kind of functions. Currently we're
      // vectorizing over 0-dimension only.
      dim = 0;
    } else {
      assert(CI->getNumArgOperands() == 1 && "Expected one-operand call!");
      ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
      assert(C && "tid arg must be constant");
      dim = C->getValue().getZExtValue();
      assert(dim < MAX_OCL_NUM_DIM && "tid not in range");
    }
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
          : PHINode::Create(indTy, 2, name + "ind_var", &*head->begin());

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

Value* generateRemainderMask(unsigned packetWidth, Value* loopLen, BasicBlock* BB) {

  Module* m = BB->getModule();

  auto IndTy = getIndTy(m);
  auto Int32Ty = Type::getInt32Ty(m->getContext());
  auto MaskTy = FixedVectorType::get(Int32Ty, packetWidth);

  ConstantInt* constZero = ConstantInt::get(Int32Ty, 0);

  // Generate sequential vector 0 ... packetWidth-1.
  SmallVector<Constant*, 16> indVec;
  Constant *indVar = nullptr;
  for (unsigned ind = 0; ind < packetWidth; ++ind) {
    indVar = ConstantInt::get(IndTy, ind);
    indVec.push_back(indVar);
  }
  Constant* sequenceVec = ConstantVector::get(indVec);

  // Generate splat vector loopLen ... loopLen.
  Value* loopLenVec = UndefValue::get(FixedVectorType::get(IndTy, packetWidth));
  Instruction* lenInsertVec = InsertElementInst::Create(loopLenVec, loopLen, constZero, "", BB);

  Constant *shuffleMask =
      ConstantVector::getSplat(ElementCount::getFixed(packetWidth), constZero);
  Instruction* lenSplatVec = new ShuffleVectorInst(lenInsertVec, loopLenVec, shuffleMask, "", BB);

  // Generate mask.
  Instruction* vecCmp = new ICmpInst(*BB, CmpInst::ICMP_ULT, sequenceVec, lenSplatVec, "mask.bool");
  Value* mask = new SExtInst(vecCmp, MaskTy, "mask.i32", BB);

  return mask;
}

void inlineMaskToScalar(Function* scalarKernel, Function* maskedKernel) {

  auto pModule = scalarKernel->getParent();
  assert(pModule == maskedKernel->getParent() &&
         "scalar and masked kernel are not in the same module!");
  LLVMContext &context = pModule->getContext();

  // Prepare args for masked kernel.
  SmallVector<Value*, 4> args;
  for (auto arg = scalarKernel->arg_begin(), arg_end = scalarKernel->arg_end();
       arg != arg_end; ++arg)
    args.push_back(arg);

  // Mask argument is handled in generateRemainderMask.
  auto dummyMaskArg = UndefValue::get((maskedKernel->arg_end()-1)->getType());
  args.push_back(dummyMaskArg);

  auto *entry = BasicBlock::Create(context, "", scalarKernel,
                                   &scalarKernel->getEntryBlock());
  // Call the masked kernel.
  auto *call = CallInst::Create(maskedKernel, args, "", entry);
  // Set debug scope which should be in scalarKernel's DISubprogram.
  if (DISubprogram *sp = scalarKernel->getSubprogram()) {
    call->setDebugLoc(DILocation::get(context, 0, 0, sp));
  }

  // Let DCE delete the scalar kernel body.
  ReturnInst::Create(context, entry);

  // Inline the masked kernel to scalar kernel.
  InlineFunctionInfo inlineInfo;
  CallBase* callBase = dyn_cast<CallBase>(call);
  assert(callBase && "unexpected call");
  InlineFunction(*callBase, inlineInfo);

  // Erase the masked kernel.
  maskedKernel->eraseFromParent();
}
} // LoopUtils
} // namespace intel
