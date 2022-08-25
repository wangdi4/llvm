//=------------------------ SGSizeCollector.cpp -*- C++ -*-------------------=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/SGSizeCollector.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "dpcpp-kernel-sg-size-collector"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

bool DPCPPEnableDirectFunctionCallVectorization = true;
static cl::opt<bool, true> EnableDirectFunctionCallVectorizationOpt(
    "dpcpp-enable-direct-function-call-vectorization",
    cl::location(DPCPPEnableDirectFunctionCallVectorization), cl::Hidden,
    cl::desc("Enable direct function call vectorization"));

bool DPCPPEnableSubgroupDirectCallVectorization = true;
static cl::opt<bool, true> EnableSubgroupDirectCallVectorizationOpt(
    "dpcpp-enable-direct-subgroup-function-call-vectorization",
    cl::location(DPCPPEnableSubgroupDirectCallVectorization), cl::Hidden,
    cl::desc("Enable direct subgroup function call vectorization"));

bool DPCPPEnableVectorizationOfByvalByrefFunctions = false;
static cl::opt<bool, true> EnableVectorizationOfByvalByrefFunctionsOpt(
    "dpcpp-enable-byval-byref-function-call-vectorization",
    cl::location(DPCPPEnableVectorizationOfByvalByrefFunctions), cl::Hidden,
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
  if (!DPCPPEnableDirectFunctionCallVectorization)
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
      if (!CalledFunc || CalledFunc->isIntrinsic() ||
          CalledFunc->isDeclaration() ||
          CalledFunc->getName().startswith("WG.boundaries.") ||
          isStructReturn ||
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
  for (auto It : SGSizes) {
    Function *F = It.first;
    StringRef VarsStr;
    DenseSet<unsigned> ExistingVars;
    if (F->hasFnAttribute(KernelAttribute::VectorVariants)) {

      Attribute Attr = F->getFnAttribute(KernelAttribute::VectorVariants);
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

    F->addFnAttr(KernelAttribute::VectorVariants, join(Variants, ","));
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

// For legacy pass manager
namespace {
class SGSizeCollectorLegacy : public ModulePass {
public:
  static char ID;

  SGSizeCollectorLegacy(VFISAKind ISA = VFISAKind::SSE);

  llvm::StringRef getPassName() const override {
    return "SGSizeCollectorLegacy pass";
  }

protected:
  bool runOnModule(Module &M) override;

private:
  VFISAKind ISA;
};
} // namespace

SGSizeCollectorLegacy::SGSizeCollectorLegacy(VFISAKind ISA)
    : ModulePass(ID), ISA(ISA) {
  initializeSGSizeCollectorLegacyPass(*PassRegistry::getPassRegistry());
}

bool SGSizeCollectorLegacy::runOnModule(Module &M) {
  return SGSizeCollectorPass(ISA).runImpl(M);
}

char SGSizeCollectorLegacy::ID = 0;

INITIALIZE_PASS(SGSizeCollectorLegacy, "dpcpp-kernel-sg-size-collector",
                "Collecting subgroup size information", false, false)

ModulePass *llvm::createSGSizeCollectorLegacyPass(VFISAKind ISA) {
  return new SGSizeCollectorLegacy(ISA);
}
