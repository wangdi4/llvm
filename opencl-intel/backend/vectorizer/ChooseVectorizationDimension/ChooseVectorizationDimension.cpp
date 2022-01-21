// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "cl_env.h"
#define DEBUG_TYPE "Vectorizer"

#include "ChooseVectorizationDimension.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLAddressSpace.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

/// Support for dynamic loading of modules under Linux
char ChooseVectorizationDimension::ID = 0;

extern "C" Pass *createBuiltinLibInfoPass(ArrayRef<Module *> pRtlModuleList,
                                          std::string type);

OCL_INITIALIZE_PASS_BEGIN(ChooseVectorizationDimension,
                          "ChooseVectorizationDimension",
                          "Choosing Vectorization Dimension", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
OCL_INITIALIZE_PASS_DEPENDENCY(WorkItemAnalysisLegacy)
OCL_INITIALIZE_PASS_END(ChooseVectorizationDimension,
                        "ChooseVectorizationDimension",
                        "Choosing Vectorization Dimension", false, true)

ChooseVectorizationDimensionImpl::ChooseVectorizationDimensionImpl()
#ifndef INTEL_PRODUCT_RELEASE
  :
  OCLSTAT_INIT(Dim_Zero_Good_Store_Loads,
    "stores and loads that are good if the vectorization dimension is zero",
    m_kernelStats),
  OCLSTAT_INIT(Dim_Zero_Bad_Store_Loads,
    "stores and loads that are bad if the vectorization dimension is zero",
    m_kernelStats),
  OCLSTAT_INIT(Dim_One_Good_Store_Loads,
    "stores and loads that are good if the vectorization dimension is one",
    m_kernelStats),
  OCLSTAT_INIT(Dim_One_Bad_Store_Loads,
    "stores and loads that are bad if the vectorization dimension is one",
    m_kernelStats),
  OCLSTAT_INIT(Dim_Two_Good_Store_Loads,
    "stores and loads that are good if the vectorization dimension is two",
    m_kernelStats),
  OCLSTAT_INIT(Dim_Two_Bad_Store_Loads,
    "stores and loads that are bad if the vectorization dimension is two",
    m_kernelStats),
  OCLSTAT_INIT(Chosen_Vectorization_Dim,
    "The chosen vectorization dimension",
    m_kernelStats)
#endif
{}

enum class PreferredOption : unsigned {
  First = 0,
  Second = 1,
  Neutral = 2
};


/// @brief checks whether it is even allowed to switch dimensions.
/// specifically: a) KernelAnalysis pass determined no barrier
/// b) no get_local_id and get_group_id c) no access to local memory.
static bool canSwitchDimensions(Function* F) {
  using namespace DPCPPKernelMetadataAPI;

  // 1. test whether KernelAnalysis pass said: "no barrier", otherwise
  // switching dimensions is not supported.
  auto skimd = KernelInternalMetadataAPI(F);
  bool result = skimd.NoBarrierPath.hasValue() && skimd.NoBarrierPath.get();
  if (!result)
    return false;

  // 1a. test whether we use subgroups. That is not a hard requirement,
  // it is possible to switch vec dim, but with current Volcano vectorizer
  // it eases implementation. That can be lifted in VPO.
  bool HasSubGroups =
    skimd.KernelHasSubgroups.hasValue() && skimd.KernelHasSubgroups.get();
  if (HasSubGroups)
    return false;

  // 2. test if we use get_local_id / get_group_id / get_local_size.
  Function* lid = F->getParent()->getFunction
    (CompilationUtils::mangledGetLID());
  Function* groupId = F->getParent()->getFunction
    (CompilationUtils::mangledGetGroupID());
  Function* localSize = F->getParent()->getFunction
    (CompilationUtils::mangledGetLocalSize());
  Function* numGroups = F->getParent()->getFunction
    (CompilationUtils::mangledGetNumGroups());
  Function* enqueuedLocalSize = F->getParent()->getFunction
    (CompilationUtils::mangledGetEnqueuedLocalSize());

  std::set<Function *> forbiddenFunctions;
  if (lid)
    forbiddenFunctions.insert(lid);
  if (groupId)
    forbiddenFunctions.insert(groupId);
  if (localSize)
    forbiddenFunctions.insert(localSize);
  if (numGroups)
    forbiddenFunctions.insert(numGroups);
  if (enqueuedLocalSize)
    forbiddenFunctions.insert(enqueuedLocalSize);

  std::set<Function *> userFuncs;
  LoopUtils::fillFuncUsersSet(forbiddenFunctions, userFuncs);
  if (userFuncs.find(F) != userFuncs.end()) {
    return false; // a forbidden function is used.
  }

  // 3. test if we use local memory.
  for (auto &currInst : instructions(F)) {
    if (LoadInst* loadInst = dyn_cast<LoadInst>(&currInst)) {
      if (loadInst->getPointerAddressSpace() ==
        Utils::OCLAddressSpace::Local) {
        return false; // using local memory.
      }
    } else if (StoreInst* storeInst = dyn_cast<StoreInst>(&currInst)) {
      if (storeInst->getPointerAddressSpace() ==
        Utils::OCLAddressSpace::Local) {
        return false; // using local memory.
      }
    }
  }

  return true;
}

bool ChooseVectorizationDimensionImpl::hasDim(Function* F, unsigned int dim) {
  Function* gid =
      F->getParent()->getFunction(CompilationUtils::mangledGetGID());
  if (!gid)
    return false;

  for (auto *U : gid->users()) {
    if (U->use_empty())
      continue;

    CallInst *pInstCall = cast<CallInst>(U);
    if (pInstCall->getFunction() != F) {
      continue; // only interested if F uses the dim directly.
    }
    bool err, isTidGen;
    unsigned inst_dim = 0;
    std::tie(isTidGen, err, inst_dim) = RTService->isTIDGenerator(pInstCall);
    // KernelAnalysis should have set noBarrierPath to false if err is true.
    (void)isTidGen;
    V_ASSERT(isTidGen && !err &&
      "TIDGen inst receives non-constant input. Cannot vectorize!");
    if (dim == inst_dim)
      return true;
  }

  return false;
}

/// @brief counts how many good load stores and bad load stores
/// are in the given BasicBlock, if using the provided WorkItemAnalysis.
/// Good load stores: uniform load/stores and consecutive scalar load/stores.
/// Bad load stores: random/strided load stores and also vector consecutive
/// load/stores.
/// @param wi the WorkItemAnlaysis to ask for the dependency of the pointer param.
/// @param BB the basic block to count its instructions
/// @param goodLoadStores write the number of good load/stores into this param.
/// @param badLoadStores write the number of bad load/stores into this param.
/// @param dim the dimension we are counting. Only needed for statistics.
static void countLoadStores(WorkItemInfo &wi, BasicBlock *BB,
                            int &goodLoadStores, int &badLoadStores,
                            int /*dim*/) {
  goodLoadStores = badLoadStores = 0;
  for (auto &I : *BB) {
    Value* pointerOperand = nullptr;
    if (LoadInst* load = dyn_cast<LoadInst>(&I)) {
      pointerOperand = load->getPointerOperand();
    } else if (StoreInst* store = dyn_cast<StoreInst>(&I)) {
      pointerOperand = store->getPointerOperand();
    }

    if (!pointerOperand)
      continue;

    // it is either a load or a store
    if (wi.whichDepend(&I) == WorkItemInfo::UNIFORM) {
      goodLoadStores++; // uniform ops are always good.
      continue;
    }

    if (I.getType()->isVectorTy()) {
      badLoadStores++; // non-uniform vector ops are bad.
      continue;
    }

    WorkItemInfo::Dependency dep = wi.whichDepend(pointerOperand);
    if (dep == WorkItemInfo::PTR_CONSECUTIVE) {
      goodLoadStores++; // consecutive scalar ops are good.
      continue;
    }

    if (dep == WorkItemInfo::UNIFORM) {
      // arguably counter-intuitive, as this memop might be scalarized,
      // but we believe a uniform address is likely to suggest few executions anyway.
      goodLoadStores++;
      continue;
    }

    badLoadStores++; // random/strided are bad.
  }
}

/// @brief compares this basic block under for two different dimensions,
/// and outputs which dimension is better for vectorization considering
/// this specific basic block. Answer is: first/second/neutral.
/// When choosing between the two options, prefer the one with a higher
/// number of good load/stores. If number is identical, prefer
/// an option under which the basic block is uniform rather than divergent.
/// if uniformity/divergency is also identical, return neutral.
/// @param isDivergentInFirst is the block divergent in if vectorizing by the first dimension contested.
/// @param isDivergentInSecond is the block divergent in if vectorizing by the second dimension contested.
/// @param goodLoadStoresFirst number of good load/stores for the first dimension contested.
/// @param goodLoadStoresSecond number of good load/stores for the second dimension contested.
static PreferredOption getPreferredOption(
    bool isDivergentInFirst, bool isDivergentInSecond,
    int goodLoadStoresFirst, int goodLoadStoresSecond) {
  if (goodLoadStoresFirst > goodLoadStoresSecond)
    return PreferredOption::First;
  if (goodLoadStoresSecond > goodLoadStoresFirst)
    return PreferredOption::Second;

  if (!isDivergentInFirst && isDivergentInSecond)
    return PreferredOption::First;
  if (!isDivergentInSecond && isDivergentInFirst)
    return PreferredOption::Second;
  return PreferredOption::Neutral;
}

void ChooseVectorizationDimensionImpl::setFinalDecision(int dim, bool canUniteWorkGroups) {
#ifndef INTEL_PRODUCT_RELEASE
  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "DONT_SWITCH_DIM"))
    dim = 0;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "DONT_UNITE_WORKGROUPS"))
    canUniteWorkGroups = false;
