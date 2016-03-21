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
/// This file defines the class for transforming vector intrinsics to SVML
/// calls and scalar intrinsics to libm calls.
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

  /// \brief Gets the name of a math library function from an LLVM intrinsic
  /// name, which will be used for querying the IML accuracy interface. This
  /// function returns true if the intrinsic is properly formed and can be
  /// found as a LibFunc in TargetLibraryInfo. Properly formed intrinsics
  /// begin with "llvm." and are followed by the function name and vector
  /// type information which are separated by a '.'. E.g., llvm.sin.v4f32.
  bool parseIntrinsicName(StringRef IntrinsicName, StringRef &FuncName,
                          TargetLibraryInfo *TLI);

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
  /// Also returns the target vector length in TargetVL.
  unsigned calculateNumReturns(TargetTransformInfo *TTI, unsigned TypeBitWidth,
                               unsigned LogicalVL, unsigned *TargetVL);

  /// \brief Split the function call parameters into pseudo-registers the size
  /// corresponding to the legal target vector length.
  void splitArgs(SmallVectorImpl<Value *> &Args,
                 SmallVectorImpl<SmallVector<Value *, 8>> &NewArgs,
                 unsigned NumRet, unsigned TargetVL);

  /// \brief Build a linked list of IMF attributes used to query the IML
  /// accuracy interface.
  void createImfAttributeList(IntrinsicInst *II, ImfAttr **List);

  /// \brief Performs type legalization on parameter arguments and inserts the
  /// legally typed svml function declaration.
  FunctionType *legalizeFunctionTypes(FunctionType *FT,
                                      SmallVectorImpl<Value *> &Args,
                                      unsigned TargetVL, LibFunc::Func &LF);

  /// \brief Generates \p NumRet number of call instructions to the svml
  /// function and inserts them into \p Calls. \p Args are the arguments used
  /// for the call instructions.
  void generateSvmlCalls(unsigned NumRet, Constant *Func,
                         SmallVectorImpl<SmallVector<Value *, 8>> &Args,
                         SmallVectorImpl<Instruction *> &Calls,
                         Instruction **InsertPt);

  /// \brief For multiple svml call cases, combine all of the result vectors
  /// of the target vector length into a single vector of the logical vector
  /// length.
  Instruction *combineSvmlCallResults(unsigned NumRet,
                                      SmallVectorImpl<Instruction *> &WorkList,
                                      Instruction **InsertPt);

  /// \brief Add the AlwaysInline attribute to the function call.
  void addAlwaysInlineAttribute(CallInst *CI);

  /// \brief Finds the stride attribute for the call argument and returns
  /// the type of load/store needed for code gen. This function also returns
  /// the StringRef for the stride value through the \p AttrValStr reference.
  LoadStoreMode getLoadStoreModeForArg(AttributeSet &AS, unsigned ArgNo,
                                       StringRef &AttrValStr);

  /// \brief Generate the store for the __svml_sincos variant based on the
  /// memory reference pattern passed via argument attributes on the intrinsic
  /// call (VectorIntrin).
  void generateSinCosStore(IntrinsicInst *VectorIntrin,
                           Instruction *ResultVector, unsigned NumElemsToStore,
                           unsigned TargetVL, unsigned StorePtrIdx,
                           Instruction **InsertPt);

  /// \brief Duplicate low order elements of a smaller vector into a larger
  /// vector.
  void generateNewArgsFromPartialVectors(IntrinsicInst *II, FunctionType *FT,
                                         unsigned TargetVL,
                                         SmallVectorImpl<Value *> &NewArgs,
                                         Instruction **InsertPt);

  /// \brief Extracts NumElems from vector register \p Vector and returns an 
  /// extract instruction.
  Instruction *extractElemsFromVector(Value *Vector, unsigned StartPos,
                                      unsigned NumElems);

  /// \brief Checks if both types are vectors and if \p ValType width is less
  /// than \p LegalType width. If yes, the function returns true.
  bool isLessThanFullVector(Type *ValType, Type *LegalType);

  /// \brief Returns the largest vector type represented in the intrinsic
  /// signature.
  VectorType *getIntrinsicType(IntrinsicInst *Intrin);

  /// \brief Generate VL calls to the scalar library version of the function.
  void scalarizeVectorIntrinsic(IntrinsicInst *II, StringRef LibFuncName,
                                unsigned VL, Type *ElemType);

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
