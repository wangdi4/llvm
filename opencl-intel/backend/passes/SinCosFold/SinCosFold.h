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

File Name:  SinCosFold.h

\*****************************************************************************/

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
