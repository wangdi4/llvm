/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __APPLE_WI_DEP_PRE_PACKETIZATION_PASS_H__
#define __APPLE_WI_DEP_PRE_PACKETIZATION_PASS_H__

#include "Logger.h"
#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "WIAnalysis.h"

#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;


namespace intel {

///@brief 
/// This pass takes an openCL scalar kernel and prepare it for vectorization
/// it replaces scalar function calls with fake scalar function that are more 
/// suitable for vectorization and root it's arguments and return value
/// this Pass assumes it is running under apple environment with runtime services
/// is of Type AppleOCLRuntime
class AppleWIDepPrePacketizationPass : public FunctionPass {

public:

  static char ID;
  /// @brief C'tor
  AppleWIDepPrePacketizationPass();
  
  /// @brief D'tor
  ~AppleWIDepPrePacketizationPass();
  
  /// @brief Provides name of pass
  virtual StringRef getPassName() const {
    return "Apple WI dependaent Pre-Packetization Pass";
  }
  
  virtual bool runOnFunction(Function &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<WIAnalysis>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addRequired<BuiltinLibInfo>();
  }


private:

  /// @brief handle cases of write image by extracting the coord elements
  /// @param CI - scalar write image call instruction tio handle
  /// @param funcName - name of builtin
  void handleWriteImage(CallInst *CI, std::string &resolvedName);

  Value *getWriteImgCoordsVector(Value *xCoord, Value *yCoord, CallInst *CI);

  ///@brief - appleOCLRuntime interface for getting builtins attributes.
  const OpenclRuntime *m_appleRuntimeServices;

  ///@brief  holds the module of the processed function
  Module *m_curModule;

  ///@brief hold the instruction marked for removal
  std::vector<Instruction *> m_removedInsts;
  


};// ApplePreVectorizationPass

} // namespace intel


#endif //__APPLE_WI_DEP_PRE_PACKETIZATION_PASS_H__