#endif // INTEL_PRODUCT_RELEASE
  OCLSTAT_GATHER_CHECK(Chosen_Vectorization_Dim = dim);
  m_vectorizationDim = dim;
  m_canUniteWorkgroups = canUniteWorkGroups;
}

bool ChooseVectorizationDimensionImpl::preCheckDimZero(
    Function &F, RuntimeService *RTS) {
  RTService = RTS;
  int chosenVectorizationDimension = 0;
  bool canUniteWorkgroups = true;
  std::fill(std::begin(SwitchMotivation), std::end(SwitchMotivation), 0);
  std::fill(std::begin(PreferredDim), std::end(PreferredDim), 0);
  TotalDims = 0;

  if (!canSwitchDimensions(&F)) {
    chosenVectorizationDimension = 0;
    canUniteWorkgroups = false;
    setFinalDecision(chosenVectorizationDimension, canUniteWorkgroups);
    OCLSTAT_GATHER_CHECK(
        intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE));
    return true;
  }

  for (unsigned int Dim = 0; Dim < MAX_WORK_DIM; Dim++) {
    if (hasDim(&F, Dim)) {
      DimExist[Dim] = true;
      DimValid[Dim] = true;
      SwitchMotivation[Dim] = false;
      PreferredDim[Dim] = 0;
      TotalDims++;
    } else {
      DimExist[Dim] = false;
      DimValid[Dim] = false;
    }
  }

  if (TotalDims < 2 || !DimExist[0]) {
    chosenVectorizationDimension = 0;
    setFinalDecision(chosenVectorizationDimension, canUniteWorkgroups);
    OCLSTAT_GATHER_CHECK(
        intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE));
    return true;
  }
  return false;
}

