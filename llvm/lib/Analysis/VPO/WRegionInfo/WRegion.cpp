#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----- WRegion.cpp - Implements the WRegion class ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//   This file implements the derived classes based on WRegionNode
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Transforms/Utils/GeneralUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wregion"

#if INTEL_CUSTOMIZATION
// Local routine to print HLNodes for WRNs built from HIR
void printHIREntryExitLoop(formatted_raw_ostream &OS,
                           loopopt::HLNode *EntryHLNode,
                           loopopt::HLNode *ExitHLNode, loopopt::HLLoop *HLp,
                           unsigned Depth, unsigned Verbosity) {

  assert (EntryHLNode && "Missing Entry HLNode in WRN from HIR");
  assert (ExitHLNode  && "Missing Exit HLNode in WRN from HIR");

  OS.indent(2*Depth) << "EntryHLNode:\n";
  EntryHLNode->print(OS, 1);
  if (Verbosity > 0) {
    OS << "\n";
    OS.indent(2*Depth) << "HLLoop: ";
    if (HLp) {
      OS << "\n";
      HLp->print(OS, 1);
    } else
      OS << "none. Loop optimized away?\n";
  }
  OS << "\n";
  OS.indent(2*Depth) << "ExitHLNode:\n";
  ExitHLNode->print(OS, 1);
}
#endif // INTEL_CUSTOMIZATION

//
// Methods for WRNLoopInfo
//
Value *WRNLoopInfo::getNormIV(unsigned I) const {
  if (NormIV.size() == 0)
    return nullptr;
  assert(I < NormIV.size() && "getNormIV: bad idx");
  return NormIV[I];
}

Type *WRNLoopInfo::getNormIVElemTy(unsigned I) const {
  if (NormIVElemTy.size() == 0)
    return nullptr;
  assert(I < NormIVElemTy.size() && "getNormIVElemTy: bad idx");
  return NormIVElemTy[I];
}

Value *WRNLoopInfo::getNormUB(unsigned I) const {
  if (NormUB.size() == 0)
    return nullptr;
  assert(I < NormUB.size() && "getNormUB: bad idx");
  assert(NormUB[I] && "normalized UB must not be null.");
  return NormUB[I];
}

Type *WRNLoopInfo::getNormUBElemTy(unsigned I) const {
  if (NormUBElemTy.size() == 0)
    return nullptr;
  assert(I < NormUBElemTy.size() && "getNormUBElemTy: bad idx");
  assert(NormUBElemTy[I] && "normalized UB type must not be null.");
  return NormUBElemTy[I];
}

void WRNLoopInfo::printNormIVUB(formatted_raw_ostream &OS) const {
  if (NormIV.size() > 0) {
    OS << "  IV clause: ";
    for (unsigned I = 0; I < NormIV.size(); I++) {
      NormIV[I]->print(OS);
      OS << ", TYPED (TYPE: ";
      NormIVElemTy[I]->print(OS);
      OS << ", NUM_ELEMENTS: i32 1); ";
    }
    OS << "\n";
  }

  if (NormUB.size() > 0) {
    OS << "  UB clause: ";
    for (unsigned I = 0; I < NormUB.size(); I++) {
      NormUB[I]->print(OS);
      OS << ", TYPED (TYPE: ";
      NormUBElemTy[I]->print(OS);
      OS << ", NUM_ELEMENTS: i32 1); ";
    }
    OS << "\n";
  }
}

void WRNLoopInfo::print(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned Verbosity) const {
  int Ind = 2*Depth;
  Loop *L = getLoop();

  if (!L) {
    OS.indent(Ind) << "Loop is missing; may be optimized away.\n";
    return;
  }

  printNormIVUB(OS);

  vpo::printBB("Loop Preheader", L->getLoopPreheader(), OS, Ind, Verbosity);
  vpo::printBB("Loop Header", L->getHeader(), OS, Ind, Verbosity);
  vpo::printBB("Loop Latch", L->getLoopLatch(), OS, Ind, Verbosity);
  vpo::printBB("Loop ZTTBB", getZTTBBOrNull(), OS, Ind, Verbosity);
  OS << "\n";
}

//
// Methods for WRNParallelNode
//

// constructor
WRNParallelNode::WRNParallelNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNParallel, BB) {
  setIsPar();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  setNumWorkers(0);
  setPipelineDepth(0);
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() << ">\n");
}

// printer
void WRNParallelNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNParallelLoopNode
//

// constructor
WRNParallelLoopNode::WRNParallelLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelLoop, BB), WRNLI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(-1);
  setLoopOrder(WRNLoopOrderAbsent);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  setNumWorkers(0);
  setPipelineDepth(0);
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelLoopNode<" << getNumber()
                    << ">\n");
}

