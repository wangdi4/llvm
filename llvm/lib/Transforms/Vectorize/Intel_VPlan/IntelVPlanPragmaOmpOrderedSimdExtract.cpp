//=------------- IntelVPlanPragmaOmpOrderedSimdExtract.cpp -*- C++ -*--------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#include "llvm/Transforms/Vectorize/IntelVPlanPragmaOmpOrderedSimdExtract.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanUtils.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPlanPragmaOmpOrderedSimdExtract"

char VPlanPragmaOmpOrderedSimdExtract::ID = 0;

INITIALIZE_PASS_BEGIN(VPlanPragmaOmpOrderedSimdExtract,
                      "vplan-pragma-omp-ordered-simd-extract",
                      "Function Outlining of Ordered Regions", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_END(VPlanPragmaOmpOrderedSimdExtract,
                    "vplan-pragma-omp-ordered-simd-extract",
                    "Function Outlining of Ordered Regions", false, false)

VPlanPragmaOmpOrderedSimdExtract::VPlanPragmaOmpOrderedSimdExtract()
    : ModulePass(ID) {
  initializeVPlanPragmaOmpOrderedSimdExtractPass(
      *PassRegistry::getPassRegistry());
}

Pass *llvm::createVPlanPragmaOmpOrderedSimdExtractPass() {
  return new VPlanPragmaOmpOrderedSimdExtract();
}

PreservedAnalyses
VPlanPragmaOmpOrderedSimdExtractPass::run(Module &M,
                                          ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  auto DT = [&FAM](Function &F) -> DominatorTree * {
    return &FAM.getResult<DominatorTreeAnalysis>(F);
  };

  auto WRI = [&FAM](Function &F) -> vpo::WRegionInfo * {
    return &FAM.getResult<WRegionInfoAnalysis>(F);
  };

  if (!Impl.runImpl(M, DT, WRI))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses::none();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}

void VPlanPragmaOmpOrderedSimdExtract::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<WRegionInfoWrapperPass>();
  AU.addPreserved<AndersensAAWrapperPass>(); // INTEL
  AU.addPreserved<AAResultsWrapperPass>();
  AU.addPreserved<GlobalsAAWrapperPass>();
}

bool VPlanPragmaOmpOrderedSimdExtract::runOnModule(Module &M) {
  // Don't run pass if module is skipped in opt-bisect mode.
  if (skipModule(M))
    return false;

  auto DT = [this](Function &F) -> DominatorTree * {
    return &this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
  };

  auto WRI = [this](Function &F) -> vpo::WRegionInfo * {
    return &this->getAnalysis<WRegionInfoWrapperPass>(F).getWRegionInfo();
  };

  return Impl.runImpl(M, DT, WRI, getLimiter());
}

// Visitor for ordered simd regions.
class OrderedSimdWRegionVisitor {
public:
  WRegionListTy &WRNOrderedSimdList;

  OrderedSimdWRegionVisitor(WRegionListTy &WL) : WRNOrderedSimdList(WL) {}

  void preVisit(WRegionNode *W) {
    if (isa<WRNOrderedNode>(W) && W->getIsSIMD())
      WRNOrderedSimdList.push_back(W);
  }

  void postVisit(WRegionNode *W) {}

  bool quitVisit(WRegionNode *W) { return false; }
};

