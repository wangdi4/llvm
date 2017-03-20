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
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"

#define DEBUG_TYPE "VPOIntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOAnalysisUtils::isRegionDirective(Intrinsic::ID Id) {
  return  Id == Intrinsic::directive_region_entry ||
          Id == Intrinsic::directive_region_exit;
}

bool VPOAnalysisUtils::isRegionDirective(Instruction *I) {
  if (I) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      return VPOAnalysisUtils::isRegionDirective(Id);
    }
  }
  return false;
}

StringRef VPOAnalysisUtils::getRegionDirectiveString(Instruction *I) {
  StringRef DirString;  // ctor initializes its data to nullptr
  if (VPOAnalysisUtils::isRegionDirective(I)) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call->getNumOperandBundles() > 0) {
      // First operand bundle has the directive name
      OperandBundleUse BU = Call->getOperandBundleAt(0);
      DirString = BU.getTagName();
    }
  }
  return DirString;
}

int VPOAnalysisUtils::getRegionDirectiveID(Instruction *I) {
  StringRef DirString = VPOAnalysisUtils::getRegionDirectiveString(I);  
  return VPOAnalysisUtils::getDirectiveID(DirString);
} 

bool VPOAnalysisUtils::isIntelDirective(Instruction *I) {
  if (I) {
    IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
    if (Call) {
      Intrinsic::ID Id = Call->getIntrinsicID();
      // Is it an intel_directive?
      if (VPOAnalysisUtils::isIntelDirective(Id))
        return true;

      // Is it an directive_region_entry/exit?
      StringRef DirString = VPOAnalysisUtils::getRegionDirectiveString(I);  
      return VPOAnalysisUtils::isOpenMPDirective(DirString);
    }
  }
  return false;
}

bool VPOAnalysisUtils::isIntelDirective(Intrinsic::ID Id) {
  return Id == Intrinsic::intel_directive;
}

bool VPOAnalysisUtils::isIntelClause(Intrinsic::ID Id) {
  return (Id == Intrinsic::intel_directive_qual ||
          Id == Intrinsic::intel_directive_qual_opnd ||
          Id == Intrinsic::intel_directive_qual_opndlist);
}

bool VPOAnalysisUtils::isIntelDirectiveOrClause(Intrinsic::ID Id) {
  return isIntelDirective(Id) || isIntelClause(Id);
}

StringRef
VPOAnalysisUtils::getDirectiveMetadataString(const IntrinsicInst *Call) {
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

StringRef
VPOAnalysisUtils::getScheduleModifierMDString(const IntrinsicInst *Call) {
  assert(isIntelDirectiveOrClause(Call->getIntrinsicID()) &&
         "Expected a call to an llvm.intel.directive* intrinsic");

  // The llvm intrin reprsenting a schedule clause has 3 arguments:
  // arg 0: Metadata string "QUAL.OMP.SCHEDULE.<SchduleKind>"
  // arg 1: Metadata string for schedule modifiers
  // arg 2: Value for the chunk size
  //
  // This util returns the string from arg 1.

  Value *Operand = Call->getArgOperand(1);

  return VPOAnalysisUtils::getScheduleModifierMDString(Operand);
}

/// \brief 'Modifier' has the MDString for the schedule modifier.
/// Extract and return the string. E.g.: "MODIFIERNONE" and "SIMD.MONOTONIC".
StringRef VPOAnalysisUtils::getScheduleModifierMDString(Value *Modifier) {

  MDString *OperandMDStr = nullptr;
  MetadataAsValue *OperandMDVal = dyn_cast<MetadataAsValue>(Modifier);
  Metadata *MD = OperandMDVal->getMetadata();

  if (isa<MDNode>(MD)) {
    MDNode *OperandNode = cast<MDNode>(MD);
    Metadata *OperandNodeMD = OperandNode->getOperand(0);
    OperandMDStr = dyn_cast<MDString>(OperandNodeMD);
  } else if (isa<MDString>(MD)) {
    OperandMDStr = cast<MDString>(MD);
  }

  assert(OperandMDStr && "Expected argument to be a metadata string");
  StringRef ModifierStr = OperandMDStr->getString();
  return ModifierStr;
}