#if INTEL_CUSTOMIZATION
// constructor for HIR representation
WRNParallelLoopNode::WRNParallelLoopNode(loopopt::HLNode *EntryHLN)
    : WRegionNode(WRegionNode::WRNParallelLoop), WRNLI(nullptr),
      EntryHLNode(EntryHLN) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(-1);
  setLoopOrder(WRNLoopOrderAbsent);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  setNumWorkers(0);
  setPipelineDepth(0);
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  setExitHLNode(nullptr);
  setHLLoop(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated HIR-WRNParallelLoopNode<" << getNumber()
                    << ">\n");
}

// printer for HIR form
void WRNParallelLoopNode::printHIR(formatted_raw_ostream &OS, unsigned Depth,
                                   unsigned Verbosity) const {
  if (!getIsFromHIR()) // using LLVM-IR representation; no HIR to print
    return;

  printHIREntryExitLoop(OS, getEntryHLNode(), getExitHLNode(), getHLLoop(),
                        Depth, Verbosity);
}
#endif // INTEL_CUSTOMIZATION


// printer
void WRNParallelLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                     unsigned Verbosity) const {
  // Print the union of WRNParallel's and WRNWksLoop's extra fields
  // minus the Nowait field
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForOmpLoop(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
#if INTEL_CUSTOMIZATION
  unsigned Indent = 2 * Depth;
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION
}

//
// Methods for WRNParallelSectionsNode
//

// constructor
WRNParallelSectionsNode::WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelSections, BB), WRNLI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIsSections();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);

  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelSectionsNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNParallelSectionsNode::printExtra(formatted_raw_ostream &OS,
                              unsigned Depth, unsigned Verbosity) const {
  // identical extra fields as WRNParallel
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNParallelWorkshareNode
//

// constructor
WRNParallelWorkshareNode::WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelWorkshare, BB), WRNLI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelWorkshareNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNParallelWorkshareNode::printExtra(formatted_raw_ostream &OS,
                               unsigned Depth, unsigned Verbosity) const {
  // identical extra fields as WRNParallel
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTeamsNode
//

// constructor
WRNTeamsNode::WRNTeamsNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTeams, BB) {
  setIsTeams();
  setThreadLimit(nullptr);
  setNumTeams(nullptr);
  setDefault(WRNDefaultAbsent);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTeamsNode<" << getNumber() << ">\n");
}

// printer
void WRNTeamsNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                              unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("THREAD_LIMIT", getThreadLimit(), OS, Indent, Verbosity);
  vpo::printVal("NUM_TEAMS", getNumTeams(), OS, Indent, Verbosity);
  vpo::printStr("DEFAULT", WRNDefaultName[getDefault()], OS, Indent,
                Verbosity);
#if INTEL_CUSTOMIZATION
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION
}

//
// Methods for WRNDistributeParLoopNode
//

// constructor
WRNDistributeParLoopNode::WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistributeParLoop, BB), WRNLI(Li) {
  setIsDistribute();
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(-1);
  setLoopOrder(WRNLoopOrderAbsent);
  setTreatDistributeParLoopAsDistribute(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNDistributeParLoopNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNDistributeParLoopNode::printExtra(formatted_raw_ostream &OS,
                               unsigned Depth, unsigned Verbosity) const {
  // Similar to WRNParallelLoopNode::printExtra
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForOmpLoop(this, OS, Depth, Verbosity);
#if INTEL_CUSTOMIZATION
  unsigned Indent = 2 * Depth;
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION
}

//
// Methods for WRNTargetNode
//

// constructor
WRNTargetNode::WRNTargetNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTarget, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);
  setParLoopNdInfoAlloca(nullptr);
  setOffloadEntryIdx(-1);
  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetNode<" << getNumber() << ">\n");
}

// printer
void WRNTargetNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
#if INTEL_CUSTOMIZATION
  unsigned Indent = 2 * Depth;
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION
}

//
// Methods for WRNInteropNode
//

// constructor
WRNInteropNode::WRNInteropNode(BasicBlock* BB)
    : WRegionNode(WRegionNode::WRNInterop, BB) {
  setIsInterop();
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNInteropNode<" << getNumber() << ">\n");
}

// printer
void WRNInteropNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("DEVICE", getDevice(), OS, Indent, Verbosity);
  vpo::printBool("NOWAIT", getNowait(), OS, Indent, Verbosity);
}

//
// Methods for WRNTargetDataNode
//

// constructor
WRNTargetDataNode::WRNTargetDataNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetData, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetDataNode<" << getNumber() << ">\n");
}

// printer
void WRNTargetDataNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                   unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetEnterDataNode
//

