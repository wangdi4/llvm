//===---- DDGraph.cpp - DD Graph implementation ---------------------------===//
//
// Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/IR/Metadata.h"

using namespace llvm;
using namespace llvm::loopopt;

DDEdge::DepType DDEdge::getEdgeType() const {
  RegDDRef *SrcRef = dyn_cast<RegDDRef>(Src);
  RegDDRef *SinkRef = dyn_cast<RegDDRef>(Sink);
  bool SrcIsLval = (SrcRef && SrcRef->isLval()) ? true : false;
  bool SinkIsLval = (SinkRef && SinkRef->isLval()) ? true : false;

  if (SrcIsLval) {
    if (SinkIsLval) {
      return DepType::OUTPUT;
    }
    return DepType::FLOW;
  } else {
    if (SinkIsLval) {
      return DepType::ANTI;
    }
    return DepType::INPUT;
  }
}

bool DDEdge::isForwardDep() const {
  auto SrcTopSortNum = getSrc()->getHLDDNode()->getTopSortNum();
  auto SinkTopSortNum = getSink()->getHLDDNode()->getTopSortNum();

  // Handle the case A[I] = A[I] + B[I]
  // Case 1: The flow edge (from Lval to Rval) is backward.
  // Case 2: The anti edge (from Rval to Lval) is forward.
  if (SrcTopSortNum == SinkTopSortNum) {
    RegDDRef *SrcRef = dyn_cast<RegDDRef>(Src);
    bool SrcIsLval = (SrcRef && SrcRef->isLval());
    return !SrcIsLval;
  }

  return (SrcTopSortNum < SinkTopSortNum);
}

static std::string getNameAndDbgLoc(DDRef *Ref) {

  std::string NameAndDbgLoc = "";

  auto getSourceName = [&](Value *Val) {
    // First try for global, trace load if ptr
    GlobalVariable *GV = nullptr;
    if (isa<GlobalVariable>(Val)) {
      GV = cast<GlobalVariable>(Val);
    } else if (auto *Load = dyn_cast<LoadInst>(Val)) {
      if (auto *LoadGV = dyn_cast<GlobalVariable>(Load->getPointerOperand())) {
        GV = LoadGV;
      }
    }

    // Get name from global debug info, or local debug metadata
    if (GV) {
      SmallVector<DIGlobalVariableExpression *, 1> GVEs;
      GV->getDebugInfo(GVEs);
      if (!GVEs.empty()) {
        NameAndDbgLoc.append(GVEs.front()->getVariable()->getName().str() +
                             " ");
      }
    } else if (Val->isUsedByMetadata()) {
      if (auto *L = LocalAsMetadata::getIfExists(Val)) {
        if (auto *MDV = MetadataAsValue::getIfExists(Val->getContext(), L)) {
          for (User *U : MDV->users()) {
            if (auto *DI = dyn_cast<DbgVariableIntrinsic>(U)) {
              auto *DbgVar = DI->getVariable();
              assert(DbgVar && "Variable is null!\n");
              NameAndDbgLoc.append(DbgVar->getName().str() + " ");
            }
          }
        }
      }
    }
  };

  if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Ref)) {
    Value *Val = BRef->getBlobUtils().getTempBlobValue(BRef->getBlobIndex());
    assert(Val && "Underlying BRef Value is null!\n");
    getSourceName(Val);

    // Add DebugLoc from parent RegDDRef
    const DebugLoc &DbgLoc = BRef->getParentDDRef()->getDebugLoc();
    if (DbgLoc.get())
      NameAndDbgLoc.append("(" + std::to_string(DbgLoc.getLine()) + ":" +
                           std::to_string(DbgLoc.getCol()) + ") ");
  } else {
    RegDDRef *RegRef = cast<RegDDRef>(Ref);
    Value *Val = nullptr;
    if (RegRef->hasGEPInfo()) {
      // Note: We only output the BaseValue, as reconstructing the
      // original ref is potentially expensive.
      Val = RegRef->getBaseValue();
    } else if (RegRef->isSelfBlob()) {
      Val = RegRef->getBlobUtils().getTempBlobValue(RegRef->getSelfBlobIndex());
    } else if (RegRef->isLval() && RegRef->isTerminalRef()) {
      auto &BU = RegRef->getBlobUtils();
      Val = BU.getTempBlobValue(BU.findTempBlobIndex(RegRef->getSymbase()));
    }

    assert(Val && "Underlying Value is null!\n");
    getSourceName(Val);

    // DbgLoc is embedded inside RegDDRef
    const DebugLoc &DbgLoc = RegRef->getDebugLoc();
    if (DbgLoc.get())
      NameAndDbgLoc.append("(" + std::to_string(DbgLoc.getLine()) + ":" +
                           std::to_string(DbgLoc.getCol()) + ") ");
  }
  return NameAndDbgLoc;
}

std::string DDEdge::getOptReportStr() const {

  std::string Str;
  raw_string_ostream OS(Str);
  OS << "assumed ";
  OS << getEdgeType();
  OS << " dependence";

  std::string SrcInfo = getNameAndDbgLoc(Src);
  std::string SinkInfo = getNameAndDbgLoc(Sink);

  if (!SrcInfo.empty() && !SinkInfo.empty())
    OS << " between " << SrcInfo << "and " << SinkInfo;
  return OS.str();
}

void DDEdge::print(raw_ostream &OS) const {
  formatted_raw_ostream FOS(OS);
  FOS << Src->getHLDDNode()->getNumber() << ":";
  FOS << Sink->getHLDDNode()->getNumber() << " ";
  Src->print(FOS);
  FOS << " --> ";
  Sink->print(FOS);
  FOS << " ";
  FOS << getEdgeType();
  FOS << " ";
  assert(DV.size() == DistVector.size());
  DV.print(FOS);
  DistVector.print(FOS);
  FOS << " \n";
  // todo
}

unsigned DDGraph::getTotalNumIncomingFlowEdges(const DDRef *Ref) const {
  unsigned Num = 0;
  for (auto &Edge : incoming(Ref)) {
    if (Edge->isFlow()) {
      Num++;
    }
  }

  const RegDDRef *RRef = dyn_cast<RegDDRef>(Ref);
  if (!RRef) {
    return Num;
  }

  for (auto &BRRef : make_range(RRef->blob_begin(), RRef->blob_end())) {
    for (auto &Edge : incoming(BRRef)) {
      assert(Edge->isFlow() &&
             "Incoming edges to blob refs should be flow edges only");
      (void)Edge;
      Num++;
    }
  }
  return Num;
}

bool DDGraph::singleEdgeGoingOut(const DDRef *LRef) {
  unsigned NumEdge = 0;

  for (auto *Edge : outgoing(LRef)) {
    (void)Edge;
    if (NumEdge++ > 1) {
      return false;
    }
  }

  return true;
}

void DDGraph::print(raw_ostream &OS) const {
  DDARefGatherer::MapTy Refs;
  DDARefGatherer::gather(CurNode, Refs);

  for (auto Pair : Refs) {
    for (DDRef *Ref : Pair.second) {
      for (DDEdge *E : outgoing(Ref)) {
        E->print(OS);
      }
    }
  }
}
