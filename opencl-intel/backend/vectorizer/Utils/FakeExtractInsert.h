/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __FAKE_INSERT_H__
#define __FAKE_INSERT_H__
#include "Mangler.h"
#include "Logger.h"
#include "VectorizerUtils.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

using namespace llvm;


namespace intel {


/// Base class providing common functionality to derived classes
class FakeVectorOp {
protected:
  /// The call instruction
  const CallInst &Call;
  /// Index of the function argument which is the index of the scalar element.
  const unsigned IndexOfIndexArg;
  /// C-tor
  /// call - the call instruction to the fake insert
  FakeVectorOp(const CallInst &call, unsigned indexOfIndexArg):
    Call(call), IndexOfIndexArg(indexOfIndexArg) {
    V_ASSERT(!Mangler::isMangledCall(Call.getCalledFunction()->getName()) && "Fake vector op has no side effects and should not have been masked!");
  }
  public:
  /// Returns the function argument which is the insertion index
  ConstantInt* getIndexArg()              { return cast<ConstantInt>(Call.getArgOperand(IndexOfIndexArg)); }
  const ConstantInt* getIndexArg() const  { return cast<ConstantInt>(Call.getArgOperand(IndexOfIndexArg)); }
  /// Returns the argument which is the vector into which the new element is inserted
  const Value* getVectorArg() const { return Call.getArgOperand(0); }
  Value* getVectorArg()             { return Call.getArgOperand(0); }
};
/// Fake call that mimics extract element that is used
/// when a vectorizable built-in contains a vector ret value.
class FakeExtract : public FakeVectorOp {
  public:
  /// C-tor
  /// call - the call instruction to the fake insert
  FakeExtract(const CallInst &call):
    FakeVectorOp(call, 1) {
    V_ASSERT(isFakeExtract(call) && "Not a fake extract");
  }
  /// Returns true if call is a call to a fake insert
  static bool isFakeExtract(const CallInst &call) { return Mangler::isFakeExtract(call.getCalledFunction()->getName()); }
  /// Factory method
  /// vec - the base vector to insert element into.
  /// indConst - the constant index to mimic extract for.
  /// val - value to insert.
  /// insertBefore - the new call will be inserted before this instruction
  /// returns fake insert call.
  static CallInst *create(Value *vec, ConstantInt *indConst, Instruction *insertBefore) {
    SmallVector<Value *, 3> args;
    args.push_back(vec);
    args.push_back(indConst);
    SmallVector<Attribute::AttrKind, 4> attrs;
    attrs.push_back(Attribute::ReadNone);
    attrs.push_back(Attribute::NoUnwind);
    return VectorizerUtils::createFunctionCall(insertBefore->getParent()->getParent()->getParent(),
    Mangler::getFakeExtractName(), vec->getType()->getScalarType(), args, attrs, insertBefore);
  }
};
/// Fake call that mimics insert element that is used
/// when a vectorizable built-in contains a vector argument.
class FakeInsert : public FakeVectorOp {
  public:
  /// C-tor
  /// call - the call instruction to the fake insert
  FakeInsert(const CallInst &call):
    FakeVectorOp(call, 2) {
    V_ASSERT(isFakeInsert(call) && "Not a fake insert");
  }
  /// Returns the argument which is the new element inserted into the vector
  Value* getNewEltArg()             { return Call.getArgOperand(1); }
  const Value* getNewEltArg() const { return Call.getArgOperand(1); }
  /// Returns true if call is a call to a fake insert
  static bool isFakeInsert(const CallInst &call) { return Mangler::isFakeInsert(call.getCalledFunction()->getName()); }
  /// Factory method
  /// vec - the base vector to insert element into.
  /// indConst - the constant index to mimic extract for.
  /// val - value to insert.
  /// insertBefore - the new call will be inserted before this instruction
  /// returns fake insert call.
  static CallInst *create(Value *vec, ConstantInt *indConst, Value *newElt, Instruction *insertBefore) {
    SmallVector<Value *, 3> args;
    args.push_back(vec);
    args.push_back(newElt);
    args.push_back(indConst);
    SmallVector<Attribute::AttrKind, 4> attrs;
    attrs.push_back(Attribute::ReadNone);
    attrs.push_back(Attribute::NoUnwind);
    return VectorizerUtils::createFunctionCall(insertBefore->getParent()->getParent()->getParent(),
      Mangler::getFakeInsertName(), vec->getType(), args, attrs, insertBefore);
  }
};
}
#endif
