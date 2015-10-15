/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

  INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
  LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
  ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
  PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
  DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
  PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
  including liability for infringement of any proprietary rights, relating to
  use of the code. No license, express or implied, by estoppels or otherwise,
  to any intellectual property rights is granted herein.

File Name:  SinCosFold.cpp

\*****************************************************************************/
#define DEBUG_TYPE "SinCosFoldPass"

#include "SinCosFold.h"
#include "ParameterType.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"

#include <map>

namespace intel {

  char SinCosFold::ID=0;

  static const StringRef sin("sin");
  static const StringRef cos("cos");

  /// Register pass to for opt
  OCL_INITIALIZE_PASS_BEGIN(SinCosFold, "cos-sin-pass", "Folds cos(x) and sin(x) functions with a single sincos function", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(SinCosFold, "cos-sin-pass", "Folds cos(x) and sin(x) functions with a single sincos function", false, false)

  SinCosFold::SinCosFold() : ModulePass(ID), OCLSTAT_INIT(Additinal_sin_cos_not_replaced,
    "stores if there are extra sin and cos that should have been replaced, but were not",
    m_kernelStats) {
      initializeSinCosFoldPass(*llvm::PassRegistry::getPassRegistry());
    }

  void SinCosFold::replaceSingleInst(CallInst *it, Value * val) {
    it->replaceAllUsesWith(val);
    it->eraseFromParent();
  }

  bool SinCosFold::replaceAllInst(InstructionData * iData,llvm::Module &M) {
    assert(iData != NULL && "SinCosFold::replaceAllInst iData == null");
    if (!iData->cosInst || !iData->sinInst) {
      return false;
    }

    Value * val = iData->sinInst->getArgOperand(0);
    Instruction* pEntryPoint = (iData->cosInst)->getParent()->getParent()->getEntryBlock().begin();
    AllocaInst * cos = new AllocaInst(val->getType(), 0, "cosPtr",pEntryPoint);

    reflection::FunctionDescriptor fd;
    fd.name = "sincos";
    reflection::RefParamType PF(new reflection::PointerType(iData->argType)); //cos return value
    fd.parameters.push_back(iData->argType);
    fd.parameters.push_back(PF);
    std::string MangledFName = mangle(fd);

    Function * sinCosF = m_rtServices->findInRuntimeModule(MangledFName);
    assert(sinCosF && "sincos function should exist!");

    // Find (or create) declaration for newly called function
    Constant * sinCosFunc = M.getOrInsertFunction(sinCosF->getName(), sinCosF->getFunctionType(), sinCosF->getAttributes());
    assert(sinCosFunc && "Failed generating function in current module");
    Function * f = dyn_cast<Function>(sinCosFunc);
    assert(f && "Function type mismatch, caused a constant expression cast!");

    SmallVector<Value *, 16> args;
    args.push_back(val);
    args.push_back(cos); //pointer to cosval

    CallInst * sincos = CallInst::Create(f,args,"sinPtr", iData->firstCallInst);
    LoadInst * cosLoad = new LoadInst(cos,"cosVal", iData->firstCallInst);

    sincos->setDebugLoc(iData->firstCallInst->getDebugLoc());
    cosLoad->setDebugLoc(iData->cosInst->getDebugLoc());

    Value * sinVal = sincos;
    Value * cosVal = cosLoad;

    replaceSingleInst(iData->sinInst,sinVal);
    replaceSingleInst(iData->cosInst,cosVal);
    return true;
  }


  bool SinCosFold::runOnModule(llvm::Module &M) {
    bool changedModule = false;
    m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
    assert(m_rtServices && "m_rtServices should exist!");
    for (Module::iterator fn = M.begin(), fne = M.end(); fn != fne; ++fn) {
      for (Function::iterator bb = fn->begin(), bbe = fn->end(); bb != bbe; ++bb) {
        DataMap paramToInstructions;
        for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
          CallInst* inst = dyn_cast<CallInst>(&*i);
          if (!inst) continue;

          Function * f = inst->getCalledFunction();
          if (!f) continue;

          reflection::FunctionDescriptor fd = demangle(f->getName().data());
          StringRef fName(fd.name);

          bool isSin = fName.compare(sin) == 0;
          bool isCos = fName.compare(cos) == 0;

          if (!isSin && !isCos) continue;

          //We are calling either sin or cos.

          //add to map
          Value * val = inst->getArgOperand(0);
          DataMap::iterator iter = paramToInstructions.find(val);

          //value is not in the map
          if (iter == paramToInstructions.end()) {
            InstructionData * iData = new InstructionData(fd.parameters[0]);
            iData->addInstruction(inst,isSin);
            paramToInstructions[val] = iData;
          }
          else {
            //value is in the map
            InstructionData * iData = paramToInstructions[val];
            if (!iData->addInstruction(inst,isSin)) {
              Additinal_sin_cos_not_replaced+=1;
            }
          }
        }
        //replace all sin cos instructions
        for (DataMap::iterator i = paramToInstructions.begin(); i != paramToInstructions.end(); ++i) {
          bool replaced = replaceAllInst((*i).second,M);
          changedModule = changedModule || replaced;
          //release allocated data
          delete((*i).second);
        }
        //clear map
        paramToInstructions.clear();
      }
      intel::Statistic::pushFunctionStats(m_kernelStats, (*fn), DEBUG_TYPE);
    }
    return changedModule;
  }

  bool SinCosFold::InstructionData::addInstruction(CallInst* inst, bool isSin) {
    if (!firstCallInst) {
      firstCallInst = inst;
    }
    if (isSin) {
      if (sinInst!=NULL) {
        return false;
      }
      else{
        sinInst = inst;
      }
    }
    else {
      if (cosInst!=NULL) {
        return false;
      }
      else {
        cosInst = inst;
      }
    }
    return true;
  }
}

extern "C"{
  ModulePass* createSinCosFoldPass() {
    return new intel::SinCosFold();
  }
}
