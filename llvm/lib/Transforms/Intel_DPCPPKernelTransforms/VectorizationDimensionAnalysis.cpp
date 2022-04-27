//===- VectorizationDimensionAnalysis.cpp - Vectorize dim analysis *-C++-*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorizationDimensionAnalysis.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-vec-dim-analysis"

INITIALIZE_PASS_BEGIN(VectorizationDimensionAnalysisLegacy, DEBUG_TYPE,
                      "Choose vectorization dimension", false, true)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(WorkItemAnalysisLegacy)
INITIALIZE_PASS_END(VectorizationDimensionAnalysisLegacy, DEBUG_TYPE,
                    "Choose vectorization dimension", false, true)

char VectorizationDimensionAnalysisLegacy::ID = 0;

VectorizationDimensionAnalysisLegacy::VectorizationDimensionAnalysisLegacy()
    : ModulePass(ID) {
  initializeVectorizationDimensionAnalysisLegacyPass(
      *PassRegistry::getPassRegistry());
}

void VectorizationDimensionAnalysisLegacy::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
  AU.addRequired<WorkItemAnalysisLegacy>();
}

static void print(raw_ostream &OS,
                  const MapVector<Function *, VectorizeDimInfo> &VDInfos) {
  for (auto &VD : VDInfos) {
    OS << "VectorizationDimensionAnalysis for function " << VD.first->getName()
       << "\n";
    VD.second.print(OS);
  }
}

void VectorizationDimensionAnalysisLegacy::print(raw_ostream &OS,
                                                 const Module *) const {
  ::print(OS, VDInfos);
}

bool VectorizationDimensionAnalysisLegacy::runOnModule(Module &M) {
  auto *RTService = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                        .getResult()
                        .getRuntimeService();
  assert(RTService != nullptr && "Invalid runtime service!");
  auto Kernels = DPCPPKernelMetadataAPI::KernelList(M).getList();
  for (Function *F : Kernels) {
    if (F->hasOptNone())
      continue;
    VectorizeDimInfo VDInfo;
    if (!VDInfo.preCheckDimZero(*F, RTService)) {
      WorkItemInfo &WIInfo =
          getAnalysis<WorkItemAnalysisLegacy>(*F).getResult();
      VDInfo.compute(*F, WIInfo);
    }
    VDInfos.insert({F, std::move(VDInfo)});
  }
  return false;
}

ModulePass *llvm::createVectorizationDimensionAnalysisLegacyPass() {
  return new VectorizationDimensionAnalysisLegacy();
}

AnalysisKey VectorizationDimensionAnalysis::Key;

VectorizationDimensionAnalysis::Result
VectorizationDimensionAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
  auto *RTService = AM.getResult<BuiltinLibInfoAnalysis>(M).getRuntimeService();
  assert(RTService != nullptr && "Invalid runtime service!");

  auto Kernels = DPCPPKernelMetadataAPI::KernelList(M).getList();
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  Result VDInfos;
  for (Function *F : Kernels) {
    if (F->hasOptNone())
      continue;
    VectorizeDimInfo VDInfo;
    if (!VDInfo.preCheckDimZero(*F, RTService)) {
      WorkItemInfo &WIInfo = FAM.getResult<WorkItemAnalysis>(*F);
      VDInfo.compute(*F, WIInfo);
    }
    VDInfos.insert({F, std::move(VDInfo)});
  }
  return VDInfos;
}

VectorizeDimInfo::VectorizeDimInfo()
    : VectorizeDim(0), CanUniteWorkGroups(true), TotalDims(0) {
  std::fill(std::begin(DimExist), std::end(DimExist), false);
  std::fill(std::begin(DimValid), std::end(DimValid), false);
  std::fill(std::begin(SwitchMotivation), std::end(SwitchMotivation), false);
  std::fill(std::begin(PreferredDim), std::end(PreferredDim), 0);
  std::fill(std::begin(NumGoodLoadStores), std::end(NumGoodLoadStores), 0);
  std::fill(std::begin(NumBadLoadStores), std::end(NumBadLoadStores), 0);
}

