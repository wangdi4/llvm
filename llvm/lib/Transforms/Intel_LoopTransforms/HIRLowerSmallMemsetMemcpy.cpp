//===---------------- HIRLowerSmallMemsetMemcpy.cpp ----------------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// ===--------------------------------------------------------------------===//
//
// The pass lowers a small sized (<=64) alloca memcpy/memset before
// hir-pre-vec-complete-unroll pass into a loop of store/load assignment. Not
// only will this help complete unroll's cost model, it will allow HIRLMM pass
// to optimize away more loads/stores after unroll.
//
// Ex.:
// Before:
// @llvm.memcpy.p0i8.p0i8.i64(&((i8*)(%dst)[0]),  &((i8*)(%src)[0]),  20,  0);
//
// After:
// DO i1 =0, 5
//   (%dst)[i1] = (%src)[i1]
// END DO
//

#include "llvm/Transforms/Intel_LoopTransforms/HIRLowerSmallMemsetMemcpyPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-lower-small-memset-memcpy"
#define OPT_DESC "HIR Lower Small Memset/Memcpy Pass"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableLowerSmallMemsetMemcpyPass("disable-" OPT_SWITCH, cl::init(false),
                                      cl::Hidden,
                                      cl::desc("Disable " OPT_DESC " pass"));

STATISTIC(NumLoweredMemsetMemcpy,
          "Number of memcpy/memset calls lowered to loops.");

class MemsetMemcpyVisitor final : public HLNodeVisitorBase {

public:
  MemsetMemcpyVisitor() {}

  bool visit(HLInst *Inst);
  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

private:
  bool doAnalysis(HLInst *Inst);
  void doTransform(HLInst *Inst);
};

bool MemsetMemcpyVisitor::visit(HLInst *Inst) {

  if (doAnalysis(Inst)) {
    doTransform(Inst);
    return true;
  }

  return false;
}

class HIRLowerSmallMemsetMemcpy {
public:
  HIRLowerSmallMemsetMemcpy(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

private:
  HIRFramework &HIRF;
};

bool MemsetMemcpyVisitor::doAnalysis(HLInst *Inst) {
  Intrinsic::ID IntrinID;
  if (!Inst->isIntrinCall(IntrinID)) {
    return false;
  }

  bool IsMemset = false;

  if (IntrinID == Intrinsic::memset) {
    IsMemset = true;
  } else if (IntrinID != Intrinsic::memcpy) {
    return false;
  }

  LLVM_DEBUG(dbgs() << (IsMemset ? "memset" : "memcpy") << "() is found.\n");

  (void)IsMemset;
  return true;
}

void MemsetMemcpyVisitor::doTransform(HLInst *Inst) { return; }

bool HIRLowerSmallMemsetMemcpy::run() {

  MemsetMemcpyVisitor MMV;

  for (auto &RegIt : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLRegion &Reg = cast<HLRegion>(RegIt);
    HLNodeUtils::visitRange(MMV, Reg.child_begin(), Reg.child_end());
  }

  return NumLoweredMemsetMemcpy;
}

PreservedAnalyses
HIRLowerSmallMemsetMemcpyPass::runImpl(Function &F, FunctionAnalysisManager &AM,
                                       HIRFramework &HIRF) {
  if (DisableLowerSmallMemsetMemcpyPass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");
  HIRLowerSmallMemsetMemcpy(HIRF).run();

  return PreservedAnalyses::all();
}
