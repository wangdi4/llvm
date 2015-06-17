//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file declares the base class for HIR transformation passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPODRIVER_H
#define LLVM_TRANSFORMS_VPO_VPODRIVER_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"

namespace llvm {

class Function;
class LoopInfo;

namespace vpo {

class VPODriver : public FunctionPass {

  LoopInfo *LI;
  ScalarEvolution *SC;
  WRegionInfo *WR;

public:
  static char ID; // Pass identification, replacement for typeid

  VPODriver();

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<WRegionInfo>();
    AU.addRequired<ScalarEvolution>();
  }
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPODRIVER_H
