//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities for VPO-based intrinsic function
/// calls. E.g., directives that mark the beginning and end of SIMD and
/// parallel regions.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

#define DEBUG_TYPE "IntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOUtils::isIntelDirective(Intrinsic::ID Id) {
  return Id == Intrinsic::intel_directive;
}

bool VPOUtils::isIntelClause(Intrinsic::ID Id) {
  return (Id == Intrinsic::intel_directive_qual ||
          Id == Intrinsic::intel_directive_qual_opnd ||
          Id == Intrinsic::intel_directive_qual_opndlist);
}

bool VPOUtils::isIntelDirectiveOrClause(Intrinsic::ID Id) {
  return isIntelDirective(Id) || isIntelClause(Id);
}

StringRef VPOUtils::getDirectiveMetadataString(IntrinsicInst *Call) {
  assert(isIntelDirectiveOrClause(Call->getIntrinsicID()) &&
         "Expected a call to an llvm.intel.directive* intrinsic");

  MDString *OperandMDStr = nullptr;
  Value *Operand = Call->getArgOperand(0);
  MetadataAsValue *OperandMDVal = dyn_cast<MetadataAsValue>(Operand);
  Metadata *MD = OperandMDVal->getMetadata();

  if (isa<MDNode>(MD)) {
    MDNode *OperandNode = cast<MDNode>(MD);
    Metadata *OperandNodeMD = OperandNode->getOperand(0);
    OperandMDStr = dyn_cast<MDString>(OperandNodeMD);
  } else if (isa<MDString>(MD)) {
    OperandMDStr = cast<MDString>(MD);
  }

  assert(OperandMDStr && "Expected argument to be a metadata string");
  StringRef DirectiveStr = OperandMDStr->getString();

  return DirectiveStr;
}

bool VPOUtils::stripDirectives(Function &F) {
  SmallVector<IntrinsicInst *, 4> IntrinsicsToRemove;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    IntrinsicInst *IntrinCall = dyn_cast<IntrinsicInst>(&*I);
    if (IntrinCall) {
      Intrinsic::ID Id = IntrinCall->getIntrinsicID();
      if (isIntelDirectiveOrClause(Id)) {
        IntrinsicsToRemove.push_back(IntrinCall);
      }
    }
  }

  // Remove the directive intrinsics.
  unsigned Idx = 0;
  for (Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    IntrinsicsToRemove[Idx]->eraseFromParent();
  }

  // SimplifyCFG will remove any blocks that become empty.

  // Returns true if any elimination happens.
  return Idx > 0;
}