// constructor
WRNTargetEnterDataNode::WRNTargetEnterDataNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetEnterData, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetEnterDataNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetEnterDataNode::printExtra(formatted_raw_ostream &OS,
                                        unsigned Depth,
                                        unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetExitDataNode
//

// constructor
WRNTargetExitDataNode::WRNTargetExitDataNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetExitData, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetExitDataNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetExitDataNode::printExtra(formatted_raw_ostream &OS,
                                       unsigned Depth,
                                       unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetUpdateNode
//

// constructor
WRNTargetUpdateNode::WRNTargetUpdateNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetUpdate, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetUpdateNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetUpdateNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                     unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetVariantNode
//

// constructor
WRNTargetVariantNode::WRNTargetVariantNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetVariant, BB) {
  setIsTarget();
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetVariantNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetVariantNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                      unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("DEVICE", getDevice(), OS, Indent, Verbosity);
  vpo::printBool("NOWAIT", getNowait(), OS, Indent, Verbosity);
}

//
// Methods for WRNDispatchNode
//

// constructor
WRNDispatchNode::WRNDispatchNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNDispatch, BB) {
  setDevice(nullptr);
  setNocontext(nullptr);
  setNovariants(nullptr);
  setCall(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNDispatchNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNDispatchNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                      unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("DEVICE", getDevice(), OS, Indent, Verbosity);
  vpo::printVal("NOCONTEXT", getNocontext(), OS, Indent, Verbosity);
  vpo::printVal("NOVARIANTS", getNovariants(), OS, Indent, Verbosity);
  vpo::printBool("NOWAIT", getNowait(), OS, Indent, Verbosity);
}

//
// Methods for WRNTaskNode
//

// constructor
WRNTaskNode::WRNTaskNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTask, BB) {
  setIsTask();
  setIf(nullptr);
  setFinal(nullptr);
  setPriority(nullptr);
  setDefault(WRNDefaultAbsent);
  setUntied(false);
  setMergeable(false);
  setIsTargetTask(false);
  setTaskFlag(WRNTaskFlag::Tied);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskNode<" << getNumber() << ">\n");
}

