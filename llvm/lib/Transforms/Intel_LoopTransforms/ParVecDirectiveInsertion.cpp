//===- ParVecDirectiveInsertion.cpp - Implements VecDirectiveInsertion class
//-===//
//                               Also Implements ParDirectionInsertion class
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
//#include "llvm/Analysis/Intel_LoopAnalysis/HIRParVecAnalysis.h"
//#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "parvec-transform"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<size_t>
    OptReportDDEdgesLimit("optreport-ddedges-limit", cl::init(10), cl::Hidden,
                          cl::desc("Limit DD edges count for optreport level 3 "
                                   "(limited to 1 for lower levels)"));

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

  // Add justification why can't vectorize.
  OptReportBuilder &ORBuilder =
      HLoop->getHLNodeUtils().getHIRFramework().getORBuilder();
  if (ORBuilder.isOptReportOn())
    switch (Info->getVecType()) {
    case ParVecInfo::NOVECTOR_PRAGMA_LOOP:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15319u, "Loop");
      break;
    case ParVecInfo::FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE: {
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15344u, "Loop",
                                  "");
      auto &Edges = Info->getVecEdges();
      size_t Limit = ORBuilder.getVerbosity() >= OptReportVerbosity::Medium
                         ? OptReportDDEdgesLimit
                         : 1;
      for (size_t I = 0; I < Edges.size() && I < Limit; I++)
        ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15346u,
                                    Edges[I]->getOptReportStr());
    } break;
    case ParVecInfo::FE_DIAG_VEC_FAIL_EMPTY_LOOP:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15414u, "Loop");
      break;
    case ParVecInfo::SWITCH_STMT:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15535u, "Loop");
      break;
    case ParVecInfo::UNKNOWN_CALL:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15527u, "Loop",
                                  "");
      break;
    case ParVecInfo::NON_DO_LOOP:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15521u, "Loop",
                                  "");
      break;
    case ParVecInfo::UNROLL_PRAGMA_LOOP:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15427u);
      break;
    case ParVecInfo::FE_DIAG_VEC_NOT_INNERMOST:
      ORBuilder(*HLoop).addRemark(OptReportVerbosity::Medium, 15553u);
      break;
    default:
      // No specific diagnostics.
      break;
    }

  // Insert parallelization directives?
  Insert = (Mode == ParVecInfo::ParallelForThreadizer &&
            Info->getVecType() == ParVecInfo::ParOkay);
  if (Insert) {
    insertParDirectives(HLoop, Info);
  }
}

HLInst *ParVecDirectiveInsertion::Visitor::insertBeginRegion(
    HLLoop *Lp, OMP_DIRECTIVES Dir, const ParVecInfo *Info) {
  auto Intrin = Intrinsic::getDeclaration(Func.getParent(),
                                          Intrinsic::directive_region_entry);
  assert(Intrin && "Cannot get declaration for intrinsic");

  SmallVector<llvm::OperandBundleDef, 1> IntrinOpBundle;
  llvm::OperandBundleDef OpBundle(
      std::string(IntrinsicUtils::getDirectiveString(Dir)),
      ArrayRef<llvm::Value *>{});
  IntrinOpBundle.push_back(OpBundle);

  SmallVector<RegDDRef *, 1> BundleRefs;
  unsigned Safelen = Info->getSafelen();
  // Emit safelen clause if needed
  if (Safelen != UINT_MAX) {
    Constant *SafelenConst = ConstantInt::get(
        Type::getInt32Ty(Lp->getHLNodeUtils().getContext()), Safelen);
    llvm::OperandBundleDef SafelenBundle(
        std::string(IntrinsicUtils::getClauseString(llvm::QUAL_OMP_SAFELEN)),
        ArrayRef<llvm::Value *>{SafelenConst});
    IntrinOpBundle.push_back(SafelenBundle);
    BundleRefs.push_back(Lp->getDDRefUtils().createConstDDRef(SafelenConst));
  }

  SmallVector<RegDDRef *, 1> CallArgs;
  // Create "%entry = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]"
  auto Inst = Lp->getHLNodeUtils().createCall(Intrin, CallArgs, "entry.region",
                                              nullptr /*Lvalddref*/,
                                              IntrinOpBundle, BundleRefs);

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
  llvm::OperandBundleDef OpBundle(
      std::string(IntrinsicUtils::getDirectiveString(Dir)),
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
  HLInst *RegionEntry = insertBeginRegion(Lp, DIR_VPO_AUTO_VEC, Info);
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

