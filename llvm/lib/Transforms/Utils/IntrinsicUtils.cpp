#if INTEL_COLLAB
//==---- IntrinsicUtils.cpp - Utilities for directives-based intrinsics ----==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Analysis/Directives.h"

#define DEBUG_TYPE "IntrinsicUtils"

using namespace llvm;

#if INTEL_CUSTOMIZATION
// Private utility function used by the functions that create the calls
// to the directive intrinsics.
Value *IntrinsicUtils::createMetadataAsValueFromString(Module &M,
                                                       StringRef Str) {
  MDString *MDStringPtr = MDString::get(M.getContext(), Str);
  MDNode *MDNodePtr = MDNode::get(M.getContext(), MDStringPtr);
  return llvm::MetadataAsValue::get(M.getContext(), MDNodePtr);
}

// This function generates calls to llvm.directive.region.entry() for SIMD and
// inserts the operand bundles of the directive clauses
CallInst *IntrinsicUtils::createSimdDirectiveBegin(
    Module &M,
    SmallDenseMap<StringRef, SmallVector<Value *, 4>> &DirectiveStrMap) {

  Function *DirIntrin =
      Intrinsic::getDeclaration(&M, Intrinsic::directive_region_entry);

  assert(
      DirIntrin &&
      "Cannot get declaration for @llvm.intel.directive(metadata) intrinsic");

  SmallVector<llvm::OperandBundleDef, 1> IntrinOpBundle;

  SmallVector<llvm::Value *, 1> OpBundleVal;
  llvm::OperandBundleDef OpBundle(
      IntrinsicUtils::getDirectiveString(DIR_OMP_SIMD), OpBundleVal);
  IntrinOpBundle.push_back(OpBundle);

  for (SmallDenseMap<StringRef, SmallVector<Value *, 4>>::iterator
           I = DirectiveStrMap.begin(),
           E = DirectiveStrMap.end();
       I != E; ++I) {
    llvm::OperandBundleDef OpBundle(I->first, I->second);
    IntrinOpBundle.push_back(OpBundle);
  }

  SmallVector<llvm::Value *, 1> Arg;

  CallInst *DirCall =
      CallInst::Create(DirIntrin, Arg, IntrinOpBundle, "entry.region");
  return DirCall;
}

// This function generates calls to llvm.directive.region.exit() for SIMD and
// inserts the operand bundles of the directive clauses
CallInst *IntrinsicUtils::createSimdDirectiveEnd(Module &M,
                                                 CallInst *EntryDirCall) {

  Function *DirIntrin =
      Intrinsic::getDeclaration(&M, Intrinsic::directive_region_exit);

  assert(
      DirIntrin &&
      "Cannot get declaration for @llvm.intel.directive(metadata) intrinsic");

  SmallVector<llvm::OperandBundleDef, 1> IntrinOpBundle;

  SmallVector<llvm::Value *, 1> OpBundleVal1;
  llvm::OperandBundleDef OpBundle1(
      IntrinsicUtils::getDirectiveString(DIR_OMP_END_SIMD), OpBundleVal1);
  IntrinOpBundle.push_back(OpBundle1);

  SmallVector<llvm::Value *, 1> Arg;
  Arg.push_back(EntryDirCall);

  CallInst *DirCall = CallInst::Create(DirIntrin, Arg, IntrinOpBundle);
  return DirCall;
}
#endif // INTEL_CUSTOMIZATION

StringRef IntrinsicUtils::getDirectiveString(int Id) {
  assert(Directives::DirectiveStrings.count(Id) &&
         "Can't find a string for directive id");
  return Directives::DirectiveStrings[Id];
}

StringRef IntrinsicUtils::getClauseString(int Id) {
  assert(Directives::ClauseStrings.count(Id) &&
         "Can't find a string for clause id");
  return Directives::ClauseStrings[Id];
}

bool IntrinsicUtils::isIntelDirective(Instruction *I) {
  if (I == nullptr)
    return false;
  IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
  if (Call) {
    Intrinsic::ID Id = Call->getIntrinsicID();
    if (Id == Intrinsic::intel_directive ||
        Id == Intrinsic::intel_directive_qual ||
        Id == Intrinsic::intel_directive_qual_opnd ||
        Id == Intrinsic::intel_directive_qual_opndlist)
      return true;

    if (Id == Intrinsic::directive_region_entry ||
        Id == Intrinsic::directive_region_exit)
      if (Call->getNumOperandBundles() > 0) {
        // First operand bundle has the directive name
        OperandBundleUse BU = Call->getOperandBundleAt(0);
        // Check if the TagName corresponds to an OpenMP directive name
        return Directives::DirectiveIDs.count(BU.getTagName());
      }
  }
  return false;
}
#endif // INTEL_COLLAB