bool ChooseVectorizationDimensionImpl::run(Function &F, WorkItemInfo &WIInfo) {
  int chosenVectorizationDimension = 0;
  bool canUniteWorkgroups = true;
  // Store number of good store/load per dimension.
  DenseMap<BasicBlock *, int> goodLoadStores[MAX_WORK_DIM];
  // Store number of bad store/load per dimension.
  DenseMap<BasicBlock *, int> badLoadStores[MAX_WORK_DIM];

  // now analyse the results for each BB.
  DenseSet<BasicBlock *> DivergentBlocks[MAX_WORK_DIM];
  for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
    // Dim 0 is already computed. Re-compute WorkItemInfo for higher dimension.
    if (Dim > 0)
      WIInfo.compute(Dim);

    for (auto &BB : F) {
      if (DimValid[Dim] && TotalDims > 1) {
        countLoadStores(WIInfo, &BB, goodLoadStores[Dim][&BB],
                        badLoadStores[Dim][&BB], Dim);
      } else if (DimExist[Dim]) {
        // count load stores only if needed for statistics.
        OCLSTAT_GATHER_CHECK(countLoadStores(WIInfo, &BB,
                                             goodLoadStores[Dim][&BB],
                                             badLoadStores[Dim][&BB], Dim));
      }

      if (WIInfo.isDivergentBlock(&BB))
        DivergentBlocks[Dim].insert(&BB);
    }
  }

  for (BasicBlock &currBlock : F) {
    for (unsigned int dim = 0; dim < MAX_WORK_DIM; dim++) {
      OCLSTAT_GATHER_CHECK(
      switch (dim) {
      case 0:
        Dim_Zero_Bad_Store_Loads += badLoadStores[dim][&currBlock];
        Dim_Zero_Good_Store_Loads += goodLoadStores[dim][&currBlock];
        break;
      case 1:
        Dim_One_Bad_Store_Loads += badLoadStores[dim][&currBlock];
        Dim_One_Good_Store_Loads += goodLoadStores[dim][&currBlock];
        break;
      case 2:
        Dim_Two_Bad_Store_Loads += badLoadStores[dim][&currBlock];
        Dim_Two_Good_Store_Loads += goodLoadStores[dim][&currBlock];
        break;
      });
    }
    for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
      if (!DimValid[dim])
        continue;

      PreferredOption prefer = getPreferredOption(
          DivergentBlocks[0].contains(&currBlock),
          DivergentBlocks[dim].contains(&currBlock),
          goodLoadStores[0][&currBlock], goodLoadStores[dim][&currBlock]);

      switch (prefer) {
      case PreferredOption::First:
        // if even one block prefers dim 0, than the other possiblity is never taken.
        DimValid[dim] = false;
        TotalDims--;
        break;
      case PreferredOption::Second:
        SwitchMotivation[dim] = true;
        break;
      case PreferredOption::Neutral:
        break;
      }
    }

    if (TotalDims > 2) { // find out which dims are best for this block.
      std::set<unsigned int> bestDims;
      int bestGoodLoadStores = -1;
      bool bestDivergence = true;
      for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
        if (!DimValid[dim])
          continue;
        bool currDivergence = DivergentBlocks[dim].contains(&currBlock);
        PreferredOption prefer = getPreferredOption(
            bestDivergence, currDivergence, bestGoodLoadStores,
            goodLoadStores[dim][&currBlock]);
        switch (prefer) {
        case PreferredOption::First:
          break; // best option remains the best
        case PreferredOption::Second:
          // now the new option is the best
          bestDims.clear();
          bestDims.insert(dim);
          bestGoodLoadStores = goodLoadStores[dim][&currBlock];
          bestDivergence = currDivergence;
          break;
        case PreferredOption::Neutral:
          // the current option is as good as the known best ones.
          bestDims.insert(dim);
        }
      }
      for (unsigned dim : bestDims)
        PreferredDim[dim]++;
    }
  }

  // find the best dim
  chosenVectorizationDimension = 0;
  int bestPreferredCount = -1;
  for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
    if (DimValid[dim] && SwitchMotivation[dim] &&
        PreferredDim[dim] > bestPreferredCount) {
      chosenVectorizationDimension = dim;
      bestPreferredCount = PreferredDim[dim];
    }
  }

  setFinalDecision(chosenVectorizationDimension, canUniteWorkgroups);
  OCLSTAT_GATHER_CHECK(
      intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE));
  return false;
}