// printer
void WRNTaskNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                             unsigned Verbosity) const {
  vpo::printExtraForTask(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTaskloopNode
//

// constructor
WRNTaskloopNode::WRNTaskloopNode(BasicBlock *BB, LoopInfo *Li)
    : WRNTaskNode(BB), WRNLI(Li) {
  setWRegionKindID(WRegionNode::WRNTaskloop);
  setIsTask();
  setIsOmpLoop();
  setGrainsize(nullptr);
  setIf(nullptr);
  setNumTasks(nullptr);
  setSchedCode(0);
  setCollapse(0);
  setNogroup(false);
  setTaskFlag(WRNTaskFlag::Tied);
  // These are done in WRNTaskNode's constructor
  //   setFinal(nullptr);
  //   setPriority(nullptr);
  //   setDefault(WRNDefaultAbsent);
  //   setUntied(false);
  //   setMergeable(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskloopNode<" << getNumber() << ">\n");
}

//
// Methods for WRNVecLoopNode
//

#if INTEL_CUSTOMIZATION
// constructor for LLVM IR representation
WRNVecLoopNode::WRNVecLoopNode(BasicBlock *BB, LoopInfo *Li,
                               const bool isAutoVec)
#else
// constructor
WRNVecLoopNode::WRNVecLoopNode(BasicBlock *BB, LoopInfo *Li)
#endif // INTEL_CUSTOMIZATION
    : WRegionNode(WRegionNode::WRNVecLoop, BB), WRNLI(Li) {
  setIsOmpLoop();
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIf(nullptr);
  setLoopOrder(WRNLoopOrderAbsent);
#if INTEL_CUSTOMIZATION
  setIsAutoVec(isAutoVec);
  setHasVectorAlways(false);
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() << ">\n");
}

#if INTEL_CUSTOMIZATION
// constructor for HIR representation
WRNVecLoopNode::WRNVecLoopNode(loopopt::HLNode *EntryHLN, const bool isAutoVec)
    : WRegionNode(WRegionNode::WRNVecLoop), WRNLI(nullptr),
      EntryHLNode(EntryHLN) {
  setIsOmpLoop();
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIf(nullptr);
  setLoopOrder(WRNLoopOrderAbsent);
  setIsAutoVec(isAutoVec);
  setHasVectorAlways(false);

  setExitHLNode(nullptr);
  setHLLoop(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated HIR-WRNVecLoopNode<" << getNumber() << ">\n");
}

// printer for HIR form
void WRNVecLoopNode::printHIR(formatted_raw_ostream &OS, unsigned Depth,
                              unsigned Verbosity) const {
  if (!getIsFromHIR()) // using LLVM-IR representation; no HIR to print
    return;

  printHIREntryExitLoop(OS, getEntryHLNode(), getExitHLNode(), getHLLoop(),
                        Depth, Verbosity);
}

// Check if the HIR SIMD region associated with this WRNVecLoopNode is valid.
// A given SIMD region is valid if its BEGIN and END directive nodes belong to
// the same lexical parent as that of the associated HLLoop.
// NOTE: Since directives can be embedded in loop's preheader/postexit blocks,
// we need to validate using lexical parents and not direct parents.
bool WRNVecLoopNode::isValidHIRSIMDRegion() const {
  assert(isOmpSIMDLoop() && "Checking for SIMD region in a non-SIMD loop.");

  // If the node does not have an underlying HLLoop, then it is invalid for
  // vectorization.
  if (!getHLLoop())
    return false;

  loopopt::HLNode *LoopParent = getHLLoop()->getLexicalParent();
  loopopt::HLNode *EntryNodeParent = getEntryHLNode()->getLexicalParent();
  loopopt::HLNode *ExitNodeParent = getExitHLNode()->getLexicalParent();

  return LoopParent == EntryNodeParent && LoopParent == ExitNodeParent;
}

// Specify namespace for the template instantiation or the build will fail
namespace llvm {
namespace vpo {
template <> Loop *WRNVecLoopNode::getTheLoop<Loop>() const {
  return getWRNLoopInfo().getLoop();
}
template <>
loopopt::HLLoop *WRNVecLoopNode::getTheLoop<loopopt::HLLoop>() const {
  return getHLLoop();
}
} // vpo
} // llvm
#endif // INTEL_CUSTOMIZATION

// printer
void WRNVecLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printInt("SIMDLEN", getSimdlen(), OS, Indent, Verbosity);
  vpo::printInt("SAFELEN", getSafelen(), OS, Indent, Verbosity);
  vpo::printInt("COLLAPSE", getCollapse(), OS, Indent, Verbosity);
  vpo::printVal("IF", getIf(), OS, Indent, Verbosity);
  vpo::printStr("ORDER", WRNLoopOrderName[getLoopOrder()], OS, Indent,
                Verbosity);
#if INTEL_CUSTOMIZATION
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION

  if (getRed().empty() ||
      llvm::none_of(getRed().items(),
                    [](ReductionItem *RI) { return RI->getIsInscan(); }))
    return;

  OS.indent(Indent) << "REDUCTION-INSCAN maps: ";

  for (auto *RI : getRed().items()) {
    if (!RI->getIsInscan())
      continue;

    auto *ScanI =
        WRegionUtils::getInclusiveExclusiveItemForReductionItem(this, RI);
    OS << "(" << RI->getInscanIdx() << ": "
       << (isa<InclusiveItem>(ScanI) ? "INCLUSIVE" : "EXCLUSIVE") << ") ";
  }

  OS << "\n";
}

//
// Methods for WRNWksLoopNode
//

// constructor
WRNWksLoopNode::WRNWksLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWksLoop, BB), WRNLI(Li) {
  setIsOmpLoop();
  setCollapse(0);
  setOrdered(-1);
  setLoopOrder(WRNLoopOrderAbsent);

  LLVM_DEBUG(dbgs() << "\nCreated WRNWksLoopNode<" << getNumber() << ">\n");
}

// printer
void WRNWksLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  vpo::printExtraForOmpLoop(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
#if INTEL_CUSTOMIZATION
  unsigned Indent = 2 * Depth;
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION
}

//
// Methods for WRNSectionsNode
//

// constructor
WRNSectionsNode::WRNSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNSections, BB), WRNLI(Li) {
  setIsOmpLoop();
  setIsSections();

  LLVM_DEBUG(dbgs() << "\nCreated WRNSectionsNode<" << getNumber() << ">\n");
}

// printer
void WRNSectionsNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
//
// Methods for WRNSectionNode
//

// constructor
WRNSectionNode::WRNSectionNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNSection, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNSectionNode<" << getNumber() << ">\n");
}
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

//
// Methods for WRNWorkshareNode
//

// constructor
WRNWorkshareNode::WRNWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWorkshare, BB), WRNLI(Li) {
  setIsOmpLoop();

  LLVM_DEBUG(dbgs() << "\nCreated WRNWorkshareNode<" << getNumber() << ">\n");
}

// printer
void WRNWorkshareNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                  unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNDistributeNode
//

// constructor
WRNDistributeNode::WRNDistributeNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistribute, BB), WRNLI(Li) {
  setIsOmpLoop();
  setIsDistribute();
  setCollapse(0);

  LLVM_DEBUG(dbgs() << "\nCreated WRNDistributeNode<" << getNumber() << ">\n");
}

// printer
void WRNDistributeNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                   unsigned Verbosity) const {
  vpo::printInt("COLLAPSE", getCollapse(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNAtomicNode
//

// constructor
WRNAtomicNode::WRNAtomicNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNAtomic, BB) {
  setAtomicKind(WRNAtomicUpdate); // Default Atomic Kind is WRNAtomicUpdate
  setHasSeqCstClause(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNAtomicNode<" << getNumber() << ">\n");
}

//printer
void WRNAtomicNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printStr("ATOMIC KIND", WRNAtomicName[getAtomicKind()], OS, Indent,
                Verbosity);
  vpo::printBool("SEQ_CST", getHasSeqCstClause(), OS, Indent, Verbosity);
}

//
// Methods for WRNBarrierNode
//

// constructor
WRNBarrierNode::WRNBarrierNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNBarrier, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNBarrierNode <" << getNumber() << ">\n");
}

//
// Methods for WRNCancelNode
//

// constructor
WRNCancelNode::WRNCancelNode(BasicBlock *BB, bool IsCP)
    : WRegionNode(WRegionNode::WRNCancel, BB), IsCancellationPoint(IsCP) {
  setCancelKind(WRNCancelError);
  setIf(nullptr);
  LLVM_DEBUG(dbgs() << "\nCreated WRNCancelNode <" << getNumber() << ">\n");
}

//printer
void WRNCancelNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printBool("IS CANCELLATION POINT", getIsCancellationPoint(), OS, Indent,
                 Verbosity);
  vpo::printStr("CONSTRUCT TO CANCEL", WRNCancelName[getCancelKind()], OS,
                Indent, Verbosity);
  vpo::printVal("IF_EXPR", getIf(), OS, Indent, Verbosity);
}

//
// Methods for WRNMaskedNode
//

// constructor
WRNMaskedNode::WRNMaskedNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNMasked, BB) {
  setFilter(nullptr);
  LLVM_DEBUG(dbgs() << "\nCreated WRNMaskedNode <" << getNumber() << ">\n");
}

// printer
void WRNMaskedNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("FILTER", getFilter(), OS, Indent, Verbosity);
}

//
// Methods for WRNOrderedNode
//

// constructor
WRNOrderedNode::WRNOrderedNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNOrdered, BB) {
  setIsDoacross(false);
  setIsThreads(false);
  setIsSIMD(false);
  LLVM_DEBUG(dbgs() << "\nCreated WRNOrderedNode <" << getNumber() << ">\n");
}

//printer
void WRNOrderedNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  bool IsDoacross = getIsDoacross();
  // vpo::printBool("IS DOACROSS", IsDoacross, OS, Indent, Verbosity);

  if (!IsDoacross) { // Depend clauses present for DoAcross
    // No Depend clauses => not for DoAcross
    StringRef Kind = getIsThreads() ? "THREADS" : "SIMD";
    vpo::printStr("KIND", Kind, OS, Indent, Verbosity);
  }
}

//
// Methods for WRNSingleNode
//

// constructor
WRNSingleNode::WRNSingleNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNSingle, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNSingleNode <" << getNumber() << ">\n");
}

// printer
void WRNSingleNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNCriticalNode
//

// constructor
WRNCriticalNode::WRNCriticalNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNCritical, BB), UserLockName("") {
  setHint(0);
  // UserLockName is empty by default
  LLVM_DEBUG(dbgs() << "\nCreated WRNCriticalNode <" << getNumber() << ">\n");
}

// printer
void WRNCriticalNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  StringRef Name = UserLockName.empty() ?  "UNSPECIFIED" : getUserLockName();
  vpo::printStr("User Lock Name", Name, OS, Indent, Verbosity);
  vpo::printInt("HINT", getHint(), OS, Indent, Verbosity);
}

//
// Methods for WRNFlushNode
//

// constructor
WRNFlushNode::WRNFlushNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNFlush, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNFlushNode<" << getNumber() << ">\n");
}

//
// Methods for WRNPrefetchNode
//

// constructor
WRNPrefetchNode::WRNPrefetchNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNPrefetch, BB) {
  setIf(nullptr);
  LLVM_DEBUG(dbgs() << "\nCreated WRNPrefetchNode<" << getNumber() << ">\n");
}

// printer
void WRNPrefetchNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", getIf(), OS, Indent, Verbosity);
}

//
// Methods for WRNTaskgroupNode
//

// constructor
WRNTaskgroupNode::WRNTaskgroupNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskgroup, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskgroupNode <" << getNumber() << ">\n");
}

//
// Methods for WRNTaskwaitNode
//

// constructor
WRNTaskwaitNode::WRNTaskwaitNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskwait, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskwaitNode <" << getNumber() << ">\n");
}

