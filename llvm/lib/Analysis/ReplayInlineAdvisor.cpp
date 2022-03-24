//===- ReplayInlineAdvisor.cpp - Replay InlineAdvisor ---------------------===//
// INTEL_CUSTOMIZATION
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
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements ReplayInlineAdvisor that replays inline decisions based
// on previous inline remarks from optimization remark log. This is a best
// effort approach useful for testing compiler/source changes while holding
// inlining steady.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/ReplayInlineAdvisor.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>

using namespace llvm;

#define DEBUG_TYPE "replay-inline"

ReplayInlineAdvisor::ReplayInlineAdvisor(
    Module &M, FunctionAnalysisManager &FAM, LLVMContext &Context,
    std::unique_ptr<InlineAdvisor> OriginalAdvisor,
    const ReplayInlinerSettings &ReplaySettings, bool EmitRemarks)
    : InlineAdvisor(M, FAM), OriginalAdvisor(std::move(OriginalAdvisor)),
      ReplaySettings(ReplaySettings), EmitRemarks(EmitRemarks) {

  auto BufferOrErr = MemoryBuffer::getFileOrSTDIN(ReplaySettings.ReplayFile);
  std::error_code EC = BufferOrErr.getError();
  if (EC) {
    Context.emitError("Could not open remarks file: " + EC.message());
    return;
  }

  // Example for inline remarks to parse:
  //   main:3:1.1: '_Z3subii' inlined into 'main' at callsite sum:1 @
  //   main:3:1.1;
  // We use the callsite string after `at callsite` to replay inlining.
  line_iterator LineIt(*BufferOrErr.get(), /*SkipBlanks=*/true);
  const std::string PositiveRemark = "' inlined into '";
  const std::string NegativeRemark = "' will not be inlined into '";

  for (; !LineIt.is_at_eof(); ++LineIt) {
    StringRef Line = *LineIt;
    auto Pair = Line.split(" at callsite ");

    bool IsPositiveRemark = true;
    if (Pair.first.contains(NegativeRemark))
      IsPositiveRemark = false;

    auto CalleeCaller =
        Pair.first.split(IsPositiveRemark ? PositiveRemark : NegativeRemark);

    StringRef Callee = CalleeCaller.first.rsplit(": '").second;
    StringRef Caller = CalleeCaller.second.rsplit("'").first;

    auto CallSite = Pair.second.split(";").first;

    if (Callee.empty() || Caller.empty() || CallSite.empty()) {
      Context.emitError("Invalid remark format: " + Line);
      return;
    }

    std::string Combined = (Callee + CallSite).str();
    InlineSitesFromRemarks[Combined] = IsPositiveRemark;
    if (ReplaySettings.ReplayScope == ReplayInlinerSettings::Scope::Function)
      CallersToReplay.insert(Caller);
  }

  HasReplayRemarks = true;
}

std::unique_ptr<InlineAdvisor> llvm::getReplayInlineAdvisor(
    Module &M, FunctionAnalysisManager &FAM, LLVMContext &Context,
    std::unique_ptr<InlineAdvisor> OriginalAdvisor,
    const ReplayInlinerSettings &ReplaySettings, bool EmitRemarks) {
  auto Advisor = std::make_unique<ReplayInlineAdvisor>(
      M, FAM, Context, std::move(OriginalAdvisor), ReplaySettings, EmitRemarks);
  if (!Advisor->areReplayRemarksLoaded())
    Advisor.reset();
  return Advisor;
}

#if INTEL_CUSTOMIZATION
std::unique_ptr<InlineAdvice>
ReplayInlineAdvisor::getAdviceImpl(CallBase &CB, InliningLoopInfoCache *ILIC,
                                   WholeProgramInfo *WPI, InlineCost **IC) {
#endif // INTEL_CUSTOMIZATION
  assert(HasReplayRemarks);

  Function &Caller = *CB.getCaller();
  auto &ORE = FAM.getResult<OptimizationRemarkEmitterAnalysis>(Caller);

  // Decision not made by replay system
  if (!hasInlineAdvice(*CB.getFunction())) {
    // If there's a registered original advisor, return its decision
#if INTEL_CUSTOMIZATION
    if (OriginalAdvisor) {
      auto UP = OriginalAdvisor->getAdvice(CB, ILIC, WPI, IC);
      if (IC)
        *IC = UP->getInlineCost();
      return UP;
    }
#endif // INTEL_CUSTOMIZATION

    // If no decision is made above, return non-decision
    return {};
  }

  std::string CallSiteLoc =
      formatCallSiteLocation(CB.getDebugLoc(), ReplaySettings.ReplayFormat);
  StringRef Callee = CB.getCalledFunction()->getName();
  std::string Combined = (Callee + CallSiteLoc).str();

  // Replay decision, if it has one
  auto Iter = InlineSitesFromRemarks.find(Combined);
  if (Iter != InlineSitesFromRemarks.end()) {
    if (InlineSitesFromRemarks[Combined]) {
      LLVM_DEBUG(dbgs() << "Replay Inliner: Inlined " << Callee << " @ "
                        << CallSiteLoc << "\n");
#if INTEL_CUSTOMIZATION
      auto UP = std::make_unique<DefaultInlineAdvice>(
          this, CB, llvm::InlineCost::getAlways("previously inlined"), ORE,
          EmitRemarks);
      if (IC)
        *IC = UP->getInlineCost();
      return UP;
#endif // INTEL_CUSTOMIZATION
    } else {
      LLVM_DEBUG(dbgs() << "Replay Inliner: Not Inlined " << Callee << " @ "
                        << CallSiteLoc << "\n");
#if INTEL_CUSTOMIZATION
      // A negative inline is conveyed by "nothing found in replay"
      auto UP = std::make_unique<DefaultInlineAdvice>(
          this, CB, llvm::InlineCost::getNever("nothing found in replay"), ORE,
          EmitRemarks);
      if (IC)
        *IC = UP->getInlineCost();
      return UP;
#endif // INTEL_CUSTOMIZATION
    }
  }

  // Fallback decisions
#if INTEL_CUSTOMIZATION
  if (ReplaySettings.ReplayFallback ==
      ReplayInlinerSettings::Fallback::AlwaysInline) {
    auto UP = std::make_unique<DefaultInlineAdvice>(
        this, CB, llvm::InlineCost::getAlways("AlwaysInline Fallback"), ORE,
        EmitRemarks);
    if (IC)
      *IC = UP->getInlineCost();
    return UP;
  } else if (ReplaySettings.ReplayFallback ==
           ReplayInlinerSettings::Fallback::NeverInline) {
    // A negative inline is conveyed by "nothing found in replay"
    auto UP = std::make_unique<DefaultInlineAdvice>(
        this, CB, llvm::InlineCost::getNever("nothing found in replay"), ORE,
        EmitRemarks);
    if (IC)
      *IC = UP->getInlineCost();
    return UP;
  } else {
    assert(ReplaySettings.ReplayFallback ==
           ReplayInlinerSettings::Fallback::Original);
    // If there's a registered original advisor, return its decision
    if (OriginalAdvisor) {
      auto UP = OriginalAdvisor->getAdvice(CB, ILIC, WPI, IC);
      if (IC)
        *IC = UP->getInlineCost();
      return UP;
    }
  }
#endif // INTEL_CUSTOMIZATION

  // If no decision is made above, return non-decision
  return {};
}