bool VPlanPragmaOmpOrderedSimdExtractImpl::runImpl(Module &M, DomT DT,
                                                   WRenInfo WRI,
                                                   LoopOptLimiter Limiter) {

  SmallVector<Function *, 2> FunctionsToProcess;
  for (auto &F : M.functions()) {
    // Skip function declarations.
    if (F.isDeclaration())
      continue;

    if (!doesLoopOptPipelineAllowToRun(Limiter, F))
      continue;

    FunctionsToProcess.push_back(&F);
  }

  for (unsigned FnIdx = 0, FnEnd = FunctionsToProcess.size(); FnIdx < FnEnd;
       FnIdx++) {
    Function *F = FunctionsToProcess[FnIdx];
    DominatorTree *DomTree = DT(*F);
    vpo::WRegionInfo *WRInfo = WRI(*F);
    WRInfo->buildWRGraph();
    WRegionListTy WRNOrderedSimdList;
    // Collect all the ordered simd regions. The ordered simd regions should be
    // inside the vector loop. According to OpenMP 5.0 specification, there are
    // not any nested ordered simd regions inside the vector loop.
    OrderedSimdWRegionVisitor Visitor(WRNOrderedSimdList);
    WRegionUtils::forwardVisit(Visitor, WRInfo->getWRGraph());
    for (unsigned OrderIdx = 0, OrderEnd = WRNOrderedSimdList.size();
         OrderIdx != OrderEnd; OrderIdx++) {
      auto *OrderedSimdNode = WRNOrderedSimdList[OrderIdx];
      assert(isa<WRNOrderedNode>(OrderedSimdNode) &&
             "Ordered node is expected.");
      SmallVector<BasicBlock *, 2> OrderedSimdBlocks;
      llvm::copy(sese_depth_first(OrderedSimdNode->getEntryBBlock(),
                                  OrderedSimdNode->getExitBBlock()),
                 std::back_inserter(OrderedSimdBlocks));
      // Call Code Extractor for each ordered simd region.
      CodeExtractor Extractor(
          OrderedSimdBlocks, DomTree,
          false /* live-ins and live-outs will be aggregate arguments in
                   the new function */,
          nullptr /* block frequency */, nullptr /* branch probability */,
          nullptr /* assumption cache */, false /* allow varying arguments */,
          true /*allow extraction of blocks with alloca instructions */,
          nullptr /*allocation block*/,
          "ordered.simd.region" /*suffix which is appended to the name
                                  of the new function */,
          true /* allow safety check for llvm.eh.typeid.for intrinsic */,
          false /* don't allow unreachable blocks in the extracted function */,
          nullptr);

      CodeExtractorAnalysisCache CEAC(*F);
      Function *NewFunc =
          Extractor.extractCodeRegion(CEAC, true /* fix alloca */);
      if (!NewFunc) {
        LLVM_DEBUG(dbgs() << "Code extractor failed to remove the order region."
                          << "\n");
        continue;
      }

      // Check whether the name of the newly created function has a mangled name
      // and remove it.
      StringRef OldFuncName = NewFunc->getName();
      if (OldFuncName.startswith("_ZGV") &&
          NewFunc->hasFnAttribute("vector-variants")) {
        size_t UnderscoreIdx = OldFuncName.find('_', 1);
        assert((UnderscoreIdx != StringRef::npos) && "Underscore is expected!");
        StringRef UnMangledName = OldFuncName.substr(UnderscoreIdx + 1);
        // Add a enumerator at the end of the function since a function might
        // have multiple ordered regions.
        std::string NewFuncName;
        if (isdigit(static_cast<unsigned char>(UnMangledName[0])))
          NewFuncName = std::string("_Z") + UnMangledName.str() +
                        std::string("_") + std::to_string(FnIdx + OrderIdx);
        else
          NewFuncName = UnMangledName.str() + std::string("_") +
                        std::to_string(FnIdx + OrderIdx);
        NewFunc->setName(NewFuncName);
      }

      // Prepare the new function for inlining.
      if (NewFunc->hasFnAttribute(Attribute::NoInline))
        NewFunc->removeFnAttr(Attribute::NoInline);
      NewFunc->addFnAttr(Attribute::AlwaysInline);
      // Remove vector-variants attributes to prevent call vectorization of the
      // function that code extractor emits.
      if (NewFunc->hasFnAttribute("vector-variants"))
        NewFunc->removeFnAttr("vector-variants");
    }
  }
  //FIXME: return false if all functions were skipped or IR was not modified.
  return true; // LLVM IR has been modified
}
