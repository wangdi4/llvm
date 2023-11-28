//===--------------- HIRPreprocessNonUnitStrideAccess.cpp -----------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
// This pass implements preprocessing non-unit stride access in order to help
// loop fusion, scalar-replacement, vectorization, among others. For example,
// assume we have the following sibling loops:
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %0 = (%p)[%t * %y * i1];
//     %1 = %temp0  +  %0;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %2 = (%p)[%t * %y * i1];
//     %3 = %temp1  +  %2;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %4 = (%p)[%t * %y * i1] - %temp2;
//     (%p)[%t * %y * i1] = %4;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %5 = (%p)[%t * %y * i1];
//     %6 = %temp3  +  %5;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     (%p)[%t * %y * i1] = %temp2 * i1;
//   END LOOP
//
// The non-unit stride load may be non-profitable for vectorizer. To help its
// cost model, we store the load in a temporary array and it can be accessed
// in a unit-stride manner. The copies will be done in a separate preprocessing
// loop. The result from the case shown before will be the following:
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     (%temp.arr)[%i1] = (%p)[%t * %y * i1];
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %0 = %temp.arr[i1];
//     %1 = %temp0  +  %0;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %2 = (%temp.arr)[i1];
//     %3 = %temp1  +  %2;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %4 = (%temp.arr)[%i1] - %temp2;
//     (%temp.arr)[%i1] = %4;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     %5 = (%temp.arr)[%i1];
//     %6 = %temp3  +  %5;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     (%temp.arr)[%i1] = %temp2 * i1;
//   END LOOP
//
//   DO i1 = 0, 36, 1   <DO_LOOP>
//     (%p)[%t * %y * i1] = (%temp.arr)[%i1];
//   END LOOP
//
// A more complicated example would be:
//
// DO i1 = 0, -1 * %24 + smax((1 + %21), (1 + %24)) + -1, 1   <DO_LOOP>
//   ...
//
//   DO i2 = 0, 36, 1   <DO_LOOP>
//     %t = (@off)[i2]
//     %61 = (%8)[(%29 * %27) * i2 + %t];
//     ...
//   END LOOP
//
//   ...
//
// END LOOP
//
// The previous test case shows that the access to %8 is non-linear. This may
// produce cache misses during the load. This is another case where
// preprocessing the access will help.
//
// DO i1 = 0, -1 * %24 + smax((1 + %21), (1 + %24)) + -1, 1   <DO_LOOP>
//   ...
//
//   DO i2 = 0, 36, 1   <DO_LOOP>
//     %t = (@off)[i2];
//     %61 = (%8)[(%29 * %27) * i2 + %t];
//     (%temp.arr)[i2] = %61;
//   END LOOP
//
//   DO i2 = 0, 36, 1   <DO_LOOP>
//     %75 = (%temp.arr)[i2];
//     ...
//   END LOOP
//
//   ...
//
//   DO i2 = 0, 36, 1   <DO_LOOP>
//     %t = (@off)[i2];
//     %100 = (%temp.arr)[i2];
//     (%8)[(%29 * %27) * i2 + %t] = %100;
//   END LOOP
//
// END LOOP
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRPreprocessNonUnitStrideAccess.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-preprocess-nonunit-stride-access"
#define OPT_DESC "HIR preprocess non-unit stride access"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePreprocessNonUnitStrideAccess(
    "disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
    cl::desc("Disable HIR preprocess non-unit stride access"));

class PreprocessNonUnitStrideAccess {
public:
  PreprocessNonUnitStrideAccess(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

  // TODO: This field needs to be private
  HIRFramework &HIRF;
};

// TODO: Implement pass
bool PreprocessNonUnitStrideAccess::run() {
  if (DisablePreprocessNonUnitStrideAccess)
    return false;

  return false;
}

PreservedAnalyses HIRPreprocessNonUnitStrideAccess::runImpl(
    Function &F, FunctionAnalysisManager &AM, HIRFramework &HIRF) {

  ModifiedHIR = PreprocessNonUnitStrideAccess(HIRF).run();

  return PreservedAnalyses::all();
}