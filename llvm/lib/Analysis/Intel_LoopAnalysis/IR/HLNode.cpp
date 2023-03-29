//===-- HLNode.cpp - Implements the HLNode class ---------------------===//
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
// This file implements the HLNode class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLRegion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/IR/MDBuilder.h"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> PrintTopSortNum("hir-details-topsort", cl::init(false),
                                     cl::Hidden,
                                     cl::desc("Print HLNode TopSort numbers"));

static cl::opt<bool> PrintLineNum("hir-details-line-num", cl::init(true),
                                  cl::Hidden,
                                  cl::desc("Print node origin line number"));

static cl::opt<bool> SkipVerboseIndent(
    "hir-details-no-verbose-indent", cl::init(false), cl::Hidden,
    cl::desc("Do not print HLNode's unique number. To be used "
             "for CHECKs generation."));

HLNode::HLNode(HLNodeUtils &HNU, unsigned SCID)
    : HNU(HNU), SubClassID(SCID), Parent(nullptr), TopSortNum(0),
      MaxTopSortNum(0), ProfileData(nullptr) {

  getHLNodeUtils().Objs.insert(this);
  Number = getHLNodeUtils().getUniqueHLNodeNumber();
}

HLNode::HLNode(const HLNode &HLNodeObj)
    : HNU(HLNodeObj.HNU), SubClassID(HLNodeObj.SubClassID), Parent(nullptr),
      TopSortNum(0), MaxTopSortNum(0), ProfileData(HLNodeObj.ProfileData) {

  getHLNodeUtils().Objs.insert(this);
  Number = getHLNodeUtils().getUniqueHLNodeNumber();
}

DDRefUtils &HLNode::getDDRefUtils() const {
  return getHLNodeUtils().getDDRefUtils();
}

CanonExprUtils &HLNode::getCanonExprUtils() const {
  return getHLNodeUtils().getCanonExprUtils();
}

