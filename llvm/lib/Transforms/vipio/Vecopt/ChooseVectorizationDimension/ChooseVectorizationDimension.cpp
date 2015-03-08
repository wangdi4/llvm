/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#define DEBUG_TYPE "Vectorizer"

#include "ChooseVectorizationDimension.h"
#include "BuiltinLibInfo.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"
#include "LoopUtils/LoopUtils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/PassManager.h"
#include "llvm/IR/Type.h"

using namespace Intel;

namespace intel {

#define MAX_WORK_DIM (3) /* xmain */

/// Support for dynamic loading of modules under Linux
char ChooseVectorizationDimension::ID = 0;

extern "C" Pass* createBuiltinLibInfoPass(llvm::Module* pRTModule, std::string type);

OCL_INITIALIZE_PASS_BEGIN(ChooseVectorizationDimension, "ChooseVectorizationDimension", "Predicate Function", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(ChooseVectorizationDimension, "ChooseVectorizationDimension", "Predicate Function", false, false)

ChooseVectorizationDimension::ChooseVectorizationDimension() :
  FunctionPass(ID),
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
{
}

bool ChooseVectorizationDimension::canSwitchDimensions(Function* F) {
  return true;
}

bool ChooseVectorizationDimension::hasDim(Function* F, unsigned int dim) {
  Function* gid = F->getParent()->getFunction
    (CompilationUtils::mangledGetGID());
  if (!gid)
    return false;

  for ( Value::user_iterator ui = gid->user_begin(),
    ue = gid->user_end(); ui != ue; ++ui ) {
    if (isa<Function>(*ui)) {
      // xmain ::: get_global_id() is defined in the compiled module (as opposed
      // to the runtime module), so just ignore the definition.
      continue;
    }
    CallInst *pInstCall = dyn_cast<CallInst>(*ui);
    assert( pInstCall &&
      "Something other than CallInst is using get_global_id function!" );
    if (pInstCall->getParent()->getParent() != F) {
      continue; // only interested if F uses the dim directly.
    }
    bool err, isTidGen;
    unsigned inst_dim = 0;
    isTidGen = m_rtServices->isTIDGenerator(pInstCall, &err, &inst_dim);
    // (KernelAnalysis should have set noBarrierPath to false if err is true)
    V_ASSERT(isTidGen && !err &&
      "TIDGen inst receives non-constant input. Cannot vectorize!");
    if (dim == inst_dim) return true;
  }

  return false;
}

void ChooseVectorizationDimension::countLoadStores(WIAnalysis* wi, BasicBlock* BB,
  int& goodLoadStores, int& badLoadStores, int dim) {
  goodLoadStores = badLoadStores = 0;
  for (BasicBlock::iterator it = BB->begin(), e = BB->end(); it != e; ++it) {
    Value* pointerOperand = NULL;
    if (LoadInst* load = dyn_cast<LoadInst>(it)) {
      pointerOperand = load->getPointerOperand();
    }
    else if (StoreInst* store = dyn_cast<StoreInst>(it)) {
      pointerOperand = store->getPointerOperand();
    }

    if (pointerOperand) { // it is either a load or a store
      if (wi->whichDepend(it) == WIAnalysis::UNIFORM) {
        goodLoadStores++; // uniform ops are always good.
      }
      else if (it->getType()->isVectorTy()) {
        badLoadStores++; // non-uniform vector ops are bad.
      }
      else {
        WIAnalysis::WIDependancy dep = wi->whichDepend(pointerOperand);
        if (dep == WIAnalysis::PTR_CONSECUTIVE) {
          goodLoadStores++; // consecutive scalar ops are good.
        }
        else if (dep == WIAnalysis::UNIFORM) {
          // arguably counter-intuitive, as this memop might be scalarized,
          // but we believe a uniform address is likely to suggest few executions anyway.
          goodLoadStores++;
        }
        else {
          badLoadStores++; // random/strided are bad.
        }
      }
    }
  }

  // Statistics:
  if (dim == 0) {
    Dim_Zero_Bad_Store_Loads += badLoadStores;
    Dim_Zero_Good_Store_Loads += goodLoadStores;
  }
  else if (dim == 1) {
    Dim_One_Bad_Store_Loads += badLoadStores;
    Dim_One_Good_Store_Loads += goodLoadStores;
  }
  else if (dim == 2) {
    Dim_Two_Bad_Store_Loads += badLoadStores;
    Dim_Two_Good_Store_Loads += goodLoadStores;
  }
}

ChooseVectorizationDimension::PreferredOption ChooseVectorizationDimension::getPreferredOption(
  bool isDivergentInFirst,
  bool isDivergentInSecond,
  int goodLoadStoresFirst,
  int goodLoadStoresSecond) {

  if (goodLoadStoresFirst > goodLoadStoresSecond)
    return ChooseVectorizationDimension::First;
  if (goodLoadStoresSecond > goodLoadStoresFirst)
    return ChooseVectorizationDimension::Second;

  if (!isDivergentInFirst && isDivergentInSecond)
    return ChooseVectorizationDimension::First;
  if (!isDivergentInSecond && isDivergentInFirst)
    return ChooseVectorizationDimension::Second;
  return ChooseVectorizationDimension::Neutral;
}

void ChooseVectorizationDimension::setFinalDecision(int dim, bool canUnitWorkGroups) {
  if (getenv("DONT_SWITCH_DIM")) {
    dim = 0;
  }
  if (getenv("DONT_UNITE_WORKGROUPS")) {
    canUnitWorkGroups = false;
  }
  Chosen_Vectorization_Dim = dim; // Statistics
  m_vectorizationDim = dim;
  m_canUniteWorkgroups = canUnitWorkGroups;
}

bool ChooseVectorizationDimension::runOnFunction(Function &F) {
  m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  V_ASSERT(m_scalarFunc && "missing scalar function");
  V_ASSERT(m_rtServices && "Runtime services were not initialized!");
  int chosenVectorizationDimension = 0;
  bool canUniteWorkgroups = true;

  if (!canSwitchDimensions(&F)) {
    chosenVectorizationDimension = 0;
    canUniteWorkgroups = false;
    setFinalDecision(chosenVectorizationDimension, canUniteWorkgroups);
    intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE);
    return true;
  }

  WIAnalysis* wi[MAX_WORK_DIM]; // WI for each dimension.
  bool dimExist[MAX_WORK_DIM];  // whether the dimension exists.
  bool dimValid[MAX_WORK_DIM]; // whether the dimension is a valid possibility.
  int goodLoadStores[MAX_WORK_DIM]; // will be used to store number of good store/load per dimension.
  int badLoadStores[MAX_WORK_DIM]; // will be used to store number of bad store/load per dimension.
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
    intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE);
    return true;
  }

  // create function pass manager to run the WIAnalysis for each dimension.
  FunctionPassManager runWi(F.getParent());
  runWi.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModule(), ""));
  for (unsigned int dim = 0; dim < MAX_WORK_DIM; dim++) {
    if (dimExist[dim]) {
      wi[dim] = new WIAnalysis(dim); // construct WI and tell it on which dimension to run.
      runWi.add(wi[dim]);
    }
  }
  runWi.run(F);

  // now analyse the results for each BB.
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++ it) {
    BasicBlock* currBlock = it;
    for (unsigned int dim = 0; dim < MAX_WORK_DIM; dim++) {
      if (dimValid[dim] && totalDims > 1) {
        countLoadStores(wi[dim], currBlock, goodLoadStores[dim], badLoadStores[dim], dim);
      }
      else if (dimExist[dim]) {
        // count load stores only if needed for statistics
        OCLSTAT_GATHER_CHECK(
          countLoadStores(wi[dim], currBlock, goodLoadStores[dim], badLoadStores[dim], dim);
        );
      }
    }
    for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
      if (dimValid[dim]) {
        ChooseVectorizationDimension::PreferredOption prefer =
          getPreferredOption(wi[0]->isDivergentBlock(currBlock),
            wi[dim]->isDivergentBlock(currBlock),
            goodLoadStores[0], goodLoadStores[dim]);

        switch (prefer)
        {
        case ChooseVectorizationDimension::First:
          // if even one block prefers dim 0, than the other possiblity is never taken.
          dimValid[dim] = false;
          totalDims--;
          break;
        case ChooseVectorizationDimension::Second:
          switchMotivation[dim] = true;
          break;
        case ChooseVectorizationDimension::Neutral:
          break;
        }
      }
    }
    if (totalDims > 2) { // find out which dims are best for this block
      std::set<unsigned int> bestDims;
      int bestGoodLoadStores = -1;
      bool bestDivergence = true;
      for (unsigned int dim = 1; dim < MAX_WORK_DIM; dim++) {
        if (!dimValid[dim])
          continue;
        ChooseVectorizationDimension::PreferredOption prefer =
          getPreferredOption(bestDivergence,
            wi[dim]->isDivergentBlock(currBlock),
            bestGoodLoadStores, goodLoadStores[dim]);
        switch (prefer) {
        case ChooseVectorizationDimension::First:
         break; // best option remains the best
        case ChooseVectorizationDimension::Second:
         // now the new option is the best
         bestDims.clear();
         bestDims.insert(dim);
         bestGoodLoadStores = goodLoadStores[dim];
         bestDivergence = wi[dim]->isDivergentBlock(currBlock);
         break;
        case ChooseVectorizationDimension::Neutral:
          // the current option is as good as the known best ones.
          bestDims.insert(dim);
        }
      }
      for (std::set<unsigned int>::iterator it2 = bestDims.begin(),
        e2 = bestDims.end(); it2 != e2; ++it2) {
        preferredDim[*it2]++;
      }
    }
  }

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
  intel::Statistic::pushFunctionStats(m_kernelStats, F, DEBUG_TYPE);
  return true;
}

void ChooseVectorizationDimension::setScalarFunc(Function* F) {
  m_scalarFunc = F;
}

unsigned int ChooseVectorizationDimension::getVectorizationDim() {
  return m_vectorizationDim;
}

bool ChooseVectorizationDimension::getCanUniteWorkgroups() {
  return m_canUniteWorkgroups;
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  intel::ChooseVectorizationDimension* createChooseVectorizationDimension() {
    return new intel::ChooseVectorizationDimension();
  }
}
