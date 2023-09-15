//=== ValueProfilePlugins.inc - set of plugins used by ValueProfileCollector =//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a set of plugin classes used in ValueProfileCollectorImpl.
// Each plugin is responsible for collecting Value Profiling candidates for a
// particular optimization.
// Each plugin must satisfy the interface described in ValueProfileCollector.cpp
//
//===----------------------------------------------------------------------===//

#include "ValueProfileCollector.h"
#include "llvm/Analysis/IndirectCallVisitor.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstVisitor.h"

#if INTEL_CUSTOMIZATION
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/InstIterator.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using CandidateInfo = ValueProfileCollector::CandidateInfo;

extern cl::opt<bool> MemOPOptMemcmpBcmp;

///--------------------------- MemIntrinsicPlugin ------------------------------
class MemIntrinsicPlugin : public InstVisitor<MemIntrinsicPlugin> {
  Function &F;
  TargetLibraryInfo &TLI;
  std::vector<CandidateInfo> *Candidates;

public:
  static constexpr InstrProfValueKind Kind = IPVK_MemOPSize;

  MemIntrinsicPlugin(Function &Fn, TargetLibraryInfo &TLI)
      : F(Fn), TLI(TLI), Candidates(nullptr) {}

  void run(std::vector<CandidateInfo> &Cs) {
    Candidates = &Cs;
    visit(F);
    Candidates = nullptr;
  }
  void visitMemIntrinsic(MemIntrinsic &MI) {
    Value *Length = MI.getLength();
    // Not instrument constant length calls.
    if (isa<ConstantInt>(Length))
      return;

    Instruction *InsertPt = &MI;
    Instruction *AnnotatedInst = &MI;
    Candidates->emplace_back(CandidateInfo{Length, InsertPt, AnnotatedInst});
  }
  void visitCallInst(CallInst &CI) {
    if (!MemOPOptMemcmpBcmp)
      return;
    auto *F = CI.getCalledFunction();
    if (!F)
      return;
    LibFunc Func;
    if (TLI.getLibFunc(CI, Func) &&
        (Func == LibFunc_memcmp || Func == LibFunc_bcmp)) {
      Value *Length = CI.getArgOperand(2);
      // Not instrument constant length calls.
      if (isa<ConstantInt>(Length))
        return;
      Instruction *InsertPt = &CI;
      Instruction *AnnotatedInst = &CI;
      Candidates->emplace_back(CandidateInfo{Length, InsertPt, AnnotatedInst});
    }
  }
};

///------------------------ IndirectCallPromotionPlugin ------------------------
class IndirectCallPromotionPlugin {
  Function &F;

public:
  static constexpr InstrProfValueKind Kind = IPVK_IndirectCallTarget;

  IndirectCallPromotionPlugin(Function &Fn, TargetLibraryInfo &TLI) : F(Fn) {}

  void run(std::vector<CandidateInfo> &Candidates) {
    std::vector<CallBase *> Result = findIndirectCalls(F);
    for (Instruction *I : Result) {
      Value *Callee = cast<CallBase>(I)->getCalledOperand();
      Instruction *InsertPt = I;
      Instruction *AnnotatedInst = I;
      Candidates.emplace_back(CandidateInfo{Callee, InsertPt, AnnotatedInst});
    }
  }
};

#if INTEL_CUSTOMIZATION
///------------------------ LoopTripCountPlugin --------------------------------
class LoopTripCountPlugin {
  Function &F;

public:
  static constexpr InstrProfValueKind Kind = IPVK_LoopTripCount;

  LoopTripCountPlugin(Function &Fn, TargetLibraryInfo &TLI) : F(Fn) {}

  void run(std::vector<CandidateInfo> &Candidates) {
    struct LocalCandidateInfo {
      Instruction *V = nullptr;
      Instruction *InsertPt = nullptr;
      Instruction *AnnotatedInst = nullptr;
    };
    MapVector<uint32_t, struct LocalCandidateInfo> LocalCandidates;
    for (auto &I : instructions(F)) {
      auto *MD = I.getMetadata("intel.prof.looptc_vp");
      if (!MD)
        continue;

      uint32_t NumOps = MD->getNumOperands();
      auto *TypeCI = mdconst::dyn_extract<llvm::ConstantInt>(MD->getOperand(0));
      uint32_t TypeNum = TypeCI->getZExtValue();
      if (TypeNum == LTCP_CounterResult) {
        for (uint32_t Idx = 1; Idx < NumOps; ++Idx) {
          auto *IndexCI =
              mdconst::dyn_extract<llvm::ConstantInt>(MD->getOperand(Idx));
          assert(IndexCI && "invalid metadata");
          auto &Cand = LocalCandidates[IndexCI->getZExtValue()];
          Cand.V = &I;
          assert(isa<PHINode>(Cand.V) && "Expect value to be a PHI node");
          Cand.InsertPt = Cand.V->getParent()->getFirstNonPHIOrDbgOrLifetime();
        }
      } else if (TypeNum == LTCP_AnnotationResult) {
        for (uint32_t Idx = 1; Idx < NumOps; ++Idx) {
          auto *IndexCI =
              mdconst::dyn_extract<llvm::ConstantInt>(MD->getOperand(Idx));
          assert(IndexCI && "invalid metadata");
          auto &Cand = LocalCandidates[IndexCI->getZExtValue()];
          Cand.AnnotatedInst = &I;
        }
      } else {
        llvm_unreachable("invalid metadata");
      }
    }

    Candidates.resize(LocalCandidates.size());
    for (auto &KV : LocalCandidates) {
      assert(KV.second.AnnotatedInst && "AnnotatedInst must always exist");
      Candidates[KV.first] = {KV.second.V, KV.second.InsertPt,
                              KV.second.AnnotatedInst};
    }
  }
};
#endif // INTEL_CUSTOMIZATION

///----------------------- Registration of the plugins -------------------------
/// For now, registering a plugin with the ValueProfileCollector is done by
/// adding the plugin type to the VP_PLUGIN_LIST macro.
#if INTEL_CUSTOMIZATION
#define VP_PLUGIN_LIST           \
    MemIntrinsicPlugin,          \
    IndirectCallPromotionPlugin, \
    LoopTripCountPlugin
#endif // INTEL_CUSTOMIZATION
