//===---------------- HIRNormalizeCasts.cpp -------------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
//
// This pass will replace the blobs that are integer casts in a loop to match
// with the same casts used between the loop bounds and the memory references.
// For example:
//
//   BEGIN REGION { }
//         + DO i1 = 0, zext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//                   <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//         |   %3 = (%arr)[sext.i32.i64(%cols) * i1];
//         |   %4 = (%arr)[zext.i32.i64(%cols) * i1];
//         |   %mul.i6926 = %3  *  %4;
//         |   %result.031.i = %mul.i6926  +  %result.031.i;
//         + END LOOP
//   END REGION
//
// In the example above we have a loop nest where the loop bound is a zero
// extension integer cast to the variable %cols, but the memory reference is a
// signed extension cast to the same variable, and the loop's legal max trip
// count (LEGAL_MAX_TC) is the max signed integer in src type (i32). This pass
// will replace the blob in the loop's upper bound from zero extent to signed
// extent, as well the memory references when it is legal to do so:
//
//   BEGIN REGION { }
//         + DO i1 = 0, sext.i32.i64(%cols) + -1, 1   <DO_LOOP>
//                   <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
//         |   %3 = (%arr)[sext.i32.i64(%cols) * i1];
//         |   %4 = (%arr)[sext.i32.i64(%cols) * i1];
//         |   %mul.i6926 = %3  *  %4;
//         |   %result.031.i = %mul.i6926  +  %result.031.i;
//         + END LOOP
//   END REGION
//
// In the example above, the integer casting in loop's upper bound was
// converted from zext to sext, as well instruction %4.
//
// This fix will help with other analyses that checks if the loop bound is
// used in the memory references.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRNormalizeCasts.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-normalize-casts"
#define OPT_DESC "HIR normalize casts"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableNormalizeCasts("disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
                          cl::desc("Disable HIR normalize casts"));

class NormalizeCasts {
public:
  NormalizeCasts(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

  // TODO: Field needs to be private
  HIRFramework &HIRF;
};

// TODO: Implement pass
bool NormalizeCasts::run() {
  if (DisableNormalizeCasts)
    return false;

  return false;
}

PreservedAnalyses HIRNormalizeCasts::runImpl(Function &F,
                                             FunctionAnalysisManager &AM,
                                             HIRFramework &HIRF) {

  NormalizeCasts(HIRF).run();

  return PreservedAnalyses::all();
}