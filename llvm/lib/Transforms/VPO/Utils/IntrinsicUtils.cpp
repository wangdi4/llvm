//===----- IntrinsicUtils.cpp - General Utils for Intrinsics *- C++ -*-----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a set of utilities for intrinsic function calls.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Metadata.h"

using namespace llvm;
using namespace llvm::vpo;

StringRef VPOUtils::getDirectiveMetadataString(CallInst *Call)
{
    Function *CalledFunc = Call->getCalledFunction();
    assert(CalledFunc->isIntrinsic() &&
           CalledFunc->getIntrinsicID() == Intrinsic::intel_directive &&
           "Expected a call to the llvm.intel.directive(metadata) intrinsic");

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
