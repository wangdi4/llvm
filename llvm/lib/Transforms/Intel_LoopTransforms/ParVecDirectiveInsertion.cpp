//===- ParVecDirectiveInsertion.cpp - Implements VecDirectiveInsertion class
//-===//
//                               Also Implements ParDirectionInsertion class
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
//#include "llvm/Analysis/Intel_LoopAnalysis/HIRParVecAnalysis.h"
//#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "parvec-transform"

using namespace llvm;
using namespace llvm::loopopt;

bool ParVecDirectiveInsertion::runOnFunction(Function &Func, HIRFramework *HIRF,
                                             HIRParVecAnalysis *HPVA) {
  // Analyze for all regions. Due to the on-demand nature of ParVecAnalysis,
  // this explicit call should not be necessary, but it's easier for
  // debugging. Keep this here until we confirm that on-demand functionality
  // is rock solid.
  HPVA->analyze(Mode);
  LLVM_DEBUG(dbgs() << "Analysis results for all regions\n");
  LLVM_DEBUG(HPVA->printAnalysis(dbgs()));

  // Insert Directives where VecOkay/ParOkay are seen. Recompute
  // ParVecAnalysis result if stored info doesn't match the analysis
  // mode required.
  Visitor PVV(Func, HIRF, HPVA, Mode);
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

HLInst *
ParVecDirectiveInsertion::Visitor::insertBeginRegion(HLLoop *Lp,
                                                     OMP_DIRECTIVES Dir) {
  auto Intrin = Intrinsic::getDeclaration(Func.getParent(),
                                          Intrinsic::directive_region_entry);
  assert(Intrin && "Cannot get declaration for intrinsic");

  SmallVector<llvm::OperandBundleDef, 1> IntrinOpBundle;
  llvm::OperandBundleDef OpBundle(IntrinsicUtils::getDirectiveString(Dir),
                                  ArrayRef<llvm::Value *>{});
  IntrinOpBundle.push_back(OpBundle);
  // TODO: Operand bundles for any clauses in auto-vec directive

  // No arguments to region.entry call
  SmallVector<RegDDRef *, 1> CallArgs;
  // Create "%entry = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]"
  auto Inst = Lp->getHLNodeUtils().createCall(
      Intrin, CallArgs, "entry.region", nullptr /*Lvalddref*/, IntrinOpBundle);

  HLNodeUtils::insertBefore(Lp, Inst);
  return Inst;
}

HLInst *ParVecDirectiveInsertion::Visitor::insertEndRegion(HLLoop *Lp,
                                                           OMP_DIRECTIVES Dir,
                                                           HLInst *Entry) {
  auto Intrin = Intrinsic::getDeclaration(Func.getParent(),
                                          Intrinsic::directive_region_exit);
  assert(Intrin && "Cannot get declaration for intrinsic");

  SmallVector<llvm::OperandBundleDef, 1> IntrinOpBundle;
  llvm::OperandBundleDef OpBundle(IntrinsicUtils::getDirectiveString(Dir),
                                  ArrayRef<llvm::Value *>{});
  IntrinOpBundle.push_back(OpBundle);

  SmallVector<RegDDRef *, 1> CallArgs;
  CallArgs.push_back(Entry->getLvalDDRef()->clone());
  // Create "@llvm.directive.region.exit(%entry); [ DIR.VPO.END.AUTO.VEC() ]"
  auto Inst = Lp->getHLNodeUtils().createCall(
      Intrin, CallArgs, "exit.region", nullptr /*Lvalddref*/, IntrinOpBundle);

  HLNodeUtils::insertAfter(Lp, Inst);
  return Inst;
}

void ParVecDirectiveInsertion::Visitor::insertVecDirectives(
    HLLoop *Lp, const ParVecInfo *Info) {
  LLVM_DEBUG(dbgs() << "Inserting Vec directives for\n");
  LLVM_DEBUG(Info->print(dbgs()));
  Inserted = true;

  // Insert region entry for auto-vec directive
  HLInst *RegionEntry = insertBeginRegion(Lp, DIR_VPO_AUTO_VEC);
  // Insert region exit for auto-vec directive
  insertEndRegion(Lp, DIR_VPO_END_AUTO_VEC, RegionEntry);
}

void ParVecDirectiveInsertion::Visitor::insertParDirectives(
    HLLoop *Lp, const ParVecInfo *Info) {
  LLVM_DEBUG(dbgs() << "Inserting Par directives for\n");
  LLVM_DEBUG(Info->print(dbgs()));
  Inserted = true;

  // TODO: Implement!
}

