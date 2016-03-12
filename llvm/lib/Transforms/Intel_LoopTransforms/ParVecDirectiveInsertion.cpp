//===- ParVecDirectiveInsertion.cpp - Implements VecDirectiveInsertion class
//-===//
//                               Also Implements ParDirectionInsertion class
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements ParDirectiveInsertion/VecDirectiveInsertion
// transformation. They identify parallelization/vectorization candidate
// loops and mark them via directive intrinsics.
//
// Available options:
//   -hir-enable-par         Enable auto-parallelization (at O2 and above)
//   -hir-disable-vec        Disable auto-vectorization (at O2 and above)
//   -hir-disable-vec-outer  Disable outer loop auto-vectorization (at O3)
//
// See also ParVecAnalysis.cpp for diagnostic related flags.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParVecAnalysis.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "parvec-transform"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

static cl::opt<bool>
    AutoPar("hir-enable-par", cl::init(false), cl::Hidden,
            cl::desc("Enable Auto Parallelization at O2 and above"));
static cl::opt<bool>
    NoAutoVec("hir-disable-vec", cl::init(false), cl::Hidden,
              cl::desc("Disable Auto Vectorization at O2 and above"));
static cl::opt<bool>
    NoOuterVec("hir-disable-vec-outer", cl::init(false), cl::Hidden,
               cl::desc("Disable Outer Loop Auto Vectorization at O3"));

namespace {

/// \brief Worker class for directive insertion.
class ParVecVisitor final : public HLNodeVisitorBase {
  Function &F;
  Module &M;
  ParVecAnalysis *PVA;
  ParVecInfo::AnalysisMode Mode;
  /// \brief Status flag to indicate whether we modified the HIR or not.
  bool Inserted;

  /// \brief Insert auto-vec directives to the loop.
  void insertVecDirectives(HLLoop *L, ParVecInfo *Info);
  /// \brief Insert auto-par directives to the loop.
  void insertParDirectives(HLLoop *L, ParVecInfo *Info);
  /// \brief Returns metadata RegDDRef for the OpenMP directive.
  RegDDRef *getRegDDRef(OMP_DIRECTIVES Dir) {
    return DDRefUtils::createConstDDRef(VPOUtils::getMetadataAsValue(M, Dir));
  }
  /// \brief Returns metadata RegDDRef for the OpenMP clause.
  RegDDRef *getRegDDRef(OMP_CLAUSES Qual) {
    return DDRefUtils::createConstDDRef(VPOUtils::getMetadataAsValue(M, Qual));
  }
  /// \brief Prepend/Append a directive call to the Loop.
  HLInst *insertDirective(HLLoop *L, OMP_DIRECTIVES Dir, bool Append);

public:
  ParVecVisitor(Function &F, ParVecAnalysis *PVA, ParVecInfo::AnalysisMode Mode)
      : F(F), M(*(F.getEntryBlock().getModule())), PVA(PVA), Mode(Mode),
        Inserted(false) {}
  void visit(HLNode *N) {}
  void postVisit(HLNode *N) {}
  /// \brief Checks if directive insertion is needed for the loop
  /// and invokes insertDirective() function.
  void visit(HLLoop *L);
  /// \brief Returns true if directive is inserted for at least one loop.
  bool getInserted() { return Inserted; }
};

/// \brief Abstract parent class for ParDirectiveInsertion/VecDirectiInsertion.
class ParVecDirectiveInsertion : public HIRTransformPass {
  ParVecInfo::AnalysisMode Mode;

public:
  ParVecDirectiveInsertion(char &ID, ParVecInfo::AnalysisMode Mode)
      : HIRTransformPass(ID), Mode(Mode) {
    // Be sure to take the ID parameter as reference, not by value.
    // Otherwise, it will be hard to diagnose bugs.
  }

  /// \brief Analyze auto-parallelizability/auto-vectorizability of the loops
  /// in the function and insert directives for auto-parallelization/
  /// auto-vectorization.
  bool runOnFunction(Function &F) override {
    auto PVA = &getAnalysis<ParVecAnalysis>();

    // Analyze for all regions. Due to the on-demand nature of ParVecAnalysis,
    // this explicit call should not be necessary, but it's easier for
    // debugging. Keep this here until we confirm that on-demand functionality
    // is rock solid,
    PVA->analyze(Mode);
    DEBUG(dbgs() << "Analysis results for all regions\n");
    DEBUG(PVA->print(dbgs()));

    // Insert Directives where VecOkay/ParOkay are seen. Recompute
    // ParVecAnalysis result if stored info doesn't match the analysis
    // mode required.
    ParVecVisitor V(F, PVA, Mode);
    HLNodeUtils::visitAll(V);
    return V.getInserted();
  }
  void releaseMemory() override {}

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<ParVecAnalysis>();
    AU.setPreservesAll();
  }
};

/// \brief Invoke auto-parallelizability analysis (including cost model) and
/// insert auto-parallelization directive to the loops. When the directive
/// is inserted to a loop, auto-parallelization decision is already made.
class ParDirectiveInsertion : public ParVecDirectiveInsertion {
public:
  static char ID;

  ParDirectiveInsertion()
      : ParVecDirectiveInsertion(ID, ParVecInfo::ParallelForThreadizer) {
    initializeParDirectiveInsertionPass(*PassRegistry::getPassRegistry());
  }
  /// \brief Analyze auto-parallelizability of the loops.
  bool runOnFunction(Function &F) override {
    if (!AutoPar) {
      DEBUG(dbgs() << "Par Directive Insertion skipped"
                      " due to lack of -hir-enable-par.\n");
      return false;
    }
    if (ParVecAnalysis::isSIMDEnabledFunction(F)) {
      DEBUG(dbgs() << "Par Directive Insertion skipped"
                      " for vector variants of SIMD Enabled Function : "
                   << F.getName() << "\n");
      return false;
    }
    DEBUG(dbgs() << "Par Directive Insertion for Function : " << F.getName()
                 << "\n");
    return ParVecDirectiveInsertion::runOnFunction(F);
  }
};

/// \brief Invoke auto-vectorization legality analysis and insert
/// auto-vectorization candidate directive to the loops. When the directive
/// is inserted to a loop, further analysis will be performed by the vectorizer
/// before final auto-vectorization decision is made.
class VecDirectiveInsertion : public ParVecDirectiveInsertion {
  bool OuterVec;

public:
  static char ID;

  VecDirectiveInsertion(bool OuterVec = true)
      : ParVecDirectiveInsertion(
            ID, OuterVec && !NoOuterVec
                    ? ParVecInfo::VectorForVectorizer
                    : ParVecInfo::VectorForVectorizerInnermost),
        OuterVec(OuterVec && !NoOuterVec) {
    initializeVecDirectiveInsertionPass(*PassRegistry::getPassRegistry());
  }
  /// \brief Analyze auto-vectorizability of the loops.
  bool runOnFunction(Function &F) override {
    if (NoAutoVec) {
      DEBUG(dbgs() << "Vec Directive Insertion disabled"
                      " due to -hir-disable-vec.\n");
      return false;
    }
    if (ParVecAnalysis::isSIMDEnabledFunction(F)) {
      DEBUG(dbgs() << "Vec Directive Insertion skipped"
                      " for vector variants of SIMD Enabled Function : "
                   << F.getName() << "\n");
      return false;
    }
    DEBUG(dbgs() << "Vec Directive Insertion (Outer Loop "
                 << (OuterVec ? "Enabled" : "Disabled")
                 << ") for Function : " << F.getName() << "\n");
    return ParVecDirectiveInsertion::runOnFunction(F);
  }
};

} // unnamed namespace

char ParDirectiveInsertion::ID = 0;
INITIALIZE_PASS_BEGIN(ParDirectiveInsertion, "hir-parvec-par",
                      "Par Directive Insertion Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(ParVecAnalysis)
INITIALIZE_PASS_END(ParDirectiveInsertion, "hir-parvec-par",
                    "Par Directive Insertion Pass", false, false)

char VecDirectiveInsertion::ID = 0;
INITIALIZE_PASS_BEGIN(VecDirectiveInsertion, "hir-parvec-vec",
                      "Vec Directive Insertion Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(ParVecAnalysis)
INITIALIZE_PASS_END(VecDirectiveInsertion, "hir-parvec-vec",
                    "Vec Directive Insertion Pass", false, false)

FunctionPass *llvm::createParDirectiveInsertionPass() {
  return new ParDirectiveInsertion();
}

FunctionPass *llvm::createVecDirectiveInsertionPass(bool OuterVec) {
  return new VecDirectiveInsertion(OuterVec);
}

void ParVecVisitor::visit(HLLoop *HLoop) {
  auto Info = PVA->getInfo(Mode, HLoop);

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

HLInst *ParVecVisitor::insertDirective(HLLoop *L, OMP_DIRECTIVES Dir,
                                       bool Append) {
  SmallVector<RegDDRef *, 1> CallArgs;
  auto F = Intrinsic::getDeclaration(&M, Intrinsic::intel_directive);
  assert(F && "Cannot get declaration for intrinsic");

  auto DD = getRegDDRef(Dir);
  CallArgs.push_back(DD);

  // Create "call void @llvm.intel.directive(metadata !9)"
  auto I = HLNodeUtils::createCall(F, CallArgs);

  if (Append) {
    HLNodeUtils::insertAfter(L, I);
  } else {
    HLNodeUtils::insertBefore(L, I);
  }
  return I;
}

void ParVecVisitor::insertVecDirectives(HLLoop *L, ParVecInfo *Info) {
  DEBUG(dbgs() << "Inserting Vec directives for\n");
  DEBUG(Info->print(dbgs()));
  Inserted = true;

  // Insert SIMD directives and clauses
  insertDirective(L, DIR_OMP_SIMD, false /* prepend */);

  // TODO: Clauses

  // End of SIMD directives and clauses insertion
  // insertDirective(L, DIR_QUAL_LIST_END, false /* prepend */);

  // Insert END SIMD directives
  // insertDirective(L, DIR_QUAL_LIST_END, true  /* append  */);
  // insertDirective(L, DIR_OMP_END_SIMD,  true  /* append  */);

  //  L->getParentRegion()->setGenCode();
}

void ParVecVisitor::insertParDirectives(HLLoop *L, ParVecInfo *Info) {
  DEBUG(dbgs() << "Inserting Par directives for\n");
  DEBUG(Info->print(dbgs()));
  Inserted = true;

  // TODO: Implement!
}
