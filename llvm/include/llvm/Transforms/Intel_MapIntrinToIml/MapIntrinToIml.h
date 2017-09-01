//==--- MapIntrinToIml.h - Class definition for MapIntrinToIml -*- C++ -*---==//
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
/// This file defines the class for legalizing and applying IMF attributes to
/// SVML calls.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/iml_accuracy_interface.h"

#ifndef LLVM_TRANSFORMS_MAPINTRINTOIML_H
#define LLVM_TRANSFORMS_MAPINTRINTOIML_H

namespace llvm {

namespace vpo {

enum LoadStoreMode {
  SCALAR = 0,
  UNIT_STRIDE = 1,
  NON_UNIT_STRIDE = 2,
  INDIRECT = 3 // gather/scatter
};

class MapIntrinToIml : public FunctionPass {

private:
  /// Parent of Func.
  Module *M;

  /// Current function being processed.
  Function *Func;

  bool runOnFunction(Function &F) override;

  /// \brief For a given intrinsic \p CI, try to find an equivalent math
  /// library function to replace it with.
  const char* findX86Variant(CallInst *CI, StringRef FuncName,
                             unsigned LogicalVL, unsigned TargetVL);

  /// \brief Add an IMF attribute to the attribute list.
  void addAttributeToList(ImfAttr **List, ImfAttr **Tail, ImfAttr *Attr);

  /// \brief Delete all attributes from the IMF attribute list.
  void deleteAttributeList(ImfAttr **List);

  /// \brief Determines if a StringRef representation of an IMF attribute is
  /// legal.
  bool isValidIMFAttribute(std::string AttrName);

  /// \brief Returns the number of registers that must be used for a value
  /// with a bit width of \p TypeBitWidth and logical vector length of
  /// \p LogicalVL.
  /// Also returns the target vector length in \p TargetVL.
  unsigned calculateNumReturns(TargetTransformInfo *TTI, unsigned TypeBitWidth,
                               unsigned LogicalVL, unsigned *TargetVL);

  /// \brief Split the function call parameters into pseudo-registers the size
  /// corresponding to the legal target vector length.
  void splitArgs(SmallVectorImpl<Value *> &Args,
                 SmallVectorImpl<SmallVector<Value *, 8>> &NewArgs,
                 unsigned NumRet, unsigned TargetVL);

  /// \brief Build a linked list of IMF attributes used to query the IML
  /// accuracy interface.
  void createImfAttributeList(CallInst *CI, ImfAttr **List);

  /// \brief Performs type legalization on parameter arguments and inserts the
  /// legally typed svml function declaration.
  FunctionType *legalizeFunctionTypes(FunctionType *FT,
                                      SmallVectorImpl<Value *> &Args,
                                      unsigned TargetVL, StringRef FuncName);

  /// \brief Generates \p NumRet number of call instructions to the math
  /// function and inserts them into \p Calls. \p Args are the arguments used
  /// for the call instructions.
  void generateMathLibCalls(unsigned NumRet, Constant *Func,
                            SmallVectorImpl<SmallVector<Value *, 8>> &Args,
                            SmallVectorImpl<Instruction *> &Calls,
                            Instruction **InsertPt);

  /// \brief For multiple math library call cases, combine all of the result
  /// vectors of the target vector length into a single vector of the logical
  /// vector length.
  Instruction *combineCallResults(unsigned NumRet,
                                  SmallVectorImpl<Instruction *> &WorkList,
                                  Instruction **InsertPt);

  /// \brief Finds the stride attribute for the call argument and returns
  /// the type of load/store needed for code gen. This function also returns
  /// the StringRef for the stride value through the \p AttrValStr reference.
  LoadStoreMode getLoadStoreModeForArg(AttributeList &AL, unsigned ArgNo,
                                       StringRef &AttrValStr);

  /// \brief Generate the store for the __svml_sincos variant based on the
  /// memory reference pattern passed via call argument attributes.
  void generateSinCosStore(CallInst *CI, Instruction *ResultVector,
                           unsigned NumElemsToStore, unsigned TargetVL,
                           unsigned StorePtrIdx, Instruction **InsertPt);

  /// \brief Duplicate low order elements of a smaller vector into a larger
  /// vector.
  void generateNewArgsFromPartialVectors(CallInst *CI, FunctionType *FT,
                                         unsigned TargetVL,
                                         SmallVectorImpl<Value *> &NewArgs,
                                         Instruction **InsertPt);

  /// \brief Extracts \p NumElems from vector register \p Vector and returns an 
  /// extract instruction.
  Instruction *extractElemsFromVector(Value *Vector, unsigned StartPos,
                                      unsigned NumElems);

  /// \brief Checks if both types are vectors and if \p ValType width is less
  /// than \p LegalType width. If yes, the function returns true.
  bool isLessThanFullVector(Type *ValType, Type *LegalType);

  /// \brief Returns the largest vector type represented in the call
  /// signature.
  VectorType *getCallType(CallInst *CI);

  /// \brief Returns the scalar function name from the widened name of the
  /// vector call.
  StringRef getScalarFunctionName(StringRef FuncName, unsigned LogicalVL);

  /// \brief Generate \p LogicalVL calls to the scalar library version of the
  /// function.
  void scalarizeVectorCall(CallInst *CI, StringRef LibFuncName,
                           unsigned LogicalVL, Type *ElemType);

  /// \brief Returns true if \p FuncName refers to a sincos call.
  bool isSinCosCall(StringRef FuncName);

public:
  static char ID;

  MapIntrinToIml();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

}; // end pass class
} // end vpo namespace

FunctionPass *createMapIntrinToImlPass();
} // end llvm namespace

#endif // LLVM_TRANSFORMS_MAPINTRINTOIML_H
