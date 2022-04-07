// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#define DEBUG_TYPE "SinCosFoldPass"

#include "CompilationUtils.h"
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

using namespace llvm::NameMangleAPI;

namespace intel {

  char SinCosFold::ID=0;

  static const StringRef sin("sin");
  static const StringRef cos("cos");

  /// Register pass to for opt
  OCL_INITIALIZE_PASS_BEGIN(SinCosFold, "cos-sin-pass", "Folds cos(x) and sin(x) functions with a single sincos function", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(SinCosFold, "cos-sin-pass", "Folds cos(x) and sin(x) functions with a single sincos function", false, false)

  SinCosFold::SinCosFold()
      : ModulePass(ID),
        DPCPP_STAT_INIT(Additinal_sin_cos_not_replaced,
                        "stores if there are extra sin and cos that should "
                        "have been replaced, but were not",
                        m_kernelStats) {
    initializeSinCosFoldPass(*llvm::PassRegistry::getPassRegistry());
  }

  void SinCosFold::replaceSingleInst(CallInst *it, Value * val) {
    it->replaceAllUsesWith(val);
    it->eraseFromParent();
  }

  bool SinCosFold::replaceAllInst(InstructionData * iData,llvm::Module &M) {
    assert(iData != nullptr && "SinCosFold::replaceAllInst iData == null");
    if (!iData->cosInst || !iData->sinInst) {
      return false;
    }

    Value * val = iData->sinInst->getArgOperand(0);
    assert(val != nullptr && "First argument operand is null!");
    Instruction* pEntryPoint = &*(iData->cosInst)->getParent()->getParent()->getEntryBlock().begin();
    AllocaInst * cos = new AllocaInst(val->getType(), 0, "cosPtr",pEntryPoint);
    cos->setDebugLoc(iData->cosInst->getDebugLoc());

    reflection::FunctionDescriptor fd;
    fd.Name = "sincos";
    reflection::RefParamType PF(new reflection::PointerType(
        iData->argType, {reflection::ATTR_PRIVATE})); // cos return value
    fd.Parameters.push_back(iData->argType);
    fd.Parameters.push_back(PF);
    std::string MangledFName = mangle(fd);

    Function * sinCosF = m_rtServices->findInRuntimeModule(MangledFName);
    assert(sinCosF && "sincos function should exist!");

    // Find (or create) declaration for newly called function
    using namespace Intel::OpenCL::DeviceBackend;
    FunctionCallee sinCosFunc = CompilationUtils::importFunctionDecl(&M, sinCosF);
    assert(sinCosFunc && "Failed generating function in current module");
    Function * f = dyn_cast<Function>(sinCosFunc.getCallee());
    assert(f && "Function type mismatch, caused a constant expression cast!");

    SmallVector<Value *, 16> args;
    args.push_back(val);
    args.push_back(cos); //pointer to cosval

    CallInst * sincos = CallInst::Create(f,args,"sinPtr", iData->firstCallInst);
    LoadInst *cosLoad =
        new LoadInst(val->getType(), cos, "cosVal", iData->firstCallInst);

    // SinCosFold pass will change the behavior of original code, so we can
    // only guarantee that the debugger will have a place to stop.
    sincos->setDebugLoc(iData->sinInst->getDebugLoc());
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
    MapVector<llvm::Value *, InstructionData> paramToInstructions;
    for (Module::iterator fn = M.begin(), fne = M.end(); fn != fne; ++fn) {
      if (fn->hasOptNone())
        continue;
      for (Function::iterator bb = fn->begin(), bbe = fn->end(); bb != bbe; ++bb) {
        paramToInstructions.clear();
        for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
          CallInst* inst = dyn_cast<CallInst>(&*i);
          if (!inst) continue;

          Function * f = inst->getCalledFunction();
          if (!f) continue;

          reflection::FunctionDescriptor fd = demangle(f->getName().data());
          StringRef fName(fd.Name);

          bool isSin = fName.compare(sin) == 0;
          bool isCos = fName.compare(cos) == 0;

          if (!isSin && !isCos) continue;

          //We are calling either sin or cos.

          //add to map
          Value * val = inst->getArgOperand(0);
          auto iter = paramToInstructions.find(val);

          //value is not in the map
          if (iter == paramToInstructions.end()) {
            auto Res = paramToInstructions.insert({val, fd.Parameters[0]});
            Res.first->second.addInstruction(inst, isSin);
          }
          else {
            //value is in the map
            if (!iter->second.addInstruction(inst, isSin)) {
              Additinal_sin_cos_not_replaced+=1;
            }
          }
        }
        //replace all sin cos instructions
        for (auto &Item : paramToInstructions) {
          bool replaced = replaceAllInst(&Item.second, M);
          changedModule = changedModule || replaced;
        }
      }
      DPCPPStatistic::pushFunctionStats(m_kernelStats, (*fn), DEBUG_TYPE);
    }
    return changedModule;
  }

  bool SinCosFold::InstructionData::addInstruction(CallInst* inst, bool isSin) {
    if (!firstCallInst) {
      firstCallInst = inst;
    }
    if (isSin) {
      if (sinInst!=nullptr) {
        return false;
      }
      else{
        sinInst = inst;
      }
    }
    else {
      if (cosInst!=nullptr) {
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
