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
/// Utilities to support llvm.directive.region.entry/exit intrinsics,
/// which are used to represent compiler directives in the IR.
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
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"  // INTEL

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
      "Cannot get declaration for the @llvm.directive.region.entry intrinsic");

  SmallVector<llvm::OperandBundleDef, 1> IntrinOpBundle;

  SmallVector<llvm::Value *, 1> OpBundleVal;
  llvm::OperandBundleDef OpBundle(
      std::string(IntrinsicUtils::getDirectiveString(DIR_OMP_SIMD)),
      OpBundleVal);
  IntrinOpBundle.push_back(OpBundle);

  for (SmallDenseMap<StringRef, SmallVector<Value *, 4>>::iterator
           I = DirectiveStrMap.begin(),
           E = DirectiveStrMap.end();
       I != E; ++I) {
    llvm::OperandBundleDef OpBundle(std::string(I->first), I->second);
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
      std::string(IntrinsicUtils::getDirectiveString(DIR_OMP_END_SIMD)),
      OpBundleVal1);
  IntrinOpBundle.push_back(OpBundle1);

  SmallVector<llvm::Value *, 1> Arg;
  Arg.push_back(EntryDirCall);

  CallInst *DirCall = CallInst::Create(DirIntrin, Arg, IntrinOpBundle);
  return DirCall;
}

bool IntrinsicUtils::isValueUsedBySimdPrivateClause(const Instruction *I,
                                                      const Value *V) {
  if (vpo::VPOAnalysisUtils::getDirectiveID(const_cast<Instruction *>(I)) !=
      DIR_OMP_SIMD)
    return false;
  const CallInst *CI = cast<const CallInst>(I);
  SmallVector<OperandBundleDef, 8> OpBundles;
  CI->getOperandBundlesAsDefs(OpBundles);
  return std::any_of(OpBundles.begin(), OpBundles.end(),
                     [V](OperandBundleDef &B) {
                       return vpo::VPOAnalysisUtils::getClauseID(B.getTag()) ==
                                  QUAL_OMP_PRIVATE &&
                              *B.input_begin() == V;
                     });
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

bool IntrinsicUtils::isDirective(Instruction *I) {
  if (I == nullptr)
    return false;
  IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
  if (Call) {
    Intrinsic::ID Id = Call->getIntrinsicID();
    if (Id == Intrinsic::directive_region_entry ||
        Id == Intrinsic::directive_region_exit)
      return true;
  }
  return false;
}

// Creates a clone of CI without the operand bundles matched by Predicate.
// If different, CI is replaced and the clone is returned.
CallInst *IntrinsicUtils::removeOperandBundlesFromCall(
    CallInst *CI,
    function_ref<bool(const OperandBundleDef &Bundle)> Predicate) {
  assert(CI && "removeOperandBundlesFromCall: Null CallInst");

  if (!Predicate)
    return CI;

  SmallVector<OperandBundleDef, 8> OpBundles;
  SmallVector<OperandBundleDef, 8> OpBundlesUpdated;
  CI->getOperandBundlesAsDefs(OpBundles);

  // Go over all Bundles in CI, skipping any that matched by Predicate.
  for (auto &Bundle : OpBundles) {
    if (Predicate(Bundle))
      continue;
    OpBundlesUpdated.push_back(Bundle);
  }

  if (OpBundles.size() == OpBundlesUpdated.size()) // Nothing removed
    return CI;

  SmallVector<Value *, 8> Args;
  for (auto AI = CI->arg_begin(), AE = CI->arg_end(); AI != AE; AI++)
    Args.push_back(*AI);

  auto *NewI =
      CallInst::Create(CI->getCalledValue(), Args, OpBundlesUpdated, "", CI);

  NewI->takeName(CI);
  NewI->setCallingConv(CI->getCallingConv());
  NewI->setAttributes(CI->getAttributes());
  NewI->setDebugLoc(CI->getDebugLoc());

  CI->replaceAllUsesWith(NewI);
  CI->eraseFromParent();
  return NewI;
}

#if INTEL_CUSTOMIZATION
// Creates a clone of CI without private clauses for V.
CallInst *IntrinsicUtils::removePrivateClauseForValue(CallInst *CI,
                                                      const Value *V) {
  assert(vpo::VPOAnalysisUtils::isBeginDirective(CI) && "Not a region entry directive!");
  return removeOperandBundlesFromCall(
      CI, [V](const OperandBundleDef &Bundle) {
        return vpo::VPOAnalysisUtils::getClauseID(Bundle.getTag()) ==
                   QUAL_OMP_PRIVATE &&
               *Bundle.input_begin() == V;
      });
}
#endif // INTEL_CUSTOMIZATION
#endif // INTEL_COLLAB
