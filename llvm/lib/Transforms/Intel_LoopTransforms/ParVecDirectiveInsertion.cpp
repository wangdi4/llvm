//===- ParVecDirectiveInsertion.cpp - Implements VecDirectiveInsertion class
//-===//
//                               Also Implements ParDirectionInsertion class
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements ParVecDirectiveInsertion transformation,
// which is common to ParDirInsert/VecDirInsert passes.
//
// See also ParVecAnalysis.cpp for diagnostic related flags.
//
//===----------------------------------------------------------------------===//

#include "ParVecDirectiveInsertion.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"
//#include "llvm/Analysis/Intel_LoopAnalysis/HIRParVecAnalysis.h"
//#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "parvec-transform"

using namespace llvm;
using namespace llvm::loopopt;

bool ParVecDirectiveInsertion::runOnFunction(Function &Func) {
  if (skipFunction(Func))
    return false;

  auto HIRF = &getAnalysis<HIRFramework>();
  auto PVA = &getAnalysis<HIRParVecAnalysis>();

  // Analyze for all regions. Due to the on-demand nature of ParVecAnalysis,
  // this explicit call should not be necessary, but it's easier for
  // debugging. Keep this here until we confirm that on-demand functionality
  // is rock solid.
  PVA->analyze(Mode);
  DEBUG(dbgs() << "Analysis results for all regions\n");
  DEBUG(PVA->print(dbgs()));

  // Insert Directives where VecOkay/ParOkay are seen. Recompute
  // ParVecAnalysis result if stored info doesn't match the analysis
  // mode required.
  Visitor PVV(Func, HIRF, PVA, Mode);
  HIRF->getHLNodeUtils().visitAll(PVV);
  return PVV.getInserted();
}

void ParVecDirectiveInsertion::Visitor::visit(HLLoop *HLoop) {
  auto Info = PVA->getInfo(Mode, HLoop);

  if (HLoop->isInnermost()) {
    SkipNode = HLoop;
  }

  // Insert vectorization directives?
  bool Insert = (Mode == ParVecInfo::VectorForVectorizer ||
                 Mode == ParVecInfo::VectorForVectorizerInnermost) &&
                Info->getVecType() == ParVecInfo::VecOkay;
  if (Insert) {
    insertVecDirectives(HLoop, Info);
    return;
  }
  // Insert parallelization directives?
  Insert = (Mode == ParVecInfo::ParallelForThreadizer &&
            Info->getVecType() == ParVecInfo::ParOkay);
  if (Insert) {
    insertParDirectives(HLoop, Info);
  }
}

HLInst *ParVecDirectiveInsertion::Visitor::insertDirective(HLLoop *Lp,
                                                           OMP_DIRECTIVES Dir,
                                                           bool Append) {
  SmallVector<RegDDRef *, 1> CallArgs;
  auto Intrin =
      Intrinsic::getDeclaration(Func.getParent(), Intrinsic::intel_directive);
  assert(Intrin && "Cannot get declaration for intrinsic");

  auto DD = createRegDDRef(Dir);
  CallArgs.push_back(DD);

  // Create "call void @llvm.intel.directive(metadata !9)"
  auto Inst = Lp->getHLNodeUtils().createCall(Intrin, CallArgs);

  if (Append) {
    Lp->getHLNodeUtils().insertAfter(Lp, Inst);
  } else {
    Lp->getHLNodeUtils().insertBefore(Lp, Inst);
  }
  return Inst;
}

void ParVecDirectiveInsertion::Visitor::insertVecDirectives(
    HLLoop *Lp, const ParVecInfo *Info) {
  DEBUG(dbgs() << "Inserting Vec directives for\n");
  DEBUG(Info->print(dbgs()));
  Inserted = true;

  // Insert SIMD directives and clauses
  insertDirective(Lp, DIR_OMP_SIMD, false /* prepend */);
  insertDirective(Lp, DIR_QUAL_LIST_END, false /* prepend */);

  // TODO: Clauses

  // End of SIMD directives and clauses insertion
  insertDirective(Lp, DIR_QUAL_LIST_END, true /* append  */);
  insertDirective(Lp, DIR_OMP_END_SIMD, true /* append  */);
}

void ParVecDirectiveInsertion::Visitor::insertParDirectives(
    HLLoop *Lp, const ParVecInfo *Info) {
  DEBUG(dbgs() << "Inserting Par directives for\n");
  DEBUG(Info->print(dbgs()));
  Inserted = true;

  // TODO: Implement!
}

RegDDRef *
ParVecDirectiveInsertion::Visitor::createRegDDRef(OMP_DIRECTIVES Dir) {
  return HIRF->getDDRefUtils().createConstDDRef(
      IntelIntrinsicUtils::createDirectiveMetadataAsValue(*(Func.getParent()),
                                                          Dir));
}

RegDDRef *ParVecDirectiveInsertion::Visitor::createRegDDRef(OMP_CLAUSES Qual) {
  return HIRF->getDDRefUtils().createConstDDRef(
      IntelIntrinsicUtils::createClauseMetadataAsValue(*(Func.getParent()),
                                                       Qual));
}
