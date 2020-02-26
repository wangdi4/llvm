//=---- OCLPrepareKernelForVecClone.cpp - Vector function to loop transform -*-
// C++
//-*----=//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// OCLPrepareKernelForVecClone class is used by OCLVecClone pass. It adds the
/// vector-variant attributes to each kernel. These atributes trigger VecClone
/// pass. For example, for the following kernel:
///
/// _kernel void f(_global int *A, _global int *B, _global int *C) {
///        size_t i = get_global_id(0);
///        A[i] = B[i] + C[i];
/// }
///
/// OCLPrepareKernelForVecClone will generate the _ZGVdN8uuu_f vector-variant
/// attribute:
/// _ZGV + d (for AVX2 ISA) + N (do not generate masks) + 8 (Vector Length) +
/// uuu (the kernel has three uniform parameters) + _ + f (name of the original
/// function).
/// NOTE: This version only vectorizes x dimension and there is not support for
/// masks.
// ===--------------------------------------------------------------------=== //
#include "OCLPrepareKernelForVecClone.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "TargetArch.h"

#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;
using namespace Intel::MetadataAPI;

enum IsaEncodingValue {
  AVX512Core = 'e',
  AVX2 = 'd',
  AVX1 = 'c',
  SSE42 = 'b'
};

static cl::opt<IsaEncodingValue> CPUIsaEncodingOverride(
    "ocl-vec-clone-isa-encoding-override", cl::Hidden,
    cl::desc("Override target CPU ISA encoding for the OCL Vec Clone pass."),
    cl::values(clEnumVal(AVX512Core, "AVX512Core"), clEnumVal(AVX2, "AVX2"),
               clEnumVal(AVX1, "AVX1"), clEnumVal(SSE42, "SSE42")));

namespace intel {

OCLPrepareKernelForVecClone::OCLPrepareKernelForVecClone(
    const Intel::CPUId *CPUId)
    : CPUId(CPUId) {}

OCLPrepareKernelForVecClone::OCLPrepareKernelForVecClone() {}

// Creates the encoding for the vector variants. The encoding is based on: 1.
// ISA, 2. Masked operations, 3. Vector length, 4. Parameters type.
void OCLPrepareKernelForVecClone::createEncodingForVectorVariants(
    Function *Fn, unsigned VlenVal, ArrayRef<ParamAttrTy> ParamAttrs,
    MaskTy State) {

  // Finds the biggest vector type supported by the target and encodes.
  char ISAEncoding = 0;

  if (CPUIsaEncodingOverride.getNumOccurrences())
    ISAEncoding = static_cast<char>(CPUIsaEncodingOverride.getValue());
  else if (CPUId->HasAVX512Core())
    ISAEncoding = 'e';
  else if (CPUId->HasAVX2())
    ISAEncoding = 'd';
  else if (CPUId->HasAVX1())
    ISAEncoding = 'c';
  else if (CPUId->HasSSE2())
    ISAEncoding = 'b';
  else if (CPUId->HasSSE41())
    ISAEncoding = 'b';
  else if (CPUId->HasSSE42())
    ISAEncoding = 'b';
  else
    llvm_unreachable("Missing vector type!");

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

#if INTEL_CUSTOMIZATION
  std::string Buffer;
  if (Fn->hasFnAttribute("vector-variants")) {
    llvm::Attribute Attr = Fn->getFnAttribute("vector-variants");
    Buffer = Attr.getValueAsString().str();
  }
  llvm::raw_string_ostream Out(Buffer);

  for (auto Mask : Masked) {
    if (!Buffer.empty())
      Out << ",";
#endif // INTEL_CUSTOMIZATION
    Out << "_ZGV" << ISAEncoding << Mask;

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
    Out.flush(); // INTEL
  }

  Fn->addFnAttr("vector-variants", Out.str()); // INTEL
}

// For each kernel, it creates vector-variant attributes which are needed to
// activate the VecClone pass.
void OCLPrepareKernelForVecClone::addVectorVariantAttrsToKernel(Function *F) {
  // The vector length is calculated by WeightedInstCounter pass. Metadata are
  // used to communicate the vector length value between the two passes.
  auto MD = KernelInternalMetadataAPI(F);
  auto KMD = KernelMetadataAPI(F);
  V_ASSERT(MD.OclRecommendedVectorLength.hasValue() &&
           "Vector Length was not set!");
  unsigned VectorLength;
  if (KMD.hasVecLength()) // Check for forced vector length
    VectorLength = KMD.getVecLength();
  else
    VectorLength = MD.OclRecommendedVectorLength.get();

  // Use "uniform" parameter for all arguments.
  SmallVector<ParamAttrTy, 3> ParamsVec;
  for (unsigned i = 0; i < F->arg_size(); i++)
    ParamsVec.push_back(ParamAttrTy(Uniform));
  createEncodingForVectorVariants(F, VectorLength, ParamsVec, MT_NonMask);

  // Create masked variant when there are some subgroup calls
  // in the kernel.
  // TODO: When there are some barriers in the kernel, we can also run
  // masked kernel instead of scalar kernel. In that case, we need
  // to make a heuristic choice which kernel to run. That's may be a chance
  // to improve the performance.
  if (MD.KernelHasSubgroups.hasValue() && MD.KernelHasSubgroups.get())
    createEncodingForVectorVariants(F, VectorLength, ParamsVec, MT_Mask);
}

void OCLPrepareKernelForVecClone::run(Function *F) {

  addVectorVariantAttrsToKernel(F);
}
} // namespace intel