/// Check whether it is even allowed to switch dimensions. Specifically all of
/// the following are true:
///   1. KernelAnalysis pass determined no barrier.
///   2. no get_local_id, get_group_id, get_local_size, get_num_groups or
///      get_enqueued_local_size.
///   3. no access to local memory.
static bool canSwitchDimension(Function *F) {
  using namespace DPCPPKernelMetadataAPI;

  // 1. Check whether KernelAnalysis pass said "no barrier", otherwise
  // switching dimension is not supported.
  auto KIMD = KernelInternalMetadataAPI(F);
  if (!(KIMD.NoBarrierPath.hasValue() && KIMD.NoBarrierPath.get()))
    return false;

  // 1a. Check whether we use subgroups. That is not a hard requirement, it is
  // possible to switch vec dim, but with current Volcano vectorizer it eases
  // implementation. That can be lifted in VPO.
  if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get())
    return false;

  // 2. Check whether we use get_local_id/get_group_id/get_local_size.
  FuncSet ForbiddenFuncs;
  std::string Names[] = {mangledGetLID(), mangledGetGroupID(),
                         mangledGetLocalSize(), mangledGetNumGroups(),
                         mangledGetEnqueuedLocalSize()};
  for (auto It = std::begin(Names), E = std::end(Names); It != E; ++It)
    if (Function *Func = F->getParent()->getFunction(*It))
      ForbiddenFuncs.insert(Func);
  FuncSet UserFuncs;
  DPCPPKernelLoopUtils::fillFuncUsersSet(ForbiddenFuncs, UserFuncs);
  if (UserFuncs.contains(F))
    return false;

  // 3. Check if we use local memory.
  for (auto &I : instructions(F)) {
    if (auto *LI = dyn_cast<LoadInst>(&I)) {
      if (LI->getPointerAddressSpace() == ADDRESS_SPACE_LOCAL)
        return false;
    } else if (auto *SI = dyn_cast<StoreInst>(&I))
      if (SI->getPointerAddressSpace() == ADDRESS_SPACE_LOCAL)
        return false;
  }

  return true;
}

bool VectorizeDimInfo::hasDim(Function *F, unsigned Dim) const {
  auto *Gid = F->getParent()->getFunction(mangledGetGID());
  if (!Gid)
    return false;

  for (auto *U : Gid->users()) {
    if (U->use_empty())
      continue;

    auto *CI = cast<CallInst>(U);
    if (CI->getFunction() != F) {
      // Only interested if F uses the dim directly.
      continue;
    }

    bool IsTidGen;
    bool Err;
    unsigned Dimension;
    std::tie(IsTidGen, Err, Dimension) = RTService->isTIDGenerator(CI);
    // KernelAnalysis should have set NobarrierPath to false if err is true.
    assert(IsTidGen && !Err &&
           "TIDGen inst receives non-constant input. Cannot vectorize!");
    if (Dim == Dimension)
      return true;
  }

  return false;
}

bool VectorizeDimInfo::preCheckDimZero(Function &F, const RuntimeService *RTS) {
  RTService = RTS;

  if (!canSwitchDimension(&F)) {
    LLVM_DEBUG(dbgs() << F.getName() << ": can't switch dimension\n");
    CanUniteWorkGroups = false;
    return true;
  }

  for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
    if (!hasDim(&F, Dim))
      continue;
    DimExist[Dim] = true;
    DimValid[Dim] = true;
    ++TotalDims;
  }

  if (TotalDims < 2 || !DimExist[0]) {
    LLVM_DEBUG(
        dbgs() << F.getName()
               << ": TotalDims is smaller than 2 or dim 0 doesn't exist");
    return true;
  }

  return false;
}

