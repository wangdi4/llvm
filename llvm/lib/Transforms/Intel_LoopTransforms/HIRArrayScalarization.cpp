//===--- HIRArrayScalarization.cpp - ---------------------*- C++ -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//
// This file implements HIR Array Scalarization class.
//===-----------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "HIRArrayScalarization.h"

#define OPT_SWITCH "hir-array-scalarization"
#define OPT_DESC "HIR Array Scalarization"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::arrayscalarization;

const StringRef ArrayScalarizeTempName = "array-scalarize";

STATISTIC(HIRArrayScalarizationGroupsPromoted,
          "Number of HIR Array-Scalarization Group(s) Promoted");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void printRefVec(SmallVectorImpl<const RegDDRef *> &Group,
                        StringRef Msg) {
  dbgs() << Msg << ": <" << Group.size() << ">\n";
  for (auto *Ref : Group) {
    dbgs() << "\t";
    Ref->dump();
    dbgs() << "\t";
    Ref->isLval() ? dbgs() << "W" : dbgs() << "R";
    dbgs() << "\n";
  }
}
#endif

static bool isReadOnlyGroup(SmallVectorImpl<const RegDDRef *> &Group) {
  for (auto &Ref : Group) {
    if (Ref->isLval()) {
      return false;
    }
  }

  return true;
}

// -----------------------------------------------------------------------
/*  Following will be ArrayScalarizationMemRefGroup's implementation     */
// -----------------------------------------------------------------------

ArrayScalarizationMemRefGroup::ArrayScalarizationMemRefGroup(
    SmallVectorImpl<const RegDDRef *> &Group, HLLoop *Lp)
    : Lp(Lp), IsRelaxedGroup(false) {
  for (auto *Ref : Group) {
    RefVec.push_back(const_cast<RegDDRef *>(Ref));
  }
  assert((!RefVec.empty()) && "Expect RefVec be non-empty");

  // Turn on the RelaxedGroup flag if the refs in the same group are different
  // in relaxed mode.
  const RegDDRef *Ref0 = Group[0];
  if (std::find_if(std::next(Group.begin()), Group.end(),
                   [&](const RegDDRef *Ref) {
                     return !DDRefUtils::areEqual(Ref, Ref0);
                   }) != Group.end()) {
    IsRelaxedGroup = true;
  }
}

void ArrayScalarizationMemRefGroup::createATemp(HLLoop *Loop, RegDDRef *Ref,
                                                RegDDRef *&TmpRef) {
  assert(!TmpRef && "Expect TmpRef be a nullptr");
  HLNodeUtils &HNU = Loop->getHLNodeUtils();
  TmpRef = HNU.createTemp(Ref->getSrcType(), ArrayScalarizeTempName);
}

void ArrayScalarizationMemRefGroup::replaceRefWithTmp(RegDDRef *Ref,
                                                      RegDDRef *TmpRef) {
  assert(TmpRef && "Expect tmp be a valid ptr");
  assert(Ref && "Expect ref be a valid ptr");
  HIRTransformUtils::replaceOperand(Ref, TmpRef->clone());
}

// handle only groups of {W, R+}
bool ArrayScalarizationMemRefGroup::transform(void) {
  // create a temp outside of innermost lp
  RegDDRef *TmpRef = nullptr;
  RegDDRef *Ref0 = RefVec[0];
  createATemp(Lp, Ref0, TmpRef);
  assert(TmpRef && "Expect a valid TmpRef");

  // Create an additional cast from (i64)0 to (f64)0.0 only for relaxed group
  // where the RHS of the store is a constant integer 0.
  //
  // E.g. f64 A[N][N]
  // [FROM]
  // (i64*)A[0][0]  = 0    ;   0 is i64 type
  //  ...
  //     .          = A[0][0]
  //
  // [TO]
  // (i64*)A[0][0]  = 0.0  ;   0.0 is f64 type
  //  ...
  //     .          = A[0][0]
  //
  // After this compile-time cast, the relaxed group can be transformed as a
  // regular group.
  if (IsRelaxedGroup) {
    HLInst *PInst = dyn_cast<HLInst>(Ref0->getHLDDNode());
    RegDDRef *Rval = PInst->getRvalDDRef();

    // Only support integer const 0 or floating-point 0.00:
    int64_t IVal = -1;
    bool IsIntConstant = Rval->isIntConstant(&IVal);
    // --------------------------------------------------------------
    // To identify the usage cases in LIT, search the immed value
    // --------------------------------------------------------------
    // case0:
    // (i64*)(@"mat_times_vec_$AE_IP1")[0][0][0] = 0;
    //
    // case1:
    // (i64*)(@"mat_times_vec_$AE_IP1")[0][1][0] = 4607182418800017408;
    //
    // case2:
    // store i64 4562254508917369340, i64* %1848, align 8, !noalias !73
    // --------------------------------------------------------------
    // raw bit stream          |  float value
    // 0                       |  0.0
    // 4607182418800017408     |  1.00000
    // 4562254508917369340     |  0.01000
    // ..                      |  ..
    // --------------------------------------------------------------
    if (!IsIntConstant) {
      LLVM_DEBUG(dbgs() << "Error! -- Can't handle a relaxed group with a "
                           "non-constant integer RHS on store\n";);
      return false;
    }

    auto &DRU = Rval->getDDRefUtils();
    assert(Ref0->getSrcType()->isFloatingPointTy() &&
           "Expect Ref0's SrcType is a floating-point type");
    HLNodeUtils &HNU = Lp->getHLNodeUtils();
    ConstantInt *ConstInt =
        llvm::ConstantInt::get(Type::getInt64Ty(HNU.getContext()), IVal);
    auto *DoubleVal = ConstantFoldCastOperand(
        Instruction::BitCast, ConstInt, Type::getDoubleTy(HNU.getContext()),
        HNU.getDataLayout());
    RegDDRef *ConstRef = DRU.createConstDDRef(DoubleVal);
    PInst->setRvalDDRef(ConstRef);

    LLVM_DEBUG({
      const APFloat &APF = dyn_cast<ConstantFP>(DoubleVal)->getValue();
      double FPConst = APF.convertToDouble();
      dbgs() << "int val: " << IVal << "\tdouble val: " << FPConst << "\n";
      dbgs() << "DestType: ";
      Rval->getDestType()->dump();
      dbgs() << "Node:\n";
      PInst->dump(1);
    });
  }

  // replace each ref in the group with the tmp
  for (auto *Ref : RefVec) {
    replaceRefWithTmp(Ref, TmpRef);
  }

  return true;
}

