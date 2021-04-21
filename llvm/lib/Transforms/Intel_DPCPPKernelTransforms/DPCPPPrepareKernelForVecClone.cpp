//=DPCPPPrepareKernelForVecClone.cpp - Helper for DPCPP Vec Clone*-// C++//-*=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// DPCPPPrepareKernelForVecClone class is used by DPCPPKernelVecClone pass.
/// It adds the vector-variant attributes to each kernel.
/// These atributes trigger VecClone
/// pass. For example, for the following kernel (OpenCL):
///
/// _kernel void f(_global int *A, _global int *B, _global int *C) {
///        size_t i = get_global_id(0);
///        A[i] = B[i] + C[i];
/// }
///
/// DPCPPPrepareKernelForVecClone will generate the _ZGVdN8uuu_f vector-variant
/// attribute:
/// _ZGV + d (for AVX2 ISA) + N (do not generate masks) + 8 (Vector Length) +
/// uuu (the kernel has three uniform parameters) + _ + f (name of the original
/// function).
/// NOTE: This version only vectorizes x dimension and there is no support for
/// masks.
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPPrepareKernelForVecClone.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Target/TargetMachine.h"

#include <string>

using namespace llvm;

#define DEBUG_TYPE "dpcpp-prepare-kernel-for-vec-clone"

namespace llvm {

DPCPPPrepareKernelForVecClone::DPCPPPrepareKernelForVecClone(
    Function *F, TargetTransformInfo &TTI)
    : TTI(TTI), F(F) {}

// Creates the encoding for the vector variants. The encoding is based on:
// 1. ISA, 2. Masked operations, 3. Vector length, 4. Parameters type.
void DPCPPPrepareKernelForVecClone::createEncodingForVectorVariants(
    Function *Fn, unsigned VlenVal, ArrayRef<ParamAttrTy> ParamAttrs,
    MaskTy State) {

  unsigned MaxRegWidth =
      TTI.getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector);
  // Finds the biggest vector type supported by the target and encodes.
  VectorVariant::ISAClass TargetIsaClass;

  switch (MaxRegWidth) {
  case 128:
    TargetIsaClass = VectorVariant::ISAClass::XMM;
    break;
  case 256:
    // By design it's not possible to distinguish AVX/AVX2 in compiler
    // middle end. Thus, TTI does not have a machinery to do that.
    // If it comes to the need of making such distinction, TTI can be
    // amended with Intel-specific method like getVectorISAEncodingCode,
    // yet we do not want to have a lot of highly specialized methods in TTI.
    TargetIsaClass = VectorVariant::ISAClass::YMM2;
    break;
  case 512:
    TargetIsaClass = VectorVariant::ISAClass::ZMM;
    break;
  default:
    llvm_unreachable("Invalid target vector register width");
  }

  LLVM_DEBUG(dbgs() << "ISAEncoding: "
                    << VectorVariant::ISAClassToString(TargetIsaClass)
                    << '\n';);

  // Encodes masked/non-masked operations.
  llvm::SmallVector<char, 2> Masked;
  switch (State) {
  case MT_UndefinedMask:
    Masked.push_back('N');
    Masked.push_back('M');
    break;
  case MT_NonMask:
    Masked.push_back('N');
    break;
  case MT_Mask:
    Masked.push_back('M');
    break;
  }

  std::string Buffer;
  if (Fn->hasFnAttribute("vector-variants")) {
    llvm::Attribute Attr = Fn->getFnAttribute("vector-variants");
    Buffer = Attr.getValueAsString().str();
  }
  llvm::raw_string_ostream Out(Buffer);

  for (auto Mask : Masked) {
    if (!Buffer.empty())
      Out << ',';
    Out << "_ZGV"
        << VectorVariant::encodeISAClass(TargetIsaClass)
        << Mask;

    // Encodes vector length.
    Out << VlenVal;

    // Encodes uniformity parameter.
    for (const ParamAttrTy &ParamAttr : ParamAttrs) {
      switch (ParamAttr.Kind) {
      case LinearWithVarStride:
        Out << 's' << ParamAttr.StrideOrArg;
        break;
      case Linear:
        Out << 'l';
        if (!!ParamAttr.StrideOrArg)
          Out << ParamAttr.StrideOrArg;
        break;
      case Uniform:
        Out << 'u';
        break;
      case Vector:
        Out << 'v';
        break;
      }
      if (!!ParamAttr.Alignment)
        Out << 'a' << ParamAttr.Alignment;
    }
    Out << '_' << Fn->getName();
    Out.flush();
  }

  Fn->addFnAttr("vector-variants", Out.str());
}

// For each kernel, it creates vector-variant attributes which are needed to
// activate the VecClone pass.
void DPCPPPrepareKernelForVecClone::addVectorVariantAttrsToKernel() {
  // The vector length is calculated by WeightedInstCounter pass. Metadata are
  // used to communicate the vector length value between the two passes.
  // TODO: make this more sophisticated.
  unsigned VectorLength =
      TTI.getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector) / 32;
  LLVM_DEBUG(dbgs() << "VectorLength: " << VectorLength << '\n';);

  F->addFnAttr("dpcpp_kernel_recommended_vector_length", utostr(VectorLength));

  // Use "uniform" parameter for all arguments.
  SmallVector<ParamAttrTy, 4> ParamsVec;
  for (unsigned i = 0; i < F->arg_size(); i++)
    ParamsVec.push_back(ParamAttrTy(Uniform));
  createEncodingForVectorVariants(F, VectorLength, ParamsVec, MT_NonMask);
}

void DPCPPPrepareKernelForVecClone::run() { addVectorVariantAttrsToKernel(); }
} // namespace llvm