//
// Methods for WRNTaskyieldNode
//

// constructor
WRNTaskyieldNode::WRNTaskyieldNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskyield, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskyieldNode <" << getNumber() << ">\n");
}

//
// Methods for WRNScopeNode
//

// constructor
WRNScopeNode::WRNScopeNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNScope, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNScopeNode<" << getNumber() << ">\n");
}

// printer
void WRNScopeNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNGuardMemMotionNode
//

// constructor
WRNGuardMemMotionNode::WRNGuardMemMotionNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNGuardMemMotion, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNGuardMemMotionNode<" << getNumber()
                    << ">\n");
}

#if INTEL_CUSTOMIZATION
// constructor for HIR
WRNGuardMemMotionNode::WRNGuardMemMotionNode(loopopt::HLNode *EntryHLN)
    : WRegionNode(WRegionNode::WRNGuardMemMotion), EntryHLNode(EntryHLN) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNGuardMemMotionNode<" << getNumber()
                    << ">\n");
}
#endif // INTEL_CUSTOMIZATION

//
// Methods for WRNTileNode
//

// constructor
WRNTileNode::WRNTileNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNTile, BB), WRNLI(Li) {
  setIsOmpLoopTransform();

  LLVM_DEBUG(dbgs() << "\nCreated WRNTileNode<" << getNumber() << ">\n");
}

//
// Methods for WRNScanNode
//

// constructor
WRNScanNode::WRNScanNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNScan, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNScanNode<" << getNumber() << ">\n");
}

// printer
void WRNScanNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                             unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  auto printReductionTypeForIdx = [&](InclusiveExclusiveItemBase *IEI) {
    uint64_t Idx = IEI->getInscanIdx();
    auto *RI =
        WRegionUtils::getReductionItemForInclusiveExclusiveItem(this, IEI);
    OS << "(" << Idx << ": " + RI->getOpName() << ") ";
  };

  if (getInclusive().empty() && getExclusive().empty())
    return;

  OS.indent(Indent) << "INSCAN-REDUCTION maps: ";
  for (auto *InI : getInclusive().items())
    printReductionTypeForIdx(InI);
  for (auto *ExI : getExclusive().items())
    printReductionTypeForIdx(ExI);
  OS << "\n";
}

//
// Methods for WRNGenericLoopNode
//

// constructor
WRNGenericLoopNode::WRNGenericLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNGenericLoop, BB), WRNLI(Li) {
  MappedDir = -1;
  setIsOmpLoop();
  setCollapse(0);

  setLoopBind(WRNLoopBindAbsent);
  setLoopOrder(WRNLoopOrderAbsent);
  LLVM_DEBUG(dbgs() << "\nCreated WRNGenericLoopNode<" << getNumber() << ">\n");
}

//
// If BIND is present:
//   BIND=parallel  ==> change to DIR_OMP_LOOP
//   BIND=teams     ==> change to DIR_OMP_DISTRIBUTE_PARLOOP
//   BIND=thread    ==> change to DIR_OMP_SIMD
//
// If BIND is absent, then we should look at the immediate parent WRN:
//   Parent=nullptr            ==> DIR_OMP_SIMD
//   Parent=Parallel           ==> DIR_OMP_LOOP
//   Parent=Teams              ==> DIR_OMP_DISTRIBUTE_PARLOOP
//   Parent=Distribute||Target ==> DIR_OMP_PARALLEL_LOOP
//   Parent=WksLoop||ParallelLoop||DistributeParLoop||Taskloop ==> DIR_OMP_SIMD
//   Parent=anything else  ==> DIR_OMP_SIMD
//
bool WRNGenericLoopNode::mapLoopScheme() {
  bool Mapped = false;
  if (getLoopBind() == WRNLoopBindParallel) {
    MappedDir = DIR_OMP_LOOP;
    Mapped = true;
  } else if (getLoopBind() == WRNLoopBindTeams) {
    MappedDir = DIR_OMP_DISTRIBUTE_PARLOOP;
    Mapped = true;
  } else if (getLoopBind() == WRNLoopBindThread) {
    MappedDir = DIR_OMP_SIMD;
    Mapped = true;
  } else {
    assert(getLoopBind() == WRNLoopBindAbsent &&
           "Unknown binding in BIND clause");
    // This is used to map loop construct to other parallelization scheme.
    // Now it's mapped to for/simd if it's in parallel region or omp for loop.
    WRegionNode *Parent = getParent();
    if (Parent == nullptr) {
      LLVM_DEBUG(dbgs() << "GenericLoop's parent WRN is nullptr\n");
      MappedDir = DIR_OMP_SIMD;
      Mapped = true;
    } else {
      LLVM_DEBUG(dbgs() << "GenericLoop's parent WRN: " << Parent->getName()
                        << "\n");
      if (Parent->getWRegionKindID() == WRegionNode::WRNParallel) {
        MappedDir = DIR_OMP_LOOP;
        Mapped = true;
      } else if (Parent->getWRegionKindID() == WRegionNode::WRNTeams) {
        MappedDir = DIR_OMP_DISTRIBUTE_PARLOOP;
        Mapped = true;
      } else if (Parent->getWRegionKindID() == WRegionNode::WRNDistribute ||
                 Parent->getWRegionKindID() == WRegionNode::WRNTarget) {
        MappedDir = DIR_OMP_PARALLEL_LOOP;
        Mapped = true;
      } else {
        MappedDir = DIR_OMP_SIMD;
        Mapped = true;
      }
    }
  }

  if (Mapped) {
    LLVM_DEBUG(dbgs() << "Mapped DIR_OMP_GENERICLOOP to "
                      << VPOAnalysisUtils::getDirectiveString(MappedDir)
                      << "\n");
    LLVM_DEBUG(dbgs() << "Binding rules: " << WRNLoopBindName[getLoopBind()]
                      << "\n");
  }

  return Mapped;
}

void WRNGenericLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                    unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printStr("LOOPBIND", WRNLoopBindName[getLoopBind()], OS, Indent,
                Verbosity);
  vpo::printStr("LOOPORDER", WRNLoopOrderName[getLoopOrder()], OS, Indent,
                Verbosity);
  vpo::printInt("COLLAPSE", getCollapse(), OS, Indent, Verbosity);
#if INTEL_CUSTOMIZATION
  vpo::printBool("EXT_DO_CONCURRENT", getIsDoConcurrent(), OS, Indent, Verbosity);
#endif // INTEL_CUSTOMIZATION
}

//
// Auxiliary print routines
//

// Print the fields common to WRNs for which getIsPar()==true.
// Possible constructs are: WRNParallel, WRNParallelLoop,
//                          WRNParallelSections, WRNParallelWorkshare,
// The fields to print are: IfExpr, NumThreads, Default, ProcBind
void vpo::printExtraForParallel(WRegionNode const *W,
                                formatted_raw_ostream &OS, int Depth,
                                unsigned Verbosity) {
  assert(W->getIsPar() &&
         "printExtraForParallel is for WRNs with getIsPar()==true");
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", W->getIf(), OS, Indent, Verbosity);
  vpo::printVal("NUM_THREADS", W->getNumThreads(), OS, Indent, Verbosity);
  vpo::printStr("DEFAULT", WRNDefaultName[W->getDefault()], OS, Indent,
                Verbosity);
  vpo::printStr("PROCBIND", WRNProcBindName[W->getProcBind()], OS, Indent,
                Verbosity);
}

void vpo::printExtraForCancellationPoints(WRegionNode const *W,
                                          formatted_raw_ostream &OS, int Depth,
                                          unsigned Verbosity) {

  if (!W->canHaveCancellationPoints())
    return;

  unsigned Indent = 2 * Depth;
  auto &CPs = W->getCancellationPoints();
  vpo::printValList("CANCELLATION.POINTS",
                    SmallVector<Value *, 8>(CPs.begin(), CPs.end()), OS, Indent,
                    Verbosity);
}

// Print the fields common to some WRNs for which getIsOmpLoop()==true.
// Possible constructs are: WRNParallelLoop, WRNDistributeParLoop, WRNWksLoop
// The fields to print are: Collapse, Ordered, Order, Nowait
void vpo::printExtraForOmpLoop(WRegionNode const *W, formatted_raw_ostream &OS,
                               int Depth, unsigned Verbosity) {
  assert(W->getIsOmpLoop() &&
         "printExtraForOmpLoop is for WRNs with getIsOmpLoop()==true");
  unsigned Indent = 2 * Depth;
  vpo::printInt("COLLAPSE", W->getCollapse(), OS, Indent, Verbosity);
  if (W->getOrdered() > 0) {
    vpo::printInt("ORDERED(N)", W->getOrdered(), OS, Indent, Verbosity);
    vpo::printValList("ORDERED(N) Trip Counts", W->getOrderedTripCounts(), OS, Indent,
                      Verbosity);
  } else
    vpo::printBool("ORDERED", W->getOrdered() == 0, OS, Indent, Verbosity);
  vpo::printStr("ORDER", WRNLoopOrderName[W->getLoopOrder()], OS, Indent,
                Verbosity);

  // WRNs with getIsPar()==true don't have the Nowait clause
  if (!(W->getIsPar()))
    vpo::printBool("NOWAIT", W->getNowait(), OS, Indent, Verbosity);
}