void ArrayScalarizationMemRefGroup::print(raw_ostream &OS,
                                          bool PrintDetails) const {
  OS << "ASMemRefGroup: " << RefVec.size() << " -\t";
  for (auto Ref : RefVec) {
    OS << "  " << (Ref->isLval() ? " W " : " R ");
  }
  OS << "  Symbase: " << RefVec[0]->getSymbase() << "\n";

  formatted_raw_ostream FOS(OS);
  unsigned Count = 0;
  for (auto Ref : RefVec) {
    OS << "  " << Count++ << "\t";
    Ref->print(FOS, PrintDetails);
    OS << "  " << (Ref->isLval() ? "W" : "R") << "\n";
  }

  if (PrintDetails) {
    // SBS:
    FOS << "\n IsRelaxedGroup: " << IsRelaxedGroup << "\n"
        << "}\n";
  }
}

// -------------------------------------------------------------------
/*  Following will be HIRArrayScalarization's implementation       */
// -------------------------------------------------------------------

// Do array scalarization by:
//
// Scan the entire innermost-loop's body, for each HLInst:
// - find its Lval: if it is a suitable RegDDRef with all int-const
//   subscritp(s):
//   . follow the DDEdge, and all uses, collect them all into a group
//   . filter non-supported group:
//     the only supported group has 1 leading W, following with 0 or more
//     read(s).
//
// - transform the supported group:
//  . create a temp in the lp's preheader:
//   -no need to create a load, since the 1st ref is a store.
//
//  . replace each ref with the tmp:
//   -no need to create a store in any loop's postexit, since the array is
//    dead after the loop.
//
bool HIRArrayScalarization::doScalarization(HLLoop *Lp,
                                            SmallSet<unsigned, 8> &SBS) {
  SmallVector<ArrayScalarizationMemRefGroup, 8> ASMemRefGrpVec;

  HIRLoopLocality::RefGroupVecTy EqualityGroups;
  SmallSet<unsigned, 8> UniqueGroupSymbases;
  HIRLoopLocality::populateEqualityGroups(Lp->child_begin(), Lp->child_end(),
                                          EqualityGroups, &UniqueGroupSymbases);
  for (auto &Group : EqualityGroups) {
    assert(!Group.empty() && "Expect only non-empty group(s)");

    if (!SBS.count(Group[0]->getSymbase())) {
      continue;
    }

    if (!DDRefUtils::isMemRefAllDimsConstOnly(Group[0])) {
      continue;
    }

    assert(!isReadOnlyGroup(Group) &&
           "not expect a read-only group with qualified symbase");

    LLVM_DEBUG({
      printRefVec(Group, "A group suitable for array scalarization: -");
      Group[0]->dump();
      dbgs() << "\tsymbase: " << Group[0]->getSymbase() << "\n";
    });

    // Save the group
    ASMemRefGrpVec.emplace_back(Group, Lp);
  }

  if (ASMemRefGrpVec.empty()) {
    LLVM_DEBUG(dbgs() << "No suitable group available after collection\n";);
    return false;
  }

  // Check the current collection:
  LLVM_DEBUG({
    dbgs() << "Total qualified groups: <" << ASMemRefGrpVec.size() << ">\n";
    unsigned Count = 0;
    for (auto &G : ASMemRefGrpVec) {
      dbgs() << Count++ << "\n";
      G.dump();
    }
  });

  // Do Transformation
  // [Note]
  // - no need to analyze each group before transformation, because all groups
  //   are analyzed before collection.
  bool Result = true;
  for (auto &MRG : ASMemRefGrpVec) {
    Result = MRG.transform() && Result;
  }

  HIRArrayScalarizationGroupsPromoted += ASMemRefGrpVec.size();
  return Result;
}