BlobUtils &HLNode::getBlobUtils() const {
  return getHLNodeUtils().getBlobUtils();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HLNode::dump() const { this->dump(false); }

void HLNode::dump(bool Detailed) const {
  formatted_raw_ostream OS(dbgs());
  print(OS, 0, Detailed);
}
#endif

void HLNode::indent(formatted_raw_ostream &OS, unsigned Depth) const {

  static std::string LoopIndentString(100, ' ');
  static std::string SpaceString(IndentWidth, ' ');

  LoopIndentString.clear();

  if (!SkipVerboseIndent) {
    int Padding = 10;
    OS << "<" << Number;
    if (PrintTopSortNum) {
      Padding += 3;
      OS << ":" << TopSortNum << "(" << MaxTopSortNum << ")";
    }

    if (PrintLineNum) {
      Padding += 3;
      if (const DebugLoc Dbg = getDebugLoc()) {
        OS << ":" << Dbg.getLine();
      }
    }

    OS << ">";
    OS.PadToColumn(Padding);
  }

  auto Parent = getParent();

  /// Don't print loop marker "|" if the node is in preheader/postexit.
  if ((Depth > 0) && Parent && isa<HLLoop>(Parent) && isa<HLInst>(this) &&
      cast<HLInst>(this)->isInPreheaderOrPostexit()) {
    LoopIndentString = SpaceString + LoopIndentString;
    Depth--;
    Parent = Parent->getParent();
  }

  while ((Depth > 0) && Parent) {
    LoopIndentString = SpaceString + LoopIndentString;

    if (isa<HLLoop>(Parent)) {
      LoopIndentString = "|" + LoopIndentString;
    }

    Depth--;
    Parent = Parent->getParent();
  }

  OS.indent(IndentWidth * Depth);
  OS << LoopIndentString;
}

void HLNode::printFMF(raw_ostream &OS, FastMathFlags FMF) {
#if !INTEL_PRODUCT_RELEASE
  OS << "<";

  if (!FMF.isFast()) {
    bool First = true;

    if (FMF.noNaNs()) {
      OS << "nnan";
      First = false;
    }

    if (FMF.noInfs()) {
      if (!First) {
        OS << ",";
      }
      OS << "ninf";
      First = false;
    }

    if (FMF.noSignedZeros()) {
      if (!First) {
        OS << ",";
      }
      OS << "nsz";
      First = false;
    }

    if (FMF.allowReciprocal()) {
      if (!First) {
        OS << ",";
      }
      OS << "arcp";
      First = false;
    }
  } else {
    OS << "fast";
  }

  OS << ">";
#endif // !INTEL_PRODUCT_RELEASE
}

void HLNode::printPredicate(raw_ostream &OS, PredicateTy Pred) {
#if !INTEL_PRODUCT_RELEASE
  if (Pred == PredicateTy::FCMP_TRUE) {
    OS << " true ";
  } else if (Pred == PredicateTy::FCMP_FALSE) {
    OS << " false ";
  } else if ((Pred == PredicateTy::FCMP_OEQ) ||
             (Pred == PredicateTy::ICMP_EQ)) {
    OS << " == ";
  } else if ((Pred == PredicateTy::FCMP_ONE) ||
             (Pred == PredicateTy::ICMP_NE)) {
    OS << " != ";
  } else if ((Pred == PredicateTy::FCMP_OGT) ||
             (Pred == PredicateTy::ICMP_SGT)) {
    OS << " > ";
  } else if ((Pred == PredicateTy::FCMP_OGE) ||
             (Pred == PredicateTy::ICMP_SGE)) {
    OS << " >= ";
  } else if ((Pred == PredicateTy::FCMP_OLT) ||
             (Pred == PredicateTy::ICMP_SLT)) {
    OS << " < ";
  } else if ((Pred == PredicateTy::FCMP_OLE) ||
             (Pred == PredicateTy::ICMP_SLE)) {
    OS << " <= ";
  } else if (Pred == PredicateTy::FCMP_UEQ) {
    OS << " ==u ";
  } else if (Pred == PredicateTy::FCMP_UNE) {
    OS << " !=u ";
  } else if ((Pred == PredicateTy::FCMP_UGT) ||
             (Pred == PredicateTy::ICMP_UGT)) {
    OS << " >u ";
  } else if ((Pred == PredicateTy::FCMP_UGE) ||
             (Pred == PredicateTy::ICMP_UGE)) {
    OS << " >=u ";
  } else if ((Pred == PredicateTy::FCMP_ULT) ||
             (Pred == PredicateTy::ICMP_ULT)) {
    OS << " <u ";
  } else if ((Pred == PredicateTy::FCMP_ULE) ||
             (Pred == PredicateTy::ICMP_ULE)) {
    OS << " <=u ";
  } else if (Pred == PredicateTy::FCMP_ORD) {
    OS << " ORDERED ";
  } else if (Pred == PredicateTy::FCMP_UNO) {
    OS << " UNORDERED ";
  } else {
    llvm_unreachable("Unexpected predicate!");
  }
#endif // !INTEL_PRODUCT_RELEASE
}

HLNode *HLNode::getLexicalParent() const {
  HLNode *Parent = getParent();

  if (auto HInst = dyn_cast<HLInst>(this)) {
    if (HInst->isInPreheaderOrPostexit()) {
      assert(isa<HLLoop>(Parent) &&
             "Parent of preheader/postexit is expected to be a loop");
      Parent = Parent->getParent();
    }
  }

  return Parent;
}

HLLoop *HLNode::getParentLoop() const {
  assert(!isa<HLRegion>(this) && "Region cannot have a parent!");

  HLNode *Par = getParent();

  while (Par && !isa<HLLoop>(Par)) {
    Par = Par->getParent();
  }

  return cast_or_null<HLLoop>(Par);
}

HLLoop *HLNode::getLexicalParentLoop() const {
  auto ParLoop = getParentLoop();

  if (auto HInst = dyn_cast<HLInst>(this)) {
    if (ParLoop && HInst->isInPreheaderOrPostexit(ParLoop)) {
      ParLoop = ParLoop->getParentLoop();
    }
  }

  return ParLoop;
}

HLLoop *HLNode::getOutermostParentLoop() const {
  auto TempLoop = getParentLoop();
  HLLoop *ParLoop = nullptr;

  while (TempLoop) {
    ParLoop = TempLoop;
    TempLoop = TempLoop->getParentLoop();
  }

  return ParLoop;
}

HLLoop *HLNode::getParentLoopAtLevel(unsigned Level) const {
  assert(CanonExpr::isValidLoopLevel(Level) && "Invalid loop level!");
  assert((getNodeLevel() >= Level) && "Invalid level w.r.t this node!");

  const HLLoop *ParLoop = dyn_cast<HLLoop>(this);

  if (!ParLoop) {
    ParLoop = getParentLoop();
  }

  while (ParLoop && (ParLoop->getNestingLevel() > Level)) {
    ParLoop = ParLoop->getParentLoop();
  }

  assert(ParLoop && "Could not find parent at level!");

  return const_cast<HLLoop *>(ParLoop);
}

unsigned HLNode::getNodeLevel() const {
  assert(getParentRegionImpl() && "Node should be connected to a HLRegion!");

  // For the HLLoop nodes return Loop nesting level instead of the level of
  // attachment. This is a workaround for loop bounds to make them feel like
  // they are attached at the loop nesting level.
  if (auto CurrentLoop = dyn_cast<HLLoop>(this)) {
    return CurrentLoop->getNestingLevel();
  }

  HLLoop *Loop = getLexicalParentLoop();
  unsigned Level = Loop ? Loop->getNestingLevel() : 0;
  return Level;
}

HLRegion *HLNode::getParentRegionImpl() const {
  assert(!isa<HLRegion>(this) && "Region cannot not have a parent!");

  HLNode *Par = getParent();

  while (Par && !isa<HLRegion>(Par)) {
    Par = Par->getParent();
  }

  return cast_or_null<HLRegion>(Par);
}

HLRegion *HLNode::getParentRegion() const {
  auto Reg = getParentRegionImpl();
  assert(Reg && "getParentRegion() called on detached node!");

  return Reg;
}

void HLNode::verify() const {
  assert((isa<HLRegion>(this) || getParent() != nullptr) &&
         "Non-Region HLNode should have a parent node");
}

unsigned HLNode::getMinTopSortNum() const {
  auto Lp = dyn_cast<HLLoop>(this);

  if (Lp && Lp->hasPreheader()) {
    return Lp->pre_begin()->getTopSortNum();
  }

  return getTopSortNum();
}

HLNode *HLNode::clone(HLNodeMapper *NodeMapper) const {
  HLContainerTy NContainer;
  getHLNodeUtils().cloneSequence(&NContainer, this, nullptr, NodeMapper);
  return &NContainer.front();
}

HLNode *HLNode::cloneBaseImpl(const HLNode *Node, GotoContainerTy *GotoList,
                              LabelMapTy *LabelMap, HLNodeMapper *NodeMapper) {
  HLNode *Clone = Node->cloneImpl(GotoList, LabelMap, NodeMapper);
  if (NodeMapper) {
    NodeMapper->map(Node, Clone);
  }
  return Clone;
}

HLNode *HLNode::getPrevNextNodeImpl(bool Prev) {
  assert(!isa<HLRegion>(this) && "Region not expected!");

  auto FirstOrLastNode =
      Prev ? getHLNodeUtils().getFirstLexicalChild(getParent(), this)
           : getHLNodeUtils().getLastLexicalChild(getParent(), this);

  if (FirstOrLastNode == this) {
    return nullptr;
  }

  return Prev ? &*std::prev(this->getIterator())
              : &*std::next(this->getIterator());
}

HLNode *HLNode::getPrevNode() { return getPrevNextNodeImpl(true); }

HLNode *HLNode::getNextNode() { return getPrevNextNodeImpl(false); }

// Implementation is almost the same as Instruction::extractProfMetadata
bool HLNode::extractProfileData(uint64_t &TrueVal, uint64_t &FalseVal) const {
  assert((isa<HLLoop>(this) || isa<HLIf>(this) ||
          (isa<HLInst>(this) &&
           (cast<HLInst>(this)->getLLVMInstruction()->getOpcode() ==
            Instruction::Select))) &&
         "Looking for branch weights on something besides branch");

  MDNode *ProfileData = getProfileData();
  if (!ProfileData || ProfileData->getNumOperands() != 3)
    return false;

  auto *ProfDataName = dyn_cast<MDString>(ProfileData->getOperand(0));
  if (!ProfDataName || !ProfDataName->getString().equals("branch_weights"))
    return false;

  auto *CITrue = mdconst::dyn_extract<ConstantInt>(ProfileData->getOperand(1));
  auto *CIFalse = mdconst::dyn_extract<ConstantInt>(ProfileData->getOperand(2));
  if (!CITrue || !CIFalse)
    return false;

  TrueVal = CITrue->getValue().getZExtValue();
  FalseVal = CIFalse->getValue().getZExtValue();

  return true;
}

void HLNode::setProfileData(uint64_t TrueVal, uint64_t FalseVal) {
  MDBuilder MDB(HNU.getContext());
  setProfileData(MDB.createBranchWeights(TrueVal, FalseVal));
}

bool HLNode::divideProfileData(uint64_t Denominator) {
  MDNode *ProfileData = getProfileData();
  if (!ProfileData || ProfileData->getNumOperands() < 2)
    return false;

  auto *ProfDataName = dyn_cast<MDString>(ProfileData->getOperand(0));
  if (!ProfDataName || !ProfDataName->getString().equals("branch_weights"))
    return false;

  SmallVector<Metadata *, 4> NewVals;
  MDBuilder MDB(HNU.getContext());
  NewVals.push_back(MDB.createString("branch_weights"));
  for (int I = 1, E = ProfileData->getNumOperands(); I < E; I++) {
    auto *Val = mdconst::dyn_extract<ConstantInt>(ProfileData->getOperand(I));
    // Int32Ty was used to be comparable to createBrancWeights
    NewVals.push_back(MDB.createConstant(
        ConstantInt::get(Type::getInt32Ty(HNU.getContext()),
                         // Quotient zero become 1.
                         // We avoid having 0 branch_weights.
                         // Guarding 0 branch_weighted if statemt, for example,
                         // results in hiding hight branch_weights in the
                         // guarded stmts.
             Val->getValue() == 0 ?
             0 :
             std::max(Val->getValue().getZExtValue() / Denominator,
                      static_cast<uint64_t>(1)))));
  }
  setProfileData(MDNode::get(HNU.getContext(), NewVals));

  return true;
}

void HLNode::swapProfileData() {
  setProfileData(
      HLNodeUtils::swapProfMetadata(HNU.getContext(), getProfileData()));
}

bool HLNode::isUnconditionallyExecutedinRegion(
    SmallPtrSetImpl<const HLNode *> const &IgnoreSet) const {
  assert(isAttached() && "Node expected to be in Region!\n");

  // TODO: can extend to ignore nodes using HNU.areEqualConditions
  HLNode *Node = getParent();
  while (!isa<HLRegion>(Node)) {
    if (IgnoreSet.count(Node)) {
      Node = Node->getParent();
      continue;
    }

    if (isa<HLSwitch>(Node)) {
      return false;
    }

    auto *IfNode = dyn_cast<HLIf>(Node);
    if (IfNode && !IfNode->getMVTag()) {
      return false;
    }

    if (auto *Loop = dyn_cast<HLLoop>(Node)) {
      if (Loop->hasZtt()) {
        return false;
      }
    }
    Node = Node->getParent();
  }
  return true;
}
