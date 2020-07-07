//==--- MapIntrinToIml.h - Class definition for MapIntrinToIml -*- C++ -*---==//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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
#include "llvm/IR/IRBuilder.h"
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

class MapIntrinToImlImpl {
  /// Parent of Func.
  Module *M;

  /// Current function being processed.
  Function *Func;

  TargetTransformInfo *TTI;

  TargetLibraryInfo *TLI;

  /// New instructions are always prepended before the instruction being
  /// transformed using this IRBuilder.
  IRBuilder<> Builder;

  /// Try to find an SVML function for scalar function \p ScalarFuncName, with
  /// VL of \p TargetVL and IMF attributes attached to \p I. Returns the name
  /// of the SVML function. If there is no SVML function that satisfies the
  /// requirement, empty is returned. If \p Masked is true, a masked variant is
  /// returned.
  StringRef findX86SVMLVariantForScalarFunction(StringRef ScalarFuncName,
                                                unsigned TargetVL, bool Masked,
                                                Instruction *I);

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

  /// Combine all of the result values of the target vector length in
  /// \p SplitCalls into a single value of the logical vector length. And
  /// optionally merge with \p SourceValue if \p Mask is present. The values
  /// might be vectors (for most functions), or structures or vectors (for
  /// sincos and divrem functions).
  Value *joinSplitCallResults(unsigned NumRet, ArrayRef<Value *> SplitCalls,
                              FunctionType *FT, Value *SourceValue,
                              Value *Mask);

  /// Extract subvectors \p NewArgs from function call parameters \p Args.
  /// \p TargetVL is the size of subvector, and \p Part specifies which part to
  /// extract.
  void splitArg(ArrayRef<Value *> Args, SmallVectorImpl<Value *> &NewArgs,
                unsigned Part, unsigned TargetVL);

  /// \brief Build a linked list of IMF attributes used to query the IML
  /// accuracy interface.
  void createImfAttributeList(Instruction *I, ImfAttr **List);

  /// \brief Performs type legalization on parameter arguments and inserts the
  /// legally typed svml function declaration.
  FunctionType *legalizeFunctionTypes(FunctionType *FT, ArrayRef<Value *> Args,
                                      unsigned LogicalVL, unsigned TargetVL,
                                      StringRef FuncName);

  /// \brief Splits \p NumRet number of call instructions to the math function
  /// and inserts them into \p SplitCalls. \p Args are the arguments used for
  /// the call instructions.
  void splitMathLibCalls(unsigned NumRet, unsigned TargetVL,
                         FunctionCallee Func, ArrayRef<Value *> Args,
                         SmallVectorImpl<Value *> &SplitCalls);

  /// \brief Duplicate low order elements of a smaller vector into a larger
  /// vector.
  void generateNewArgsFromPartialVectors(ArrayRef<Value *> Args,
                                         ArrayRef<Type *> NewArgTypes,
                                         unsigned TargetVL,
                                         SmallVectorImpl<Value *> &NewArgs);

  /// Extract the lower part of \p ExtractingVL length from vector \p V which is
  /// of length SourceVL. The incoming value can be either a vector or a
  /// structure of vectors.
  Value *extractLowerPart(Value *V, unsigned ExtractingVL, unsigned SourceVL);

  /// \brief Checks if both types are vectors or struct of vectors and if \p
  /// ValType width is less than \p LegalType width. If yes, the function
  /// returns true.
  bool isLessThanFullVector(Type *ValType, Type *LegalType);

  /// Returns the largest vector type represented in the function signature.
  VectorType *getVectorTypeForSVMLFunction(FunctionType *FT);

  /// Extract information of an SVML function using it's function name and VL of
  /// its return type. Returns the scalar function name for the SVML function.
  /// The number of components is returned with out parameter \p LogicalVL (for
  /// complex functions, \p LogicalVL is half of \p ReturnVL, otherwise they are
  /// identical). If the SVML function is masked, out parameter \p Masked will
  /// be set to true.
  StringRef getSVMLFunctionProperties(StringRef FuncName, unsigned ReturnVL,
                                      unsigned &LogicalVL, bool &Masked);

  /// Generate \p LogicalVL calls to the scalar function \p ScalarFuncName.
  void scalarizeVectorCall(CallInst *CI, StringRef ScalarFuncName,
                           unsigned LogicalVL, Type *ElemType);

  /// Legalize and convert some vector integer divisions in a function to SVML
  /// call to avoid serialization in the CodeGen. Returns true if the
  /// function's IR is modified by this function.
  bool replaceVectorIDivAndRemWithSVMLCall(TargetTransformInfo *TTI,
                                           Function &F);

  /// Legalize source and mask arguments when splitting an AVX512 SVML call to
  /// a non-AVX512 one, or widening a non-AVX512 SVML call to AVX512
  void legalizeAVX512MaskArgs(CallInst *CI, SmallVectorImpl<Value *> &Args,
                              Value *MaskValue, unsigned LogicalVL,
                              unsigned TargetVL, unsigned ComponentBitWidth);

  // Create a call instruction to SVML function \p Callee with arguments
  // specified in \p Args. Set an appropriate calling convention and return the
  // newly created instruction.
  CallInst *createSVMLCall(FunctionCallee Callee, ArrayRef<Value *> Args,
                           const Twine &Name);

public:
  // Use TTI to provide information on the legal vector register size for the
  // target.
  MapIntrinToImlImpl(Function &F, TargetTransformInfo *TTI, TargetLibraryInfo *TLI);

  bool runImpl();
};

class MapIntrinToImlPass : public PassInfoMixin<MapIntrinToImlPass> {
public:
  MapIntrinToImlPass() {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

class MapIntrinToIml : public FunctionPass {
  bool runOnFunction(Function &Fn) override;

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
