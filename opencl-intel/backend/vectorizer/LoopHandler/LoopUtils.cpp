/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "LoopUtils.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Constants.h"

namespace intel {


Function *LoopUtils::getWIFunc(Module *M, StringRef name, Type *retTy) {
// First look in the module if it already exists
  Function *func = M->getFunction(name);
  if (func) return func;
  // create declaration.
  std::vector<Type *> argTypes(1, Type::getInt32Ty(retTy->getContext()));
  FunctionType *fType =FunctionType::get(retTy, argTypes, false);
  Constant * fConst = M->getOrInsertFunction(name, fType);
  func = dyn_cast<Function>(fConst);
  assert(func && "null WI function");
  return func;
}

CallInst *LoopUtils::getWICall(Module *M, StringRef funcName, Type *retTy,
                    unsigned dim, BasicBlock *BB, const Twine &callName) {
  Function *WIFunc = getWIFunc(M, funcName, retTy);
  Constant *dimConst = ConstantInt::get(Type::getInt32Ty(retTy->getContext()), dim);
  CallInst *WICall = CallInst::Create(WIFunc, dimConst, callName, BB);
  return WICall;
}

Type *LoopUtils::getIndTy(Module *M) {
  switch (M->getPointerSize()) {  
    case Module::Pointer32: 
      return IntegerType::get(M->getContext(), 32);    
    case Module::Pointer64: 
      return IntegerType::get(M->getContext(), 64);    
    break;
    default:
      assert(0 && "pointer size != 32 , 64");
      return NULL;
  }
}

void LoopUtils::getAllCallInFunc(StringRef funcName, Function *funcToSearch,
                      SmallVectorImpl<CallInst *>& calls) {
  Function *F = funcToSearch->getParent()->getFunction(funcName);
  if (!F) return;

  for (Value::use_iterator useIt = F->use_begin(), useE = F->use_end();
       useIt != useE; ++useIt) {
    CallInst *CI = dyn_cast<CallInst>(*useIt);
    Function *parentFunc = CI->getParent()->getParent();
    if (parentFunc == funcToSearch) calls.push_back(CI);
  }
}

bool LoopUtils::inSubLoop(Loop *L, BasicBlock *BB) {
  assert(L->contains(BB) && "Only valid if BB is IN the loop");
  for (Loop::iterator I = L->begin(), E = L->end(); I != E; ++I) {
    if ((*I)->contains(BB)) return true;  // In a subloop.
  }
  return false;
}

void LoopUtils::GetOCLKernel(Module &M, SmallVectorImpl<Function *> &kernels) {
  NamedMDNode *pOpenCLMetadata = M.getNamedMetadata("opencl.kernels");
  if (!pOpenCLMetadata) return;
  unsigned numOfKernels = pOpenCLMetadata->getNumOperands();
  // List all kernels in module
  for (unsigned i = 0, e = numOfKernels; i != e; ++i) {
    MDNode *elt = pOpenCLMetadata->getOperand(i);
    Value *field0 = elt->getOperand(0)->stripPointerCasts();
    Function *F = dyn_cast<Function>(field0);
    kernels.push_back(F);
  }
}


void LoopUtils::fillDirectUsers(std::set<Function *> *funcs,
             std::set<Function *> *userFuncs, std::set<Function *> *newUsers) {
  // Go through all of the funcs.
  SmallVector<Instruction *, 8> userInst;
  for (std::set<Function *>::iterator fit = funcs->begin(), fe = funcs->end();
       fit != fe ; ++fit) {
    Function *F = *fit;
    if (!F) continue;
    
    // Get the instruction users of the function, and insert their 
    // parent functions to the userFuncs set.
    userInst.clear();
    fillInstructionUsers(F, userInst);
    for (unsigned i = 0, e = userInst.size(); i < e; ++i ) {
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

void LoopUtils::fillFuncUsersSet(std::set<Function *> &roots,
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

void LoopUtils::fillInstructionUsers(Function *F,
                                  SmallVectorImpl<Instruction *>  &userInsts) {
    // Holds values to check.
    SmallVector<Value *, 8> workList(F->use_begin(), F->use_end());
    // Holds values that already been checked, in order to prevent 
    // inifinite loops.
    std::set<Value *> visited;
    while (workList.size()) {
      Value *user = workList.back();
      workList.pop_back();
      if (visited.count(user)) continue;
      visited.insert(user);

      // New value if it is an Instruction add to it to usedInsts Vec,
      // otherwise check all it's users.
      Instruction *I = dyn_cast<Instruction>(user);
      if (I) {
        userInsts.push_back(I);
      } else {
        workList.append(user->use_begin(), user->use_end());
      }
    }
  }
}//namespace intel