/// Count how many good load stores and bad load stores are in the given
/// BasicBlock, if using the provided WorkItemAnalysis.
///
/// Good load stores: uniform load/stores and consecutive scalar load/stores.
/// Bad load stores: random/strided load stores and vector consecutive
///                  load/stores.
/// \param WI WorkItemAnalysis to ask for the dependency of the pointer operand.
/// \param BB the basic block to count its instructions.
/// \return pair of {GoodLoadStores, BadLoadStores}.
static std::pair<unsigned, unsigned> countLoadStores(WorkItemInfo &WI,
                                                     BasicBlock &BB) {
  unsigned GoodLoadStores = 0;
  unsigned BadLoadStores = 0;
  for (auto &I : BB) {
    Value *PointerOp = nullptr;
    if (auto *LI = dyn_cast<LoadInst>(&I))
      PointerOp = LI->getPointerOperand();
    else if (auto *SI = dyn_cast<StoreInst>(&I))
      PointerOp = SI->getPointerOperand();
    if (!PointerOp)
      continue;

    // It is either a load or a store.
    if (WI.whichDepend(&I) == WorkItemInfo::UNIFORM) {
      // Uniform ops are always good.
      ++GoodLoadStores;
      continue;
    }

    if (I.getType()->isVectorTy()) {
      // Non-uniform vector ops are bad.
      ++BadLoadStores;
      continue;
    }

    WorkItemInfo::Dependency Dep = WI.whichDepend(PointerOp);
    if (Dep == WorkItemInfo::PTR_CONSECUTIVE) {
      // Consecutive scalar ops are good.
      ++GoodLoadStores;
      continue;
    }

    if (Dep == WorkItemInfo::UNIFORM) {
      // Arguably counter-intuitive, as this mem op might be scalarized, but we
      // believe a uniform address is likely to suggest few executions anyway.
      ++GoodLoadStores;
      continue;
    }

    // Random/Strided are bad.
    ++BadLoadStores;
  }

  return {GoodLoadStores, BadLoadStores};
}

enum class PreferredOption : unsigned { First = 0, Second = 1, Neutral = 2 };

/// Compare this basic block under for two different dimensions, and outputs
/// which dimension is better for vectorization considering this specific basic
/// block. Answer is: first/second/neutral.
/// When choosing between the two options, prefer the one with a higher number
/// of good load/stores.
/// If number is identical, prefer an option under which the basic block is
/// uniform rather than divergent.
/// If uniformity/divergency is also identical, return neutral.
/// \param IsDivergentInFirst is the block divergent in if vectorizing by the
/// first dimension contested.
/// \param IsDivergentInSecond is the block divergent in if vectorizing by the
/// second dimension contested.
/// \param GoodLoadStoresFirst Number of good load/stores for the first
/// dimension contested.
/// \param GoodLoadStoresSecond Number of good load/stores for the second
/// dimension contested.
static PreferredOption getPreferredOption(bool IsDivergentInFirst,
                                          bool IsDivergentInSecond,
                                          int GoodLoadStoresFirst,
                                          int GoodLoadStoresSecond) {
  if (GoodLoadStoresFirst > GoodLoadStoresSecond)
    return PreferredOption::First;
  if (GoodLoadStoresSecond > GoodLoadStoresFirst)
    return PreferredOption::Second;

  if (!IsDivergentInFirst && IsDivergentInSecond)
    return PreferredOption::First;
  if (!IsDivergentInSecond && IsDivergentInFirst)
    return PreferredOption::Second;

  return PreferredOption::Neutral;
}

