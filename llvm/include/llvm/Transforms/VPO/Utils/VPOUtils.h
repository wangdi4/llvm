//=======-- VPOUtils.h - Class definitions for VPO utilites -*- C++ -*-=======//
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
/// This file defines the VPOUtils class and provides a set of common utilities
/// that are shared across the Vectorizer and Parallelizer.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H
#define LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H

#include "llvm/ADT/SmallVector.h"

namespace llvm {

class Value;
class Module;
class Function;
class Type;
class LoopInfo;
class DominatorTree;
class StringRef;
class CallInst;
class Constant;
class LLVMContext;

namespace vpo {

/// \brief This class contains a set of utility functions used by VPO passes.
class VPOUtils {

private:
    /// \brief Private utility function used to encode a StringRef as a
    /// Metadata Value. This Value is then used by the createDirective*
    /// functions as the parameter representing the type of directive.
    static Value* createMetadataAsValueFromString(Module &M, StringRef Str);

public:
    /// Constructor and destructor
    VPOUtils() {}
    ~VPOUtils() {}

    enum VecFuncISA {
      UNDEF = 0, // ISA not defined
      XMM,       // Up to Nehalem
      YMM1,      // Sandy Bridge
      YMM2,      // Haswell
      ZMM,       // AVX3
      GEN,
    };

    /// \brief This function restructures the CFG on demand, where each
    /// directive for Cilk, OpenMP, Offload, Vectorization is put into a
    /// standalone basic block. This is a pre-required process for WRegion
    /// construction for each function. 
    ///
    /// Note that, since WRegion construction requires
    /// DominatorTreeWrapperPass and LoopInfoWrapperPass to be executed prior
    /// to it, when calling CFGRestructuring, we need to update DominatorTree
    /// and LoopInfo whenever a basic block splitting happens.
    static void CFGRestructuring(Function &F, DominatorTree *DT = nullptr,
                     LoopInfo *LI = nullptr);

    /// \brief Return the string representation of the metadata argument used
    /// within a call to the llvm.intel.directive intrinsic. Currently, this
    /// function only supports calls to llvm.intel.directive. Future support
    /// will be added for llvm.intel.directive.qual,
    /// llvm.intel.directive.qual.opnd and llvm.intel.directive.qual.opndlist.
    static StringRef getDirectiveMetadataString(CallInst *Call);

    /// \brief Return a call to the llvm.intel.directive intrinsic.
    static CallInst* createDirectiveCall(Module &M, StringRef DirectiveStr);

    /// \brief Return a call to the llvm.intel.directive.qual intrinsic.
    static CallInst* createDirectiveQualCall(Module &M, StringRef DirectiveStr);

    /// \brief Return a call to the llvm.intel.directive.qual.opnd intrinsic.
    static CallInst* createDirectiveQualOpndCall(Module &M,
                                                 StringRef DirectiveStr,
                                                 Value *Val);

    /// \brief Return a call to the llvm.intel.directive.qual.opndlist
    /// intrinsic.
    static CallInst* createDirectiveQualOpndListCall(
        Module &M,
        StringRef DirectiveStr,
        SmallVector<Value*, 4>& VarCallArgs);

    /// \brief Calculate the vector length based on the ISA class and scalar
    /// data type.
    static unsigned int calculateVecLen(VecFuncISA ISA, Type *DataType);

    /// \brief Determine the maximum vector register size based on the ISA
    /// class and data type.
    static unsigned int maxVecFuncISARegSize(VecFuncISA ISA, Type* DataType);

    /// \brief Determine the vector function's characteristic data type. The
    /// function passed in should correspond to the original scalar version
    /// of the function.
    static Type* calculateCharacteristicType(Function *ScalarFunc);

    /// \brief Returns a floating point or integer constant depending on Ty.
    template <typename T>
    static Constant* getConstantValue(Type *Ty, LLVMContext &Context, T Val);
};

} // End vpo namespace

} // End llvm namespace
#endif
