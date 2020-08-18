//==-- HIRLoopBlocking.h - HIR Loop Blocking Pass ---------- --*- C++ -*---===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LOOPBLOCKINGIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LOOPBLOCKINGIMPL_H

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
class Function;

namespace loopopt {

namespace blocking {

// Following blocksize worked best in most of pow-2 and non-pow-2
// square matrix multiplications. Compile options used are
// marked in the beginning part of this file.
const unsigned DefaultBlockSize = 64;

enum DiagMsg {
  NON_LINEAR_REFS,
  NO_MISSING_IVS_OR_SMALL_STRIDES,
  NO_PERFECTNEST,
  INNERMOST_LOOP_ONLY,
  INNERMOST_LOOP_NO_DO_LOOP,
  NO_PROFITABLE_BLOCKING_FOUND,
  MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH,
  MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH_2,
  NO_STRIPMINE_APPL,
  NO_INTERCHANGE,
  NO_KANDR,
  NO_KANDR_DEPTH_TEST_1,
  NO_KANDR_DEPTH_TEST_2,
  MULTIVERSIONED_FALLBACK_LOOP,
  NO_STENCIL_LOOP,
  NO_STENCIL_LOOP_BODY,
  NO_STENCIL_MEM_REFS,
  SUCCESS_NON_SIV,
  SUCCESS_NON_SIV_OR_NON_ADVANCED,
  SUCCESS_BASIC_SIV,
  SUCCESS_STENCIL,

  NUM_DIAGS,
};

inline std::array<std::string, NUM_DIAGS> createDiagMap() {
  std::array<std::string, NUM_DIAGS> Map;
  Map[NON_LINEAR_REFS] = "nonlinear memref in the innermost loop.";
  Map[NO_MISSING_IVS_OR_SMALL_STRIDES] =
      "no refs with missing ivs or with small strides.";
  Map[NO_PERFECTNEST] = "NearPerfect loop cannot become a perfect loop.";
  Map[INNERMOST_LOOP_ONLY] = "Innermost-loop-only in the loopnest.";
  Map[INNERMOST_LOOP_NO_DO_LOOP] = "Innermost is not Do-loop.";
  Map[NO_PROFITABLE_BLOCKING_FOUND] = "No profitable blocking found.";
  Map[MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH] = "max dims <= loop nest depth.";
  Map[MAX_DIMS_LESS_THAN_LOOP_NEST_DEPTH_2] =
      "max dims <= consecutive loop nest depth.";
  Map[NO_STRIPMINE_APPL] = "Stripmining is not possible.";
  Map[NO_INTERCHANGE] = "Interchange is not possible.";
  Map[NO_KANDR] = "KandR did not work.";
  Map[NO_KANDR_DEPTH_TEST_1] = "Failed KandR depth test 1.";
  Map[NO_KANDR_DEPTH_TEST_2] = "Failed KandR depth test 2.";
  Map[MULTIVERSIONED_FALLBACK_LOOP] = "The loop is mv fallback loop.";
  Map[NO_STENCIL_LOOP] = "The loop body is not a stencil function.";
  Map[NO_STENCIL_LOOP_BODY] = "Operations for stencil is missing.";
  Map[NO_STENCIL_MEM_REFS] = "Memrefs are not of stencil pattern.";
  Map[SUCCESS_NON_SIV] = "Non-Siv loop.";
  Map[SUCCESS_NON_SIV_OR_NON_ADVANCED] = "Non-Siv loop or Non-advanced.";
  Map[SUCCESS_BASIC_SIV] = "Basic algorithm.";
  Map[SUCCESS_STENCIL] = "Stencil pattern.";

  return Map;
}

// Mapping from a loop to its kind. Used for/during actual transformantion.
// For a loop to be strimined, (will become unit-strided), holds stripmine size.
// For a by-strip loop, holds BY_STRIP_LOOP_VAL, which is zero.
typedef std::map<const HLLoop *, unsigned> LoopMapTy;
typedef std::pair<int, RegDDRef *> LevelAndFactorPairTy;
typedef SmallVector<LevelAndFactorPairTy, MaxLoopNestLevel> PragmaPairVecTy;
typedef std::map<const HLLoop *, PragmaPairVecTy> LoopPragmaMapTy;

enum LoopTypeVal { BY_STRIP_LOOP_VAL = 0, STRIPMINE_CAND_VAL = 0 };

struct HIRLoopBlocking {

  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRSafeReductionAnalysis &SRA;
  HIRLoopStatistics &HLS;
  TargetTransformInfo &TTI;

  StringRef FuncName;
  LoopPragmaMapTy LoopToPragma;
  bool HasPragma;

  HIRLoopBlocking(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                  HIRSafeReductionAnalysis &SRA, HIRLoopStatistics &HLS,
                  TargetTransformInfo &TTI)
      : HIRF(HIRF), HDDA(HDDA), SRA(SRA), HLS(HLS), TTI(TTI) {}

  bool run(bool Pragma);

  void doTransformation(HLLoop *InnermostLoop, HLLoop *OutermostLoop,
                        LoopMapTy &LoopToBS);
};

//
} // namespace blocking
} // namespace loopopt
} // namespace llvm

#endif
