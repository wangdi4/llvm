/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "resolve-block-call"
#include "ResolveBlockToStaticCall.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/SmallSet.h"

#include <assert.h>
using namespace llvm;

extern "C" {
  Pass *createResolveBlockToStaticCallPass() {
    return new intel::ResolveBlockToStaticCall();
  }
}

namespace intel {
  char ResolveBlockToStaticCall::ID = 0;
  OCL_INITIALIZE_PASS(ResolveBlockToStaticCall, "resolve-block-call",
    "Resolves Block calls to static calls",
    false,
    false
    )
}

// index of block_invoke function address in
// block literal structure of form
// { i8**, i32, i32, i8*, %struct.__block_descriptor* }
static const int BLOCK_LITERAL_INVOKE_ADDR_INDEX = 3;

// check and get if it is function pointer call
// @return CallInst with call
//         0 - Not function pointer call
static CallInst * getFuncPtrCall(Value *I){
  CallInst *CI = dyn_cast<CallInst>(I);
  if (!CI)
    return 0;
  if(CI->getCalledFunction())
    return 0;
  return CI;
}

// check GEP is like:
// getelementptr inbounds %struct.__block_literal_generic* %block.literal, i32 0, i32 3
static bool IsGEPBlockInvokeAccess(const GetElementPtrInst *GEP){
  if(GEP->getNumIndices() != 2)
    return false;
  // check index1 is i32 0
  ConstantInt * idx1 = dyn_cast<ConstantInt>(GEP->getOperand(1));
  if(!idx1 || (idx1->getZExtValue() != 0))
    return false;
  // check index2 is i32 3
  ConstantInt * idx2 = dyn_cast<ConstantInt>(GEP->getOperand(2));
  if(!idx2 || (idx2->getZExtValue() != (uint64_t)BLOCK_LITERAL_INVOKE_ADDR_INDEX))
    return false;
  return true;
}

// finds instruction with special opcode in use list of instruction inst
// makes sure instruction is single
// @param inst - instruction
// @param opcode - opcode of instruction to search
// @return 0 - if none or more than one usage is detected
//         Value of instruction found
static Value *findSingleUsedInst(Instruction* inst, const unsigned opcode) {
  Value * res = 0;
  int32_t cnt = 0;
  for(Value::user_iterator user = inst->user_begin(), E = inst->user_end();
    user != E; ++user) {
      Instruction * inst = cast<Instruction>(*user);
      if(inst->getOpcode() == opcode){
        res = inst;
        if(++cnt > 1)
          // if more than one usage detected
          return 0;
      }
  }
  return res;
}

// find GEP and make sure it is single
static GetElementPtrInst *findSingleGEPBlockInvokeAccess(Instruction* inst) {
  GetElementPtrInst * res = 0;
  int32_t cnt = 0;

  for(Value::user_iterator user = inst->user_begin(), E = inst->user_end();
    user != E; ++user) {
      GetElementPtrInst * GEP = dyn_cast<GetElementPtrInst>(*user);
      if(GEP && IsGEPBlockInvokeAccess(GEP)) {
        res = GEP;
        if(++cnt > 1)
          // if more than one usage detected
          return 0;
      }
  }
  return res;
}

namespace intel {
  bool ResolveBlockToStaticCall::runOnFunction(Function &F) {
    SmallVector<CallInst *, 16> CallInstList;

    // find ptr callinst
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
      if(CallInst * CI = getFuncPtrCall(&*I))
        CallInstList.push_back(CI);
    }

    // process ptr callinst
    bool changed = false;
    for(SmallVector<CallInst *, 16>::iterator I = CallInstList.begin(),
      E = CallInstList.end(); I != E; ++I)
      changed = runOnCallInst(*I) || changed;
    return changed;
  }

  bool ResolveBlockToStaticCall::runOnCallInst(CallInst *CI) {
    Value *I = CI;
    Function *resolvedFunc = NULL;

    // %call1 = call i32 %8(i8* %6, i32 7)
    I = CI->getCalledValue();
    assert(I->getType()->isPointerTy() &&
        "Not pointer type of CallInst argument");

    SmallSet<Value*,8> value_visited;
    // loop over instructions in dependency chain
    while(I)
    {
      DEBUG(dbgs() << "Processing: " << *I << "\n");

      if(!value_visited.insert(I).second){
        assert(0 && "Should not be here."
          "Visited the same value more than once");
        break;
      }

      // %8 = bitcast i8* %7 to i32 (i8*, i32)*
      // %block.literal = bitcast i32 (i32)* %4 to %struct.__block_literal_generic*
      if(BitCastInst *BCI = dyn_cast<BitCastInst>(I)) {
        // look thru bitcast
        I = BCI->getOperand(0);
        continue;
      }
      // %7 = load i8** %5
      // %4 = load i32 (i32)** %kernelBlock, align 8
      else if(LoadInst *LI = dyn_cast<LoadInst>(I)) {
        I = LI->getPointerOperand();
        continue;
      }
      // %5 = getelementptr inbounds %struct.__block_literal_generic* %block.literal, i32 0, i32 3
      else if(GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I)) {
        if(!IsGEPBlockInvokeAccess(GEP))
          break;
        // get %block.literal
        I = GEP->getPointerOperand();
        continue;
      }
      // @globalBlock = addrspace(2) constant i32 (i32)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to i32 (i32)*), align 8
      else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(I)) {
        if(!GV->isConstant() || !GV->hasInitializer())
          // global variable with block literal is NOT constant or does not have initializer
          // in this case it cannot be guaranteed
          // that function pointer call is calling the function defined in
          // Global Variable's initializer
          break;
        I = GV->getInitializer();
        continue;
      }
      else if(AllocaInst *AI = dyn_cast<AllocaInst>(I)) {
        Type * type = AI->getAllocatedType();
        // %kernelBlock = alloca i32 (i32)*, align 8
        if (type->isPointerTy()) {
          Type * functype = cast<PointerType>(type)->getElementType();
          if(!functype->isFunctionTy())
            // expected function pointer to be "alloc"-ed
            break;
          // find single store instruction which writes to the "alloc"-ed variable
          // store i32 (i32)* %1, i32 (i32)** %kernelBlock, align 8
          I = findSingleUsedInst(AI, Instruction::Store);
          continue;
        }
        // %block = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>, align 8
        else if (type->isStructTy()) {
          // %block.invoke = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i32 }>* %block, i32 0, i32 3
          // for "alloc"-ed block literal structure,
          // find single GEP to field with invoke address
          GetElementPtrInst * GEP = findSingleGEPBlockInvokeAccess(AI);
          if(!GEP)
            // GEP was not found or
            // there are multiple GEPs.
            // In case of multiple GEPs we assume there might be multiple stores to invoke field
            // of block_literal structure
            // TODO. Check if we really need GEP to be single
            break;
          // store i8* bitcast (i32 (i8*, i32)* @__kernel_scope_block_invoke to i8*), i8** %block.invoke
          I = findSingleUsedInst(GEP, Instruction::Store);
          continue;
        }
        else
          // "alloca"-ed unsupported type
          break;
      }
      // store i32 (i32)* %1, i32 (i32)** %kernelBlock, align 8
      // store i8*
      //    bitcast (i32 (i8*, i32)* @__kernel_scope_block_invoke to i8*), i8** %block.invoke
      else if(StoreInst *SI = dyn_cast<StoreInst>(I)) {
        I = SI->getValueOperand();
        continue;
      }
      // constant expression in
      // store i8*
      //    bitcast (i32 (i8*, i32)* @__kernel_scope_block_invoke to i8*), i8** %block.invoke
      else if(ConstantExpr *CE = dyn_cast<ConstantExpr>(I)) {
        if (!(CE->getOpcode() == Instruction::BitCast))
          // expected bitcast in constant expression
          // observations from clang's generated code
          break;
        I = CE->getOperand(0);
        continue;
      }
      // define internal i32 @__kernel_scope_block_invoke(i8* %.block_descriptor, i32 %num) nounwind
      else if(Function *F = dyn_cast<Function>(I)) {
        // YES !!!
        // Static function is found
        resolvedFunc = F;
        break;
      }
      // internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (i32 (i8*, i32)* @globalBlock_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8
      else if(ConstantStruct *CS = dyn_cast<ConstantStruct>(I)) {
        I = CS->getOperand(BLOCK_LITERAL_INVOKE_ADDR_INDEX);
        continue;
      }
      else
        // unsupported instruction
        break;
    } // while(true)

    if(!resolvedFunc) {
      DEBUG(dbgs() << "Static function not found\n" <<
        "CallInst " << *CI << " in " <<
        CI->getParent()->getParent()->getName()   << " was not resolved\n");
      return false;
    }

    DEBUG(dbgs() << "Resolved to static function\n" <<
      "CallInst " << *CI << " to function " << resolvedFunc->getName() << "\n");

    // Create new call instruction with static call to resolved function
    CallSite CS(CI);
    SmallVector<Value*, 4> params(CS.arg_begin(), CS.arg_end());
    CallInst *pNewCall = CallInst::Create(resolvedFunc, ArrayRef<Value*>(params), CI->getName(), CI);
    // replace old call
    CI->replaceAllUsesWith(pNewCall);
    CI->eraseFromParent();

    return true;
  } // bool ResolveBlockToStaticCall::runOnCallInst(CallInst *CI)
} // namespace intel