void VectorizeDimInfo::compute(Function &F, WorkItemInfo &WIInfo) {
  // Store number of good store/load per dimension.
  DenseMap<BasicBlock *, int> GoodLoadStoresMap[MAX_WORK_DIM];

  // Now analyze the results for each BB.
  DenseSet<BasicBlock *> DivergentBlocks[MAX_WORK_DIM];
  for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
    // Compute WorkItemInfo for current dimension.
    WIInfo.compute(Dim);

    for (auto &BB : F) {
      unsigned GoodLoadStores = 0;
      unsigned BadLoadStores = 0;
      if (DimValid[Dim] && TotalDims > 1) {
        std::tie(GoodLoadStores, BadLoadStores) = countLoadStores(WIInfo, BB);
      }
#if !INTEL_PRODUCT_RELEASE
      else if (DimExist[Dim]) {
        // Count load stores only if needed for statistics.
        std::tie(GoodLoadStores, BadLoadStores) = countLoadStores(WIInfo, BB);
      }
#endif // !INTEL_PRODUCT_RELEASE
      GoodLoadStoresMap[Dim][&BB] = GoodLoadStores;
      NumGoodLoadStores[Dim] += GoodLoadStores;
      NumBadLoadStores[Dim] += BadLoadStores;

      if (WIInfo.isDivergentBlock(&BB))
        DivergentBlocks[Dim].insert(&BB);
    }
  }

  for (auto &BB : F) {
    for (unsigned Dim = 1; Dim < MAX_WORK_DIM; ++Dim) {
      if (!DimValid[Dim])
        continue;

      PreferredOption P = getPreferredOption(
          DivergentBlocks[0].contains(&BB), DivergentBlocks[Dim].contains(&BB),
          (int)GoodLoadStoresMap[0][&BB], (int)GoodLoadStoresMap[Dim][&BB]);
      switch (P) {
      case PreferredOption::First:
        // If even one block prefers dim 0, then the other possibility is never
        // taken.
        DimValid[Dim] = false;
        TotalDims--;
        break;
      case PreferredOption::Second:
        SwitchMotivation[Dim] = true;
        break;
      case PreferredOption::Neutral:
        break;
      }
    }

    if (TotalDims <= 2)
      continue;

    // Find out which dimension is the best for this block.
    std::set<unsigned> BestDims;
    int BestGoodLoadStores = -1;
    bool BestDivergence = true;
    for (unsigned Dim = 1; Dim < MAX_WORK_DIM; ++Dim) {
      if (!DimValid[Dim])
        continue;
      bool CurrDivergence = DivergentBlocks[Dim].contains(&BB);
      PreferredOption P =
          getPreferredOption(BestDivergence, CurrDivergence, BestGoodLoadStores,
                             (int)GoodLoadStoresMap[Dim][&BB]);
      switch (P) {
      case PreferredOption::First:
        // Best option remains the best.
        break;
      case PreferredOption::Second:
        // Now the new option is the best.
        BestDims.clear();
        BestDims.insert(Dim);
        BestGoodLoadStores = GoodLoadStoresMap[Dim][&BB];
        BestDivergence = CurrDivergence;
        break;
      case PreferredOption::Neutral:
        // The current option is as good as the known best one.
        BestDims.insert(Dim);
        break;
      }
    }
    for (unsigned Dim : BestDims)
      PreferredDim[Dim]++;
  }

  // Find the best dim.
  int BestPreferredCount = -1;
  for (unsigned Dim = 1; Dim < MAX_WORK_DIM; ++Dim) {
    if (DimValid[Dim] && SwitchMotivation[Dim] &&
        PreferredDim[Dim] > BestPreferredCount) {
      VectorizeDim = Dim;
      BestPreferredCount = PreferredDim[Dim];
    }
  }
}

void VectorizeDimInfo::print(raw_ostream &OS) const {
  OS.indent(2) << "VectorizeDim " << VectorizeDim << "\n";
  OS.indent(2) << "CanUniteWorkGroups " << CanUniteWorkGroups << "\n";
  OS.indent(2) << "TotalDims " << TotalDims << "\n";
  auto PrintArray = [&](StringRef Name, auto *A) {
    OS.indent(2) << Name << " [" << A[0];
    for (unsigned Dim = 1; Dim < MAX_WORK_DIM; ++Dim)
      OS << ", " << A[Dim];
    OS << "]\n";
  };
  PrintArray("DimExist", DimExist);
  PrintArray("DimValid", DimValid);
  PrintArray("SwitchMotivation", SwitchMotivation);
  PrintArray("PreferredDim", PreferredDim);
  PrintArray("NumGoodLoadStores", NumGoodLoadStores);
  PrintArray("NumBadLoadStores", NumBadLoadStores);
}

PreservedAnalyses
VectorizationDimensionAnalysisPrinter::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &VDInfos = AM.getResult<VectorizationDimensionAnalysis>(M);
  print(OS, VDInfos);
  return PreservedAnalyses::all();
}