char ChooseVectorizationDimensionModulePass::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(ChooseVectorizationDimensionModulePass,
                          "ChooseVectorizationDimensionModulePass",
                          "Choosing Vectorization Dimension", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
OCL_INITIALIZE_PASS_DEPENDENCY(WorkItemAnalysisLegacy)
OCL_INITIALIZE_PASS_END(ChooseVectorizationDimensionModulePass,
                        "ChooseVectorizationDimensionModulePass",
                        "Choosing Vectorization Dimension", false, true)

bool ChooseVectorizationDimensionModulePass::runOnModule(Module &M) {
  auto *RTS = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                  .getResult()
                  .getRuntimeService();
  auto Kernels = DPCPPKernelMetadataAPI::KernelList(M).getList();
  ChooseVectorizationDimensionImpl Impl;
  for (Function *Kernel : Kernels) {
    if (Kernel->hasOptNone())
      continue;
    if (!Impl.preCheckDimZero(*Kernel, RTS)) {
      auto &WIInfo = getAnalysis<WorkItemAnalysisLegacy>(*Kernel).getResult();
      Impl.run(*Kernel, WIInfo);
    }
    ChosenVecDims[Kernel] = Impl.getVectorizationDim();
    CanUniteWorkgroups[Kernel] = Impl.getCanUniteWorkgroups();
  }
  return false;
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  intel::ChooseVectorizationDimension* createChooseVectorizationDimension() {
    return new intel::ChooseVectorizationDimension();
  }

  intel::ChooseVectorizationDimensionModulePass*
      createChooseVectorizationDimensionModulePass() {
    return new intel::ChooseVectorizationDimensionModulePass();
  }
}
