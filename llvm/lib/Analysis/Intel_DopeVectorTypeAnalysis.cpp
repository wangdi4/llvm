//===------- DopeVectorTypeAnalysis.cpp -----------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_DopeVectorTypeAnalysis.h"

using namespace llvm;

#define DEBUG_TYPE "dopevectortype"

AnalysisKey DopeVectorTypeAnalysis::Key;

DopeVectorTypeQueryInfo DopeVectorTypeAnalysis::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  return DopeVectorTypeQueryInfo(M);
}
