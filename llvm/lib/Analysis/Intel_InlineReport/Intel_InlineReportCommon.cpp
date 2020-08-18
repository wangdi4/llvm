//===--------- Intel_InlineReportCommon.cpp - Inlining Reporti utils  ----===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements inline report utils.
//
//===----------------------------------------------------------------------===//
//
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"

using namespace llvm;

///
/// \brief Print 'indentCount' indentations
///
void llvm::printIndentCount(unsigned indentCount) {
  llvm::errs().indent(indentCount * 3);
}

StringRef llvm::getOpStr(Metadata *Node, StringRef Front) {
  assert(Node && "Empty metadata");
  MDString *StrMD = nullptr;
  MDNode *StrNode = dyn_cast<MDNode>(Node);
  if (StrNode)
    StrMD = cast<MDString>(StrNode->getOperand(0));
  else
    StrMD = cast<MDString>(Node);

  StringRef Res = StrMD->getString();
  Res.consume_front(Front);
  return Res;
}

void llvm::getOpVal(Metadata *Node, StringRef Front, int64_t *Val) {
  assert(Val && "Empty value storage");
  StringRef Res = getOpStr(Node, Front);
  assert(!Res.empty() && "Incomplete inlining report metadata");
  Res.getAsInteger(10, *Val);
}

// Print the inlining option values
void llvm::printOptionValues(unsigned OptLevel, unsigned SizeLevel) {
  InlineParams Params;
  if (!OptLevel && !SizeLevel)
    Params = llvm::getInlineParams();
  else
    Params = llvm::getInlineParams(OptLevel, SizeLevel);
  llvm::errs() << "Option Values:\n";
  llvm::errs() << "  inline-threshold: " << Params.DefaultThreshold << "\n";
  llvm::errs() << "  inlinehint-threshold: "
               << (Params.HintThreshold.hasValue()
                       ? Params.HintThreshold.getValue()
                       : 0)
               << "\n";
  llvm::errs() << "  inlinecold-threshold: "
               << (Params.ColdThreshold.hasValue()
                       ? Params.ColdThreshold.getValue()
                       : 0)
               << "\n";
  llvm::errs() << "  inlineoptsize-threshold: "
               << (Params.OptSizeThreshold.hasValue()
                       ? Params.OptSizeThreshold.getValue()
                       : 0)
               << "\n";
  llvm::errs() << "\n";
}

// Skip some llvm-specific intrinsics to make inline report shorter.
bool llvm::shouldSkipIntrinsic(IntrinsicInst *II) {
  if (!II)
    return false;
  Intrinsic::ID Intrin = II->getIntrinsicID();
  switch (Intrin) {
  default:
    return false;
  case Intrinsic::dbg_addr:
  case Intrinsic::dbg_declare:
  case Intrinsic::dbg_value:
  case Intrinsic::intel_subscript:
  case Intrinsic::lifetime_end:
  case Intrinsic::lifetime_start:
  case Intrinsic::ptr_annotation:
  case Intrinsic::var_annotation:
  case Intrinsic::assume:
  case Intrinsic::type_test:
    return true;
  }
  return false;
}
