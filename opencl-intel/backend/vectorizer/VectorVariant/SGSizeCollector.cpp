//=------------------------ SGSizeCollector.cpp -*- C++ -*-------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGSizeCollector.h"

#include "LoopHandler/CLWGBoundDecoder.h"
#include "MetadataAPI.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "SGSizeCollector"

using namespace llvm;
using namespace Intel::MetadataAPI;

bool EnableDirectFunctionCallVectorization = false;
static cl::opt<bool, true> EnableDirectFunctionCallVectorizationOpt(
    "enable-direct-function-call-vectorization", cl::location(EnableDirectFunctionCallVectorization),
    cl::Hidden,
    cl::desc(
        "Enable direct function call vectorization"));

bool EnableSubgroupDirectCallVectorization = false;
static cl::opt<bool, true> EnableSubgroupDirectCallVectorizationOpt(
    "enable-direct-subgroup-function-call-vectorization",
    cl::location(EnableSubgroupDirectCallVectorization),
    cl::Hidden, cl::desc("Enable direct subgroup function call vectorization"));

namespace intel {

char SGSizeCollector::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(SGSizeCollector, "sg-size-collector",
                          "Collecting subgroup size information", false, false)
OCL_INITIALIZE_PASS_END(SGSizeCollector, "sg-size-collector",
                        "Collecting subgroup size information", false, false)

SGSizeCollector::SGSizeCollector(const Intel::OpenCL::Utils::CPUDetect *CPUId)
    : ModulePass(ID), Impl(CPUId) {
  initializeSGSizeCollectorPass(*PassRegistry::getPassRegistry());
}

bool SGSizeCollector::runOnModule(Module &M) { return Impl.runImpl(M); }

SGSizeCollectorImpl::SGSizeCollectorImpl(
    const Intel::OpenCL::Utils::CPUDetect *CPUId)
    : CPUId(CPUId) {}

// This pass collects vector lengths from all existing functions and then
// applies it as vector-variants attribute to all directly called functions.
//
// Example:
//
//   define void @kernel() !ocl_recommended_vector_length !0 {
//   entry:
//     call void @foo()
//     ret void
//   }
//   declare void @foo()
//   !0 = !{i32 8}
//
// After the pass:
//
//   declare void @foo() #0
//   attributes #0 = { "vector-variants"="_ZGVbM8_foo,_ZGVbN8_foo" }
//
bool SGSizeCollectorImpl::runImpl(Module &M) {
  bool Modified = false;

  CallGraph CG(M);
  DenseMap<Function *, SmallSet<int, 4>> SGSizes;

  // Collect vector length information from the existing functions.
  for (auto &F : M) {
    int VecLength = 0;
    // Analyze if vectorization of direct function calls is enabled, or
    // functions has subgroup callees down the call graph as indicated
    // by the attribute.
    if (EnableDirectFunctionCallVectorization ||
        (EnableSubgroupDirectCallVectorization &&
          F.hasFnAttribute(CompilationUtils::ATTR_HAS_SUBGROUPS))) {
      if (hasVecLength(&F, VecLength)) {

        CallGraphNode *Node = CG[&F];
        for (auto It = df_begin(Node); It != df_end(Node);) {

          Function *Fn = It->getFunction();
          if (!Fn || Fn->isIntrinsic() || Fn->isDeclaration() ||
              CLWGBoundDecoder::isWGBoundFunction(Fn->getName().str())) {

            It = It.skipChildren();
            continue;
          }

          auto &Set = SGSizes[Fn];
          if (Set.count(VecLength)) {
            It = It.skipChildren();
            continue;
          }

          Set.insert(VecLength);
          It++;
        }
      }
    }
  }

  auto KernelsList = KernelList(M).getList();
  DenseSet<Function *> Kernels(KernelsList.begin(), KernelsList.end());

  // Update vector length information accordingly to the call graph.
  for (auto It : SGSizes) {
    Function *F = It.first;

    if (Kernels.find(F) != Kernels.end())
      continue;

    StringRef VarsStr;
    DenseSet<unsigned> ExistingVars;
    if (F->hasFnAttribute("vector-variants")) {

      Attribute Attr = F->getFnAttribute("vector-variants");
      VarsStr = Attr.getValueAsString();

      SmallVector<StringRef, 4> Variants;
      VarsStr.split(Variants, ",");

      for (StringRef Variant : Variants)
        ExistingVars.insert(VectorVariant(Variant).getVlen());
    }

    SmallVector<std::string, 4> Variants;
    if (!VarsStr.empty())
      Variants.push_back(VarsStr.str());

    // Create VectorVariant for each vector length.
    for (auto VecLength : It.second) {
      if (ExistingVars.find(VecLength) != ExistingVars.end())
        continue;

      std::vector<VectorKind> Parameters(F->arg_size(), VectorKind::vector());

      VectorVariant VariantMasked(getCPUIdISA(), true, VecLength, Parameters,
                                  F->getName().str(), "");
      VectorVariant VariantUnmasked(getCPUIdISA(), false, VecLength, Parameters,
                                    F->getName().str(), "");

      Variants.push_back(VariantMasked.toString());
      Variants.push_back(VariantUnmasked.toString());
    }

    F->addFnAttr("vector-variants", join(Variants, ","));
    Modified = true;
  }

  return Modified;
}

bool SGSizeCollectorImpl::hasVecLength(Function *F, int &VecLength) {
  auto KIMD = KernelInternalMetadataAPI(F);
  if (KIMD.OclRecommendedVectorLength.hasValue()) {
    VecLength = KIMD.OclRecommendedVectorLength.get();
    return VecLength > 1;
  }
  return false;
}

VectorVariant::ISAClass SGSizeCollectorImpl::getCPUIdISA() {
  if (CPUId->HasAVX512Core())
    return VectorVariant::ZMM;
  if (CPUId->HasAVX2())
    return VectorVariant::YMM2;
  if (CPUId->HasAVX1())
    return VectorVariant::YMM1;
  return VectorVariant::XMM;
}

extern "C" {
ModulePass *
createSGSizeCollectorPass(const Intel::OpenCL::Utils::CPUDetect *CPUId) {
  return new intel::SGSizeCollector(CPUId);
}
}

} // namespace intel
