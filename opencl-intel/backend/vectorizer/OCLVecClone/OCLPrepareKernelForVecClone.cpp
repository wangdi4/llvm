//=---- OCLPrepareKernelForVecClone.cpp - Vector function to loop transform -*-
// C++
//-*----=//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
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

#include <string>
#include <vector>

using namespace llvm;
using namespace Intel::MetadataAPI;

static cl::opt<VectorVariant::ISAClass> CPUIsaEncodingOverride(
    "ocl-vec-clone-isa-encoding-override", cl::Hidden,
    cl::desc("Override target CPU ISA encoding for the OCL Vec Clone pass."),
    cl::values(clEnumValN(VectorVariant::ZMM,  "AVX512Core", "AVX512Core"),
               clEnumValN(VectorVariant::YMM2, "AVX2",       "AVX2"),
               clEnumValN(VectorVariant::YMM1, "AVX1",       "AVX1"),
               clEnumValN(VectorVariant::XMM,  "SSE42",      "SSE42")));

namespace intel {

static VectorVariant::ISAClass getCPUIdISA(
    const Intel::OpenCL::Utils::CPUDetect *CPUId) {
  assert(CPUId && "Valid CPUDetect is expected!");
  if (CPUId->HasAVX512Core())
    return VectorVariant::ZMM;
  if (CPUId->HasAVX2())
    return VectorVariant::YMM2;
  if (CPUId->HasAVX1())
    return VectorVariant::YMM1;
  return VectorVariant::XMM;
}

OCLPrepareKernelForVecClone::OCLPrepareKernelForVecClone(
    const Intel::OpenCL::Utils::CPUDetect *CPUId)
    : CPUId(CPUId) {}

OCLPrepareKernelForVecClone::OCLPrepareKernelForVecClone() {}

// Creates the encoding for the vector variants. The encoding is based on: 1.
// ISA, 2. Masked operations, 3. Vector length, 4. Parameters type.
void OCLPrepareKernelForVecClone::createEncodingForVectorVariants(
    Function *F, unsigned VecLength, const std::vector<VectorKind> &ParamAttrs,
    bool NeedMaskedVariant /*=false*/) {

  // Finds the biggest vector type supported by the target and encodes.
  VectorVariant::ISAClass ISA;

  if (CPUIsaEncodingOverride.getNumOccurrences())
    ISA = CPUIsaEncodingOverride.getValue();
  else
    ISA = getCPUIdISA(CPUId);

  assert(!F->hasFnAttribute("vector-variants") &&
         "Do not expect existing vector variants!");

  SmallVector<std::string, 4> Variants;

  VectorVariant VariantUnmasked(ISA, false, VecLength, ParamAttrs, F->getName().str(), "");
  Variants.push_back(VariantUnmasked.toString());

  if (NeedMaskedVariant) {
    VectorVariant VariantMasked(ISA, true, VecLength, ParamAttrs, F->getName().str(), "");
    Variants.push_back(VariantMasked.toString());
  }

  F->addFnAttr("vector-variants", join(Variants, ","));
}

// For each kernel, it creates vector-variant attributes which are needed to
// activate the VecClone pass.
void OCLPrepareKernelForVecClone::addVectorVariantAttrsToKernel(Function *F) {
  // The vector length is calculated by WeightedInstCounter pass. Metadata are
  // used to communicate the vector length value between the two passes.
  auto MD = KernelInternalMetadataAPI(F);
  auto KMD = KernelMetadataAPI(F);
  assert(MD.OclRecommendedVectorLength.hasValue() &&
           "Vector Length was not set!");
  unsigned VectorLength = MD.OclRecommendedVectorLength.get();

  // Use "uniform" parameter for all arguments.
  std::vector<VectorKind> ParametersKind(F->arg_size(), VectorKind::uniform());

  bool NeedMaskedVariant = false;

  // Create masked variant when there are some subgroup calls
  // in the kernel.
  // TODO: When there are some barriers in the kernel, we can also run
  // masked kernel instead of scalar kernel. In that case, we need
  // to make a heuristic choice which kernel to run. That's may be a chance
  // to improve the performance.
  if (MD.KernelHasSubgroups.hasValue() && MD.KernelHasSubgroups.get())
    NeedMaskedVariant = true;

  createEncodingForVectorVariants(F, VectorLength, ParametersKind, NeedMaskedVariant);
}

void OCLPrepareKernelForVecClone::run(Function *F) {

  addVectorVariantAttrsToKernel(F);
}
} // namespace intel
