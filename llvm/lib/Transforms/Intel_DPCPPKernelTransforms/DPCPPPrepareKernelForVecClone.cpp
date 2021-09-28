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
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-prepare-kernel-for-vec-clone"

cl::opt<VectorVariant::ISAClass> IsaEncodingOverride(
    "dpcpp-vector-variant-isa-encoding-override", cl::init(VectorVariant::XMM),
    cl::Hidden,
    cl::desc("Override target CPU ISA encoding for Vector Variant passes."),
    cl::values(clEnumValN(VectorVariant::ZMM, "AVX512Core", "AVX512Core"),
               clEnumValN(VectorVariant::YMM2, "AVX2", "AVX2"),
               clEnumValN(VectorVariant::YMM1, "AVX1", "AVX1"),
               clEnumValN(VectorVariant::XMM, "SSE42", "SSE42")));

namespace llvm {

DPCPPPrepareKernelForVecClone::DPCPPPrepareKernelForVecClone(
    VectorVariant::ISAClass ISA)
    : ISA(ISA) {
  if (IsaEncodingOverride.getNumOccurrences())
    this->ISA = IsaEncodingOverride.getValue();

  LLVM_DEBUG(dbgs() << "ISAEncoding: "
                    << VectorVariant::ISAClassToString(this->ISA) << '\n';);
}

// Creates the encoding for the vector variants. The encoding is based on:
// 1. ISA, 2. Masked operations, 3. Vector length, 4. Parameters type.
void DPCPPPrepareKernelForVecClone::createEncodingForVectorVariants(
    Function &F, unsigned VF, ArrayRef<VectorKind> ParamAttrs,
    bool NeedMaskedVariant) {

  assert(!F.hasFnAttribute(KernelAttribute::VectorVariants) &&
         "Do not expect existing vector variants!");

  // Encodes masked/non-masked operations.
  SmallVector<std::string, 2> Variants;

  std::string FName = F.getName().str();
  VectorVariant Variant(ISA, false, VF, ParamAttrs, FName, "");
  Variants.push_back(Variant.toString());

  if (NeedMaskedVariant) {
    VectorVariant VariantMasked(ISA, true, VF, ParamAttrs, FName, "");
    Variants.push_back(VariantMasked.toString());
  }

  F.addFnAttr(KernelAttribute::VectorVariants, join(Variants, ","));
}

// For each kernel, it creates vector-variant attributes which are needed to
// activate the VecClone pass.
void DPCPPPrepareKernelForVecClone::addVectorVariantAttrsToKernel(Function &F) {
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(&F);
  assert(KIMD.RecommendedVL.hasValue());
  unsigned VF = KIMD.RecommendedVL.get();

  // Use "uniform" parameter for all arguments.
  std::vector<VectorKind> ParamsKind(F.arg_size(), VectorKind::uniform());

  // Create masked variant when there are some subgroup calls
  // in the kernel.
  // TODO: When there are some barriers in the kernel, we can also run
  // masked kernel instead of scalar kernel. In that case, we need
  // to make a heuristic choice which kernel to run. That's may be a chance
  // to improve the performance.
  bool NeedMaskedVariant =
      KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get();

  createEncodingForVectorVariants(F, VF, ParamsKind, NeedMaskedVariant);
}

void DPCPPPrepareKernelForVecClone::run(Function &F) {
  addVectorVariantAttrsToKernel(F);
}
} // namespace llvm
