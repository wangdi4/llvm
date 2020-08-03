// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
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

#define DEBUG_TYPE "Vectorizer"

#include "ChooseVectorizationDimension.h"
#include "BuiltinLibInfo.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "OCLAddressSpace.h"
#include "CompilationUtils.h"
#include "LoopUtils/LoopUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Type.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

/// Support for dynamic loading of modules under Linux
char ChooseVectorizationDimension::ID = 0;

extern "C" Pass* createBuiltinLibInfoPass(SmallVector<Module*, 2> pRtlModuleList, std::string type);

OCL_INITIALIZE_PASS_BEGIN(ChooseVectorizationDimension,
                          "ChooseVectorizationDimension",
                          "Choosing Vectorization Dimension", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
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
  using namespace Intel::MetadataAPI;

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
    CallInst *pInstCall = cast<CallInst>(U);
    if (pInstCall->getFunction() != F) {
      continue; // only interested if F uses the dim directly.
    }
    bool err, isTidGen;
    unsigned inst_dim = 0;
    isTidGen = m_rtServices->isTIDGenerator(pInstCall, &err, &inst_dim);
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
/// are in the given BasicBlock, if using the provided WIAnalysis.
/// Good load stores: uniform load/stores and consecutive scalar load/stores.
/// Bad load stores: random/strided load stores and also vector consecutive load/stores.
/// @param wi the WIAnlaysis to ask for the dependency of the pointer param.
/// @param BB the basic block to count its instructions
/// @param goodLoadStores write the number of good load/stores into this param.
/// @param badLoadStores write the number of bad load/stores into this param.
/// @param dim the dimension we are counting. Only needed for statistics.
static void countLoadStores(
    WIAnalysis* wi, BasicBlock* BB,
    int& goodLoadStores, int& badLoadStores, int dim) {
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
    if (wi->whichDepend(&I) == WIAnalysis::UNIFORM) {
      goodLoadStores++; // uniform ops are always good.
      continue;
    }

    if (I.getType()->isVectorTy()) {
      badLoadStores++; // non-uniform vector ops are bad.
      continue;
    }

    WIAnalysis::WIDependancy dep = wi->whichDepend(pointerOperand);
    if (dep == WIAnalysis::PTR_CONSECUTIVE) {
      goodLoadStores++; // consecutive scalar ops are good.
      continue;
    }

    if (dep == WIAnalysis::UNIFORM) {
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
  if (getenv("DONT_SWITCH_DIM")) {
    dim = 0;
  }
  if (getenv("DONT_UNITE_WORKGROUPS")) {
    canUniteWorkGroups = false;
  }
#endif // INTEL_PRODUCT_RELEASE
  OCLSTAT_GATHER_CHECK(Chosen_Vectorization_Dim = dim);
  m_vectorizationDim = dim;
  m_canUniteWorkgroups = canUniteWorkGroups;
}

bool ChooseVectorizationDimensionImpl::run(
    Function &F, const RuntimeServices *RTS,
    const SmallVector<Module *, 2> &Builtins) {
  m_rtServices = RTS;
  V_ASSERT(m_rtServices && "Runtime services were not initialized!");
  int chosenVectorizationDimension = 0;
  bool canUniteWorkgroups = true;

  if (!canSwitchDimensions(&F)) {
    chosenVectorizationDimension = 0;
    canUniteWorkgroups = false;
    setFinalDecision(chosenVectorizationDimension, canUniteWorkgroups);
    OCLSTAT_GATHER_CHECK(
        intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE));
    return false;
  }

  WIAnalysis* wi[MAX_WORK_DIM] = {nullptr}; // WI for each dimension.
  bool dimExist[MAX_WORK_DIM];  // whether the dimension exists.
  bool dimValid[MAX_WORK_DIM]; // whether the dimension is a valid possibility.
  int goodLoadStores[MAX_WORK_DIM] = {}; // will be used to store number of good store/load per dimension.
  int badLoadStores[MAX_WORK_DIM] = {}; // will be used to store number of bad store/load per dimension.
  bool switchMotivation[MAX_WORK_DIM]; // true if there is at least one block that prefers dimension x over 0.
  int preferredDim[MAX_WORK_DIM]; // how many BB's perfer dimension 1/2

  int totalDims = 0;
  for (unsigned int dim = 0; dim < MAX_WORK_DIM; dim++) {
    if (hasDim(&F, dim)) {
      dimExist[dim] = true;
      dimValid[dim] = true;
      switchMotivation[dim] = false;
      preferredDim[dim] = 0;
      totalDims++;
    }
    else {
      dimExist[dim] = false;
      dimValid[dim] = false;
    }
  }

  if (totalDims < 2 || !dimExist[0]) {
    chosenVectorizationDimension = 0;
    setFinalDecision(chosenVectorizationDimension, canUniteWorkgroups);
    OCLSTAT_GATHER_CHECK(
        intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE));
    return false;
  }

  // create function pass manager to run the WIAnalysis for each dimension.
  legacy::FunctionPassManager runWi(F.getParent());
  runWi.add(createBuiltinLibInfoPass(Builtins, ""));
  for (unsigned int dim = 0; dim < MAX_WORK_DIM; dim++) {
    if (dimExist[dim]) {
      wi[dim] = new WIAnalysis(dim); // construct WI and tell it on which dimension to run.
      runWi.add(wi[dim]);
    }
  }
  runWi.doInitialization();
  runWi.run(F);

  // now analyse the results for each BB.
  for (BasicBlock &currBlock : F) {
    for (unsigned int dim = 0; dim < MAX_WORK_DIM; dim++) {
      if (dimValid[dim] && totalDims > 1) {
        countLoadStores(wi[dim], &currBlock, goodLoadStores[dim], badLoadStores[dim], dim);
      } else if (dimExist[dim]) {
        // count load stores only if needed for statistics.
        OCLSTAT_GATHER_CHECK(
          countLoadStores(wi[dim], &currBlock, goodLoadStores[dim], badLoadStores[dim], dim);
        );
      }
      OCLSTAT_GATHER_CHECK(
      switch (dim) {
      case 0:
        Dim_Zero_Bad_Store_Loads += badLoadStores[dim];
        Dim_Zero_Good_Store_Loads += goodLoadStores[dim];
        break;
      case 1:
        Dim_One_Bad_Store_Loads += badLoadStores[dim];
        Dim_One_Good_Store_Loads += goodLoadStores[dim];
        break;
      case 2:
        Dim_Two_Bad_Store_Loads += badLoadStores[dim];
        Dim_Two_Good_Store_Loads += goodLoadStores[dim];
        break;
      });
    }
    for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
      if (!dimValid[dim])
        continue;

      PreferredOption prefer = getPreferredOption(
          wi[0]->isDivergentBlock(&currBlock),
          wi[dim]->isDivergentBlock(&currBlock),
          goodLoadStores[0], goodLoadStores[dim]);

      switch (prefer) {
      case PreferredOption::First:
        // if even one block prefers dim 0, than the other possiblity is never taken.
        dimValid[dim] = false;
        totalDims--;
        break;
      case PreferredOption::Second:
        switchMotivation[dim] = true;
        break;
      case PreferredOption::Neutral:
        break;
      }
    }

    if (totalDims > 2) { // find out which dims are best for this block.
      std::set<unsigned int> bestDims;
      int bestGoodLoadStores = -1;
      bool bestDivergence = true;
      for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
        if (!dimValid[dim])
          continue;
        bool currDivergence = wi[dim]->isDivergentBlock(&currBlock);
        PreferredOption prefer = getPreferredOption(
            bestDivergence, currDivergence,
            bestGoodLoadStores, goodLoadStores[dim]);
        switch (prefer) {
        case PreferredOption::First:
          break; // best option remains the best
        case PreferredOption::Second:
          // now the new option is the best
          bestDims.clear();
          bestDims.insert(dim);
          bestGoodLoadStores = goodLoadStores[dim];
          bestDivergence = currDivergence;
          break;
        case PreferredOption::Neutral:
          // the current option is as good as the known best ones.
          bestDims.insert(dim);
        }
      }
      for (unsigned dim : bestDims)
        preferredDim[dim]++;
    }
  }

  runWi.doFinalization();

  // find the best dim
  chosenVectorizationDimension = 0;
  int bestPreferredCount = -1;
  for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
    if (dimValid[dim] &&
      switchMotivation[dim] &&
      preferredDim[dim] > bestPreferredCount) {
      chosenVectorizationDimension = dim;
      bestPreferredCount = preferredDim[dim];
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
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(ChooseVectorizationDimensionModulePass,
                        "ChooseVectorizationDimensionModulePass",
                        "Choosing Vectorization Dimension", false, true)

bool ChooseVectorizationDimensionModulePass::runOnModule(Module &M) {
  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  auto Kernels = Intel::MetadataAPI::KernelList(*&M).getList();
  ChooseVectorizationDimensionImpl Impl;
  for (Function *Kernel : Kernels) {
    Impl.run(*Kernel, BLI.getRuntimeServices(), BLI.getBuiltinModules());
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