// Print the fields common to WRNs for which getIsTarget()==true.
// Possible constructs are: WRNTarget, WRNTargetData, WRNTargetUpdate
// The fields to print are: IfExpr, Device, Nowait
// Additionally, for WRNTarget also print the Defaultmap clause
void vpo::printExtraForTarget(WRegionNode const *W, formatted_raw_ostream &OS,
                              int Depth, unsigned Verbosity) {
  assert(W->getIsTarget() &&
         "printExtraForTarget is for WRNs with getIsTarget()==true");
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", W->getIf(), OS, Indent, Verbosity);
  vpo::printVal("DEVICE", W->getDevice(), OS, Indent, Verbosity);

  // All target constructs but WRNTargetData can have the NOWAIT clause
  if (!isa<WRNTargetDataNode>(W))
    vpo::printBool("NOWAIT", W->getNowait(), OS, Indent, Verbosity);

  // Only WRNTarget can have the defaultmap(<behavior> [:<category]) clause
  if (isa<WRNTargetNode>(W)) {
    bool Printed = false;
    auto printDefaultmapBehaviorForCategory =
        [&Printed, &W, &OS, &Indent](WRNDefaultmapCategory Category) {
          WRNDefaultmapBehavior Behavior = W->getDefaultmap(Category);
          if (Behavior != WRNDefaultmapAbsent) {
            StringRef BehaviorName = WRNDefaultmapBehaviorName[Behavior];
            StringRef CategoryName = WRNDefaultmapCategoryName[Category];
            OS.indent(Indent) << "DEFAULTMAP: " << BehaviorName << " : "
                              << CategoryName << "\n";
            Printed = true;
          }
        };

    printDefaultmapBehaviorForCategory(WRNDefaultmapAggregate);
#if INTEL_CUSTOMIZATION
    printDefaultmapBehaviorForCategory(WRNDefaultmapAllocatable);
#endif // INTEL_CUSTOMIZATION
    printDefaultmapBehaviorForCategory(WRNDefaultmapPointer);
    printDefaultmapBehaviorForCategory(WRNDefaultmapScalar);

    if (!Printed) {
      // if there's no defaulmap with category specified,
      // then handle defaultmap without category, which applies to all vars.
      WRNDefaultmapBehavior Behavior = W->getDefaultmap(WRNDefaultmapAllVars);
      StringRef BehaviorName = WRNDefaultmapBehaviorName[Behavior];
      vpo::printStr("DEFAULTMAP", BehaviorName, OS, Indent, Verbosity);
    }

    int EntryIdx = W->getOffloadEntryIdx();
    vpo::printInt("OFFLOAD_ENTRY_IDX", EntryIdx, OS, Indent, Verbosity, 0);
  }
}

// Print the fields common to WRNs for which getIsTask()==true.
// Possible constructs are: WRNTask, WRNTaskloop
// The fields to print are:
//          IfExpr, Default, Final, Priority, Untied, Mergeable
// Additionally, for WRNTaskloop also print these:
//          Grainsize, NumTasks, Collapse, Nogroup
void vpo::printExtraForTask(WRegionNode const *W, formatted_raw_ostream &OS,
                            int Depth, unsigned Verbosity) {
  assert(W->getIsTask() &&
         "printExtraForTarget is for WRNs with getIsTask()==true");
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", W->getIf(), OS, Indent, Verbosity);
  vpo::printStr("DEFAULT", WRNDefaultName[W->getDefault()], OS, Indent);
  vpo::printVal("FINAL", W->getFinal(), OS, Indent, Verbosity);
  vpo::printVal("PRIORITY", W->getPriority(), OS, Indent, Verbosity);
  vpo::printBool("UNTIED", W->getUntied(), OS, Indent, Verbosity);
  vpo::printBool("TARGET_TASK", W->getIsTargetTask(), OS, Indent, Verbosity);
  vpo::printBool("MERGEABLE", W->getMergeable(), OS, Indent, Verbosity);

  // WRNTaskloop has a few more additional fields to print
  if (isa<WRNTaskloopNode>(W)) {
    vpo::printVal((W->getIsStrict() && W->getGrainsize()) ? "GRAINSIZE:STRICT"
                                                          : "GRAINSIZE",
                  W->getGrainsize(), OS, Indent, Verbosity);
    vpo::printVal((W->getIsStrict() && W->getNumTasks()) ? "NUM_TASKS:STRICT"
                                                         : "NUM_TASKS",
                  W->getNumTasks(), OS, Indent, Verbosity);
    vpo::printInt("COLLAPSE", W->getCollapse(), OS, Indent, Verbosity);
    vpo::printBool("NOGROUP", W->getNogroup(), OS, Indent, Verbosity);
  }
}
#endif // INTEL_COLLAB
