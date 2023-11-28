//=------------------------ SGSizeCollector.cpp -*- C++ -*-------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_VectorVariant/SGSizeCollector.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "sycl-kernel-sg-size-collector"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

bool SYCLEnableDirectFunctionCallVectorization = true;
static cl::opt<bool, true> EnableDirectFunctionCallVectorizationOpt(
    "sycl-enable-direct-function-call-vectorization",
    cl::location(SYCLEnableDirectFunctionCallVectorization), cl::Hidden,
    cl::desc("Enable direct function call vectorization"));

bool SYCLEnableSubgroupDirectCallVectorization = true;
static cl::opt<bool, true> EnableSubgroupDirectCallVectorizationOpt(
    "sycl-enable-direct-subgroup-function-call-vectorization",
    cl::location(SYCLEnableSubgroupDirectCallVectorization), cl::Hidden,
    cl::desc("Enable direct subgroup function call vectorization"));

bool SYCLEnableVectorizationOfByvalByrefFunctions = false;
static cl::opt<bool, true> EnableVectorizationOfByvalByrefFunctionsOpt(
    "sycl-enable-byval-byref-function-call-vectorization",
    cl::location(SYCLEnableVectorizationOfByvalByrefFunctions), cl::Hidden,
    cl::desc(
        "Enable direct function call vectorization for byval/byref functions"));

static bool hasVecLength(Function *F, int &VecLength) {
  auto KIMD = KernelInternalMetadataAPI(F);
  if (KIMD.RecommendedVL.hasValue()) {
    VecLength = KIMD.RecommendedVL.get();
    return VecLength > 1;
  }
  return false;
}

// This pass collects vector lengths from all existing functions and then
// applies it as vector-variants attribute to all directly called functions.
//
// Example:
//
//   define void @kernel() !recommended_vector_length !0 {
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
bool SGSizeCollectorPass::runImpl(Module &M) {
  if (!SYCLEnableDirectFunctionCallVectorization)
    return false;
  // No need to vectorize function calls in OMP offload,
  // we are not enforced by the execution model.
  if (CompilationUtils::isGeneratedFromOMP(M))
    return false;

  bool Modified = false;

  CallGraph CG(M);
  DenseMap<Function *, SmallSet<int, 4>> SGSizes;
  auto Kernels = KernelList(M).getList();

  // Collect vector length information from kernel VFs (only kernel will have
  // RecommendedVL set).
  for (auto *Kernel : Kernels) {
    if (Kernel->hasOptNone())
      continue;
    int VecLength = 0;
    if (!hasVecLength(Kernel, VecLength))
      continue;

    CallGraphNode *Node = CG[Kernel];
    for (auto It = df_begin(Node); It != df_end(Node);) {
      Function *CalledFunc = It->getFunction();
      // Always skip the root node.
      if (CalledFunc == Kernel) {
        It++;
        continue;
      }

      bool isStructReturn = CalledFunc && CalledFunc->getReturnType()->isStructTy();
      bool hasStructReturnArg =
          CalledFunc && llvm::any_of(CalledFunc->args(), [](auto &Arg) {
            return Arg.hasStructRetAttr();
          });
      if (!CalledFunc || CalledFunc->isIntrinsic() ||
          CalledFunc->isDeclaration() ||
          CalledFunc->getName().startswith("WG.boundaries.") ||
          isStructReturn || hasStructReturnArg ||
          // Workaround for Vectorizer not supporting byval/byref
          // parameters. We can ignore the children as they won't be called
          // in vector context.
          (!EnableVectorizationOfByvalByrefFunctionsOpt &&
           CompilationUtils::hasByvalByrefArgs(CalledFunc))) {
        if (isStructReturn) {
          LLVM_DEBUG(dbgs() << "Vectorization for function with struct return "
                            << "type is not supported\n");
        }
        It = It.skipChildren();
        continue;
      }
      auto &Set = SGSizes[CalledFunc];
      if (Set.count(VecLength)) {
        It = It.skipChildren();
        continue;
      }

      Set.insert(VecLength);
      It++;
    }
  }

  // Update vector length information according to the call graph.
  for (const auto &It : SGSizes) {
    Function *F = It.first;
    StringRef VarsStr;
    DenseSet<unsigned> ExistingVars;
    if (F->hasFnAttribute(VectorUtils::VectorVariantsAttrName)) {

      Attribute Attr = F->getFnAttribute(VectorUtils::VectorVariantsAttrName);
      VarsStr = Attr.getValueAsString();

      SmallVector<StringRef, 4> Variants;
      VarsStr.split(Variants, ",");

      for (StringRef Variant : Variants)
        ExistingVars.insert(VFABI::demangleForVFABI(Variant).getVF());
    }

    SmallVector<std::string, 4> Variants;
    if (!VarsStr.empty())
      Variants.push_back(VarsStr.str());

    // Create VectorVariant for each vector length.
    for (auto VecLength : It.second) {
      if (ExistingVars.find(VecLength) != ExistingVars.end())
        continue;

      SmallVector<VFParamKind, 8> Parameters(F->arg_size(), VFParamKind::Vector);

      auto VariantMasked = VFInfo::get(ISA, true, VecLength, Parameters, F->getName().str());
      auto VariantUnmasked = VFInfo::get(ISA, false, VecLength, Parameters, F->getName().str());

      Variants.push_back(VariantMasked.VectorName);
      Variants.push_back(VariantUnmasked.VectorName);
    }

    F->addFnAttr(VectorUtils::VectorVariantsAttrName, join(Variants, ","));
    Modified = true;
  }
  return Modified;
}

PreservedAnalyses SGSizeCollectorPass::run(Module &M, ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
