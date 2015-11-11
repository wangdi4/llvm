//==-- IntrinsicUtils.cpp - Utilities for VPO related intrinsics -*- C++ -*-==//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "IntrinsicUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOUtils::isIntelDirective(Intrinsic::ID Id)
{
  return Id == Intrinsic::intel_directive;
}

bool VPOUtils::isIntelClause(Intrinsic::ID Id)
{
  return (Id == Intrinsic::intel_directive_qual ||
          Id == Intrinsic::intel_directive_qual_opnd ||
          Id == Intrinsic::intel_directive_qual_opndlist);
}

bool VPOUtils::isIntelDirectiveOrClause(Intrinsic::ID Id)
{
  return isIntelDirective(Id) || isIntelClause(Id);
}

StringRef VPOUtils::getDirectiveMetadataString(IntrinsicInst *Call)
{
  assert(isIntelDirectiveOrClause(Call->getIntrinsicID())&&
         "Expected a call to an llvm.intel.directive* intrinsic");

  Value *Operand = Call->getArgOperand(0);
  MetadataAsValue *OperandMDVal = dyn_cast<MetadataAsValue>(Operand);
  MDNode *OperandNode = dyn_cast<MDNode>(OperandMDVal->getMetadata());
  assert(OperandNode && "Expected argument metadata to be an MDNode");

  Metadata *OperandNodeMD = OperandNode->getOperand(0);
  assert(OperandNodeMD->getMetadataID() == Metadata::MDStringKind &&
         "Expected argument to be a metadata string");

  MDString *OperandMDStr = dyn_cast<MDString>(OperandNodeMD);
  StringRef DirectiveStr = OperandMDStr->getString();
  return DirectiveStr;
}

// Private utility function used by the functions that create the calls
// to the directive intrinsics.
Value* VPOUtils::createMetadataAsValueFromString(Module &M, StringRef Str)
{
  MDString *MDStringPtr = MDString::get(M.getContext(), Str);
  MDNode *MDNodePtr = MDNode::get(M.getContext(), MDStringPtr);
  return llvm::MetadataAsValue::get(M.getContext(), MDNodePtr);
}

// This function generates calls to llvm.intel.directive
CallInst* VPOUtils::createDirectiveCall(Module &M, StringRef DirectiveStr)
{
  Function *DirIntrin =
      Intrinsic::getDeclaration(&M, Intrinsic::intel_directive);

  assert(DirIntrin && "Cannot get declaration for\
                       @llvm.intel.directive(metadata) intrinsic");

  Value *CallArg = createMetadataAsValueFromString(M, DirectiveStr);
  CallInst *DirCall = CallInst::Create(DirIntrin, CallArg);
  return DirCall;
}

// This function generates calls to llvm.intel.directive.qual
CallInst* VPOUtils::createDirectiveQualCall(Module &M, StringRef DirectiveStr)
{
  Function *DirQualIntrin =
      Intrinsic::getDeclaration(&M, Intrinsic::intel_directive_qual);

  assert(DirQualIntrin && "Cannot get declaration for\
                           @llvm.intel.directive.qual(metadata) intrinsic");

  Value *CallArg = createMetadataAsValueFromString(M, DirectiveStr);
  CallInst *DirQualCall = CallInst::Create(DirQualIntrin, CallArg);
  return DirQualCall;
}

// This function generates calls to llvm.intel.directive.qual.opnd
CallInst* VPOUtils::createDirectiveQualOpndCall(Module &M,
                                                StringRef DirectiveStr,
                                                Value *Val)
{
  SmallVector<Type*, 4> Tys;
  Tys.push_back(Val->getType());
  Function *DirIntrin =
      Intrinsic::getDeclaration(&M, Intrinsic::intel_directive_qual_opnd, Tys);

  assert(DirIntrin && "Cannot get declaration for\
                       @llvm.intel.directive.qual.opnd(metadata, llvm_any_ty)\
                       intrinsic");

  Value *CallArg = createMetadataAsValueFromString(M, DirectiveStr);

  SmallVector<Value*, 4> CallArgs;
  CallArgs.push_back(CallArg);
  CallArgs.push_back(Val);

  CallInst *DirCall = CallInst::Create(DirIntrin, CallArgs);
  return DirCall;
}

// This function generates calls to llvm.intel.directive.qual.opndlist
CallInst* VPOUtils::createDirectiveQualOpndListCall(
    Module &M,
    StringRef DirectiveStr,
    SmallVector<Value*, 4>& VarCallArgs)
{
  Function *DirIntrin =
      Intrinsic::getDeclaration(&M, Intrinsic::intel_directive_qual_opndlist);

  assert(DirIntrin && "Cannot get declaration for\
                       @llvm.intel.directive.qual.opndlist(metadata, ...)\
                       intrinsic");

  Value *CallArg = createMetadataAsValueFromString(M, DirectiveStr);

  SmallVector<Value*, 4> CallArgs;
  CallArgs.push_back(CallArg);
  for (unsigned I = 0; I < VarCallArgs.size(); ++I) {
    CallArgs.push_back(VarCallArgs[I]);
  }

  CallInst *DirCall = CallInst::Create(DirIntrin, CallArgs);
  return DirCall;
}

void VPOUtils::stripDirectives(Function &F)
{
  SmallVector<IntrinsicInst*, 4> IntrinsicsToRemove;

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
  for (unsigned Idx = 0; Idx < IntrinsicsToRemove.size(); ++Idx) {
    IntrinsicsToRemove[Idx]->eraseFromParent();
  }

  // SimplifyCFG will remove any blocks that become empty.
}
