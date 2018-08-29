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

#ifndef __SIN_COS_FOLD__H__
#define __SIN_COS_FOLD__H__

#include "BuiltinLibInfo.h"
#include "NameMangleAPI.h"
#include "OclTune.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstrTypes.h"

namespace intel {

/// @brief  SinCosFold class replaces a sin and cos functions of the same angle by a single sincos function
class SinCosFold : public llvm::ModulePass
{
public:
  static char ID; // Pass identification, replacement for typeid

  // Constructor
  SinCosFold();

  // replace cos and sin with a single sincos function
  bool runOnModule(llvm::Module &M);

   /// @brief Inform about usage/mofication/dependency of this pass
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BuiltinLibInfo>();
  }
private:
  /// @brief stores information about sin and cos instructions
  class InstructionData
  {
    public:
      CallInst* cosInst;
      CallInst* sinInst;
      CallInst* firstCallInst;
      reflection::RefParamType argType;

      InstructionData(reflection::RefParamType& type):cosInst(nullptr),sinInst(nullptr),firstCallInst(nullptr){
        argType = type;
      }

      bool addInstruction(CallInst* inst,bool isSin);

    private:
      InstructionData();
  };

  typedef std::map<llvm::Value *, InstructionData *> DataMap;
  void replaceSingleInst(CallInst *it, Value * val);
  bool replaceAllInst(InstructionData * iData,llvm::Module &M);

  // Pointer to runtime service object
  const RuntimeServices * m_rtServices;

  // Statistics:
  Statistic::ActiveStatsT m_kernelStats;
  Statistic Additinal_sin_cos_not_replaced;
};
}
#endif
