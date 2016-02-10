//===-- VPODriver.h ---------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the base class for HIR transformation passes.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPODRIVER_H
#define LLVM_TRANSFORMS_VPO_VPODRIVER_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"

namespace llvm {

class Function;
class LoopInfo;

namespace vpo {

//
// VPODriverBase class can be moved inside anonymous namespace
// inside VPODriver.cpp, as long as the derived classes remain
// in that single file. For now, keep this in the header file.
//
class VPODriverBase : public FunctionPass {

  LoopInfo *LI;
  ScalarEvolution *SC;
  WRegionInfo *WR;

protected:
  /// Handle to AVR Generate Pass
  AVRGenerateBase *AV;

public:
  VPODriverBase(char &ID) : FunctionPass(ID){};
  bool runOnFunction(Function &F) override;
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPODRIVER_H
