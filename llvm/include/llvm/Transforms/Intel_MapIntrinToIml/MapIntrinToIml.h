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

  /// Extract subvectors \p NewArgs from function call parameters \p Args.
  /// \p TargetVL is the size of subvector, and \p Part specifies which part to
  /// extract.
  void splitArg(ArrayRef<Value *> Args, SmallVectorImpl<Value *> &NewArgs,
                unsigned Part, unsigned TargetVL);

  /// \brief Build a linked list of IMF attributes used to query the IML
  /// accuracy interface.
  void createImfAttributeList(CallInst *CI, ImfAttr **List);

  /// \brief Performs type legalization on parameter arguments and inserts the
  /// legally typed svml function declaration.
  FunctionType *legalizeFunctionTypes(FunctionType *FT, ArrayRef<Value *> Args,
                                      unsigned TargetVL, StringRef FuncName);

  /// \brief Generates \p NumRet number of call instructions to the math
  /// function and inserts them into \p Calls. \p Args are the arguments used
  /// for the call instructions.
  void generateMathLibCalls(unsigned NumRet, unsigned TargetVL,
                            FunctionCallee Func, ArrayRef<Value *> Args,
                            SmallVectorImpl<Value *> &Calls);

  /// \brief Finds the stride attribute for the call argument and returns
  /// the type of load/store needed for code gen. This function also returns
  /// the StringRef for the stride value through the \p AttrValStr reference.
  LoadStoreMode getLoadStoreModeForArg(AttributeList &AL, unsigned ArgNo,
                                       StringRef &AttrValStr);

  /// \brief Generate the store for the __svml_sincos variant based on the
  /// memory reference pattern passed via call argument attributes.
  void generateSinCosStore(CallInst *CI, Value *ResultVector,
                           unsigned NumElemsToStore, unsigned TargetVL,
                           unsigned StorePtrIdx);

  /// \brief Duplicate low order elements of a smaller vector into a larger
  /// vector.
  void generateNewArgsFromPartialVectors(ArrayRef<Value *> Args,
                                         ArrayRef<Type *> NewArgTypes,
                                         unsigned TargetVL,
                                         SmallVectorImpl<Value *> &NewArgs);

  /// \brief Extracts \p NumElems from vector register \p Vector and returns an
  /// extract instruction.
  Value *extractElemsFromVector(Value *Vector, unsigned StartPos,
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

  /// \brief Returns true if \p FuncName and \p FT refer to an SVML 3-argument
  /// sincos call.
  bool isSincosRefArg(StringRef FuncName, FunctionType *FT);

  /// Create a single instruction calling SVML for integer division. Opcode must
  /// be one of sdiv/srem/udiv/urem. V0 and V1 must have the same integer vector
  /// type, and their vector length need to be legal. Returns the newly created
  /// call instruction.
  CallInst *generateSVMLIDivOrRemCall(Instruction::BinaryOps Opcode, Value *V0,
                                      Value *V1);

  /// Legalize and convert some vector integer divisions in a function to SVML
  /// call to avoid serialization in the CodeGen. Returns true if the
  /// function's IR is modified by this function.
  bool replaceVectorIDivAndRemWithSVMLCall(TargetTransformInfo *TTI,
                                           Function &F);

  /// Legalize source and mask arguments when splitting an AVX512 SVML call to
  /// a non-AVX512 one, or widening a non-AVX512 SVML call to AVX512
  void legalizeAVX512MaskArgs(CallInst *CI, SmallVectorImpl<Value *> &Args,
                              Value *MaskValue, unsigned LogicalVL,
                              unsigned TargetVL, unsigned ScalarBitWidth);

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
