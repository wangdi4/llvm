/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __FAKE_INSERT_H__
#define __FAKE_INSERT_H__
#include "Mangler.h"
#include "Logger.h"

#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

#include <vector>

using namespace llvm;


namespace intel {


/// Fake call that mimics insert element that is used 
/// when a vectorizable built-in contains a vector argument.
class FakeInsert {
  const CallInst &Call;
  public:
  /// C-tor
  /// call - the call instruction to the fake insert
  FakeInsert(const CallInst &call):
    Call(call) {
      V_ASSERT(isFakeInsert(call) && "Not a fake insert");
      V_ASSERT(!Mangler::isMangledCall(Call.getCalledFunction()->getName()) && "Fake insert has no side effects and should not have been masked!");
  }
  bool isMasked() const             { V_ASSERT(!Mangler::isMangledCall(Call.getCalledFunction()->getName())); return Mangler::isMangledCall(Call.getCalledFunction()->getName()); }
  /// Returns the function argument which is the insertion index
  ConstantInt* getIndexArg()              { return cast<ConstantInt>(Call.getArgOperand(2)); }
  const ConstantInt* getIndexArg() const  { return cast<ConstantInt>(Call.getArgOperand(2)); }
  /// Returns the argument which is the new element inserted into the vector
  Value* getNewEltArg()             { return Call.getArgOperand(1); }
  const Value* getNewEltArg() const { return Call.getArgOperand(1); }
  /// Returns the argument which is the vector into which the new element is inserted
  const Value* getVectorArg() const { return Call.getArgOperand(0); }
  Value* getVectorArg()             { return Call.getArgOperand(0); }
  /// Returns true if call is a call to a fake insert
  static bool isFakeInsert(const CallInst &call) { return Mangler::isFakeInsert(call.getCalledFunction()->getName()); }
  /// Factory method
  /// vec - the base vector to insert element into.
  /// indConst - the constant index to mimic extract for.
  /// val - value to insert.
  /// loc - location of the new call.
  /// returns fake insert call.
  static CallInst *create(Value *vec, ConstantInt *indConst, Value *newElt, Instruction *loc) {
    SmallVector<Value *, 3> args;
    args.push_back(vec);
    args.push_back(newElt);
    args.push_back(indConst);
    SmallVector<Attributes, 4> attrs;
    attrs.push_back(Attribute::ReadNone);
    attrs.push_back(Attribute::NoUnwind);
    return VectorizerUtils::createFunctionCall(loc->getParent()->getParent()->getParent(),
      Mangler::getFakeInsertName(), vec->getType(), args, attrs, loc);
  }
};
}
#endif
