//===----- Intel_MultiVersion.cpp - Whole Function multi-versioning -*-----===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements whole function multi-versioning transformation based
// on the boolean invariant.
//
// If a function body has lots of conditional code depending on a boolean
// variable which is not changed during function execution, we can create
// function body specializations for the 'true' and 'false' values of that
// variable and branch to the appropriate specialization from the function
// entry.
//
// The transformation is currently limited to boolean values that are read
// from the structures whose address was passed to the function in arguments.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_MultiVersioning.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

#define DEBUG_TYPE "multiversioning"

#define DEBUG_PREFIX "MultiVersioning: "

// Defines the lowest number of conditional uses for bool closure in order to
// apply function multiversioning.
static cl::opt<uint32_t> MultiVersioningThreshold("multiversioning-threshold",
                                                  cl::init(20u),
                                                  cl::ReallyHidden);

STATISTIC(NumMultiVersionedFunctions, "Number of multi-versioned functions");

namespace {

class BoolMultiVersioningImpl {
  // Represents a use-def sequence of instructions (closure) which match the
  // following pattern
  //
  //              Arg (pointer-to-struct function argument)
  //                 /           |          \
  //               GEP          ...         GEP
  //             /     \                      \
  //          Load     Load                  Load
  //          /          \                      \
  //      ICmp EQ, 0     ...                 ICmp NE, 0
  //         |                              /    |     \
  //      Branch                          ...  Select  ...
  //
  // Each GEP returns an address of the structure field whose address is
  // passed to the function in arguments. All GEPs included into closure have
  // the same constant offset and have the same result type. Load result is
  // a scalar integer. All loads in closure have the same result type. Load
  // results are compared for EQ/NE with zero. Compare results are used as
  // condition in conditional branches and select instructions. Instructions
  // included into closure may have other uses outside of the closure.
  //
  // An example of such use-def chain
  //
  // define void @foo(%StructType* %Arg) {
  // ...
  //   %0 = getelementptr inbounds %StructType, %StructType* %Arg, ...
  //   %1 = load i8, i8* %0, align 4, !tbaa !2, !range !11
  //   %2 = icmp ne i8 %1, 0
  //   ...
  //   br i1 %2, label %if.true, label %if.false
  //   ...
  //   select i1 %2, label %val.true, label %val.false
  // ...
  // }
  class BoolClosure {
  public:
    // A list of pairs - compare instruction and the number of its uses which
    // use compare as condition.
    using CmpList = SmallVector<std::pair<ICmpInst *, unsigned>, 8u>;

    // A list of pairs - load instruction and its uses which are integer EQ/NE
    // compares with a zero.
    using LoadList = std::list<std::pair<LoadInst *, CmpList>>;

    // A list of pairs - GEP instruction and a list of load instructions which
    // load integer from the address returned by the GEP.
    using GEPList = std::list<std::pair<GetElementPtrInst *, LoadList>>;

    GEPList &getGEPs() { return GEPs; }
    const GEPList &getGEPs() const { return GEPs; }

    // Returns the total number of uses which use compares from this closure as
    // condition.
    unsigned getNumUses() const {
      unsigned NumUses = 0u;
      for (const auto &GEP : GEPs)
        for (const auto &Load : GEP.second)
          for (const auto &Cmp : Load.second)
            NumUses += Cmp.second;
      return NumUses;
    }

    bool validate(AAResults &AAR) {
      // Do early checks before engaging expensive alias analysis
      // 1. Check the number of conditional uses. If it is below threshold
      //    do not even bother with further validation.
      // 2. Check type consistency. All GEPs and loads are supposed to have
      //    matching types.
      if (getNumUses() < MultiVersioningThreshold) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Not enough uses (pre-check) - "
                          << getNumUses() << "\n");
        return false;
      }
      if (!hasConsistentTypes()) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "(Unsafe) Inconsistent types\n");
        return false;
      }

      // Now iterate over loads and check if it would be safe to move each load
      // to the function entry block using the alias analysis results. If not,
      // remove such unsafe loads from the closure. It would still be legal to
      // do multiversioning for the remaining closure given that it has enough
      // conditional uses.
      for (auto GI = GEPs.begin(), GE = GEPs.end(); GI != GE;) {
        auto GEPIt = GI++;

        auto isSafeToMoveToEntryBlock = [&](const LoadInst *Load) -> bool {
          const auto *BB = Load->getParent();
          auto Loc = MemoryLocation::get(Load);

          if (AAR.canInstructionRangeModRef(BB->front(), *Load, Loc,
                                            ModRefInfo::Mod))
            return false;

          df_iterator_default_set<const BasicBlock *, 16> Visited;
          for (const auto *PBB : predecessors(BB))
            for (const auto *TBB : inverse_depth_first_ext(PBB, Visited))
              if (AAR.canBasicBlockModify(*TBB, Loc))
                return false;
          return true;
        };

        // Check to see if it is safe to move loads to the function entry block.
        auto &Loads = GEPIt->second;
        for (auto LI = Loads.begin(), LE = Loads.end(); LI != LE;) {
          auto LoadIt = LI++;
          if (!isSafeToMoveToEntryBlock(LoadIt->first)) {
            LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Removing unsafe-to-move load\n"
                              << *LoadIt->first << "\n");
            Loads.erase(LoadIt);
          }
        }

        // Remove GEPs with no uses.
        if (Loads.empty())
          GEPs.erase(GEPIt);
      }

      // After removing unsafe loads check again if the number of conditional
      // uses is still sufficient for the transformation.
      if (getNumUses() < MultiVersioningThreshold) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Not enough uses (post-check) - "
                          << getNumUses() << "\n");
        return false;
      }
      return true;
    }

  private:
    // Checks type consistency in the closure
    // (a) all GEPs in the closure have the same types
    // (b) all loads have the same types
    bool hasConsistentTypes() const {
      for (const auto &GEP : GEPs) {
        const auto &FirstGEP = GEPs.front();
        if (GEP.first->getType() != FirstGEP.first->getType())
          return false;
        for (const auto &Load : GEP.second) {
          const auto &FirstLoad = FirstGEP.second.front();
          if (Load.first->getType() != FirstLoad.first->getType())
            return false;
        }
      }
      return true;
    }

  private:
    GEPList GEPs;
  };

  void buildClosures(const Argument &Arg,
                     SmallVectorImpl<BoolClosure> &Closures) const {
    // We are interested only in a pointer-to-a-struct arguments.
    const auto *Type = dyn_cast<PointerType>(Arg.getType());
    if (!Type || !Type->getElementType()->isStructTy())
      return;

    LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Building closure for " << F.getName()
                      << " argument " << Arg << " ...\n");

    struct APIntCompare {
      bool operator()(const APInt &L, const APInt &R) const { return L.ult(R); }
    };
    std::map<APInt, BoolClosure, APIntCompare> Fields;

    // Iterate argument uses and build use chains which match the pattern
    // we are looking for.
    for (const auto &ArgU : Arg.uses()) {
      LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Checking argument use:\n"
                        << *ArgU.getUser() << "\n");

      auto *GEP = dyn_cast<GetElementPtrInst>(ArgU.getUser());
      if (!GEP || !GEP->getResultElementType()->isIntegerTy()) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "(Skip) Does not match pattern\n");
        continue;
      }

      // We are interested only in GEPs with constant offsets.
      const auto &DL = Arg.getParent()->getParent()->getDataLayout();
      APInt Offset(DL.getPointerSizeInBits(GEP->getPointerAddressSpace()), 0u);
      if (!GEP->accumulateConstantOffset(DL, Offset)) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "(Skip) Non constant GEP offset\n");
        continue;
      }

      // Look at GEP uses and collect loads.
      BoolClosure::LoadList Loads;
      for (const auto &GEPU : GEP->uses()) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Checking GEP use (offset " << Offset
                          << ")\n"
                          << *GEPU.getUser() << "\n");

        // Should be used by a load returning a scalar integer.
        auto *Load = dyn_cast<LoadInst>(GEPU.getUser());
        if (!Load || !Load->isSimple() || !Load->getType()->isIntegerTy()) {
          LLVM_DEBUG(dbgs() << DEBUG_PREFIX "(Skip) Does not match pattern\n");
          continue;
        }

        // Collect compares.
        BoolClosure::CmpList Cmps;
        for (const auto &LoadU : Load->uses()) {
          LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Checking load use\n"
                            << *LoadU.getUser() << "\n");

          // We are looking for integer EQ/NE compares with a zero.
          auto *Cmp = dyn_cast<ICmpInst>(LoadU.getUser());
          if (!Cmp || !Cmp->isEquality()) {
            LLVM_DEBUG(dbgs()
                       << DEBUG_PREFIX "(Skip) Does not match pattern\n");
            continue;
          }
          const auto *Zero = dyn_cast<ConstantInt>(Cmp->getOperand(1));
          if (!Zero || !Zero->isZero()) {
            LLVM_DEBUG(dbgs()
                       << DEBUG_PREFIX "(Skip) Does not match pattern\n");
            continue;
          }

          // Count the number of cmp uses which use it as condition.
          unsigned NumCondUses = 0u;
          for (const auto &Use : Cmp->uses())
            if (isa<BranchInst>(Use.getUser()))
              ++NumCondUses;
            else if (auto *Select = dyn_cast<SelectInst>(Use.getUser()))
              if (Select->getCondition() == Cmp)
                ++NumCondUses;

          // Ok, this compare matches pattern - add it to the tree.
          LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Adding compare with "
                            << NumCondUses << " conditional uses\n");
          Cmps.emplace_back(Cmp, NumCondUses);
        }
        if (!Cmps.empty()) {
          LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Adding load to closure\n");
          Loads.emplace_back(Load, std::move(Cmps));
        }
      }
      if (!Loads.empty()) {
        LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Adding GEP to " << Arg << " (offset "
                          << Offset << ") closure\n");
        Fields[Offset].getGEPs().emplace_back(GEP, std::move(Loads));
      }
    }

    // Validation part. Check if it is safe to do function multi-versioning for
    // all collected closures. If true, move closure to the list of candidates
    // for multi-versioning.
    for (auto &Field : Fields) {
      LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Validating " << Arg << " (offset "
                        << Field.first << ") closure ...\n");

      auto &Closure = Field.second;
      if (!Closure.validate(AAR))
        continue;

      LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Is safe for bool multi-versioning. "
                        << "Number of uses - " << Closure.getNumUses() << "\n");
      Closures.emplace_back(std::move(Closure));
    }
  }

  // An extension of ValueToValueMapTy which is used for cloning function body
  // (added template to cast cloned value to the original value type). Defining
  // it here because function-level templates are not allowed.
  struct Value2CloneMapTy : public ValueToValueMapTy {
    template <typename T> T *getClone(const T *V) const {
      auto It = this->find(V);
      assert(It != this->end() && "no clone for the given value");
      return cast<T>(It->second);
    }
  };

  // Return the branch weights for the 'true' and 'false' path, if there is
  // profiling data.
  static Optional<std::pair<uint64_t, uint64_t>>
  getBranchWeights(const BranchInst *I) {
    MDNode *ProfMD = I->getMetadata(LLVMContext::MD_prof);
    if (!ProfMD)
      return None;

    MDString *MDS = dyn_cast<MDString>(ProfMD->getOperand(0));
    if (!MDS || !MDS->getString().equals("branch_weights"))
      return None;

    assert(ProfMD->getNumOperands() == 3 && "Expected 3 value operands");
    ConstantInt *CI1 = mdconst::extract<ConstantInt>(ProfMD->getOperand(1));
    ConstantInt *CI2 = mdconst::extract<ConstantInt>(ProfMD->getOperand(2));

    return std::make_pair(CI1->getValue().getZExtValue(),
                          CI2->getValue().getZExtValue());
  }

  // Multi-version function body for the given closure.
  void doMultiVersioning(BoolClosure &C) const {
    LLVM_DEBUG(dbgs() << DEBUG_PREFIX "Multi-versioning function "
                      << F.getName() << "\n");

    // Create a clone of the whole function body.
    SmallVector<BasicBlock *, 32> OrigBBs;
    SmallVector<BasicBlock *, 32> CloneBBs;
    Value2CloneMapTy VMap;

    for (auto &BB : F)
      OrigBBs.push_back(&BB);
    for (auto *OBB : OrigBBs) {
      auto *CBB = CloneBasicBlock(OBB, VMap, ".clone", &F);
      VMap[OBB] = CBB;
      CloneBBs.push_back(CBB);
    }
    remapInstructionsInBlocks(CloneBBs, VMap);

    // Create a new entry block with branching code which transfers control to
    // the 'true'/'false' specialization of the function body.
    auto *OldEntryBB = &F.getEntryBlock();
    auto *NewEntryBB = BasicBlock::Create(F.getParent()->getContext(),
                                          "entry.new", &F, OldEntryBB);

    IRBuilder<> Builder(NewEntryBB);
    // All GEPs in the closure are expected to be the same, so we can clone any
    // of them (for example the first).
    auto *GEP = Builder.Insert<>(C.getGEPs().front().first->clone());
    auto *Load = Builder.CreateLoad(GEP);
    auto *ICmp = Builder.CreateICmp(ICmpInst::ICMP_NE, Load,
                                    ConstantInt::get(Load->getType(), 0u));
    auto *Br =
        Builder.CreateCondBr(ICmp, OldEntryBB, VMap.getClone(OldEntryBB));

    // Fixup conditional uses for 'true' and 'false' specializations. Replace
    // GEPs and Loads with the GEP and Load which we have added to the entry.
    auto *True = ConstantInt::getTrue(F.getParent()->getContext());
    auto *False = ConstantInt::getFalse(F.getParent()->getContext());

    uint64_t TrueWeight = 0;
    uint64_t FalseWeight = 0;
    MDBuilder MDB(F.getContext());
    for (const auto &GEPNode : C.getGEPs()) {
      for (const auto &LoadNode : GEPNode.second) {
        for (auto &CmpNode : LoadNode.second) {
          auto *Inst = CmpNode.first;
          auto *Clone = VMap.getClone(Inst);

          auto Pred = Inst->getPredicate();
          assert(ICmpInst::isEquality(Pred));

          // Collect the profile counts to enable setting the branch weights on
          // the version selection branch instruction. The version selection
          // branch always uses not-equal, update and accumulate the weights
          // based on which way the branches will be revised below.
          bool PredIsNE = Pred == ICmpInst::ICMP_NE;
          for (auto *U : Inst->users()) {
            if (auto *BrInst = dyn_cast<BranchInst>(U)) {
              if (Optional<std::pair<uint64_t, uint64_t>> Weights =
                      getBranchWeights(BrInst)) {
                auto *BrInstClone = VMap.getClone(BrInst);
                assert(BrInstClone && "Branch should have a clone");

                // The version selection branch always uses "not equal", update
                // and accumulate the weights based on which way the branches
                // will be revised below.
                TrueWeight += PredIsNE ? Weights.getValue().first
                                       : Weights.getValue().second;
                FalseWeight += PredIsNE ? Weights.getValue().second
                                        : Weights.getValue().first;

                // Update the profiling data on the branches. The conditional
                // will eventually be discarded, so this is not strictly
                // necessary, but do it to try to keep the profile info
                // in a consistent state.
                if (PredIsNE) {
                  // The original branch will become 'if i1 true ...'
                  // The clone branch will become 'if i1 false ...'
                  BrInst->setMetadata(
                      LLVMContext::MD_prof,
                      MDB.createBranchWeights(Weights.getValue().first, 0));
                  BrInstClone->setMetadata(
                      LLVMContext::MD_prof,
                      MDB.createBranchWeights(0, Weights.getValue().second));
                } else {
                  // The original branch will become 'if i1 false ...'
                  // The clone branch will become 'if i1 true ...'
                  BrInst->setMetadata(
                      LLVMContext::MD_prof,
                      MDB.createBranchWeights(0, Weights.getValue().second));
                  BrInstClone->setMetadata(
                      LLVMContext::MD_prof,
                      MDB.createBranchWeights(Weights.getValue().first, 0));
                }
              }
            }
          }

          Inst->replaceAllUsesWith(Pred == ICmpInst::ICMP_NE ? True : False);
          Inst->eraseFromParent();

          Clone->replaceAllUsesWith(Pred == ICmpInst::ICMP_NE ? False : True);
          Clone->eraseFromParent();
        }

        auto *Inst = LoadNode.first;
        auto *Clone = VMap.getClone(Inst);

        Inst->replaceAllUsesWith(Load);
        Inst->eraseFromParent();

        Clone->replaceAllUsesWith(Load);
        Clone->eraseFromParent();
      }

      auto *Inst = GEPNode.first;
      auto *Clone = VMap.getClone(Inst);

      Inst->replaceAllUsesWith(GEP);
      Inst->eraseFromParent();

      Clone->replaceAllUsesWith(GEP);
      Clone->eraseFromParent();
    }

    // Set the profile weight for the branch the controls the version
    // selection as a percentage of the function entry count.
    Function::ProfileCount EntryCount = F.getEntryCount();
    if (EntryCount.hasValue() && (TrueWeight != 0 || FalseWeight != 0)) {
      uint64_t TotalWeight = TrueWeight + FalseWeight;
      Br->setMetadata(
          LLVMContext::MD_prof,
          MDB.createBranchWeights(
              ((float)TrueWeight / TotalWeight) * EntryCount.getCount(),
              ((float)FalseWeight / TotalWeight) * EntryCount.getCount()));
    }

    // Update statistics.
    NumMultiVersionedFunctions++;
  }

private:
  Function &F;
  AAResults &AAR;
  TargetTransformInfo &TTI;

public:
  BoolMultiVersioningImpl(Function &F, AAResults &AAR,
      TargetTransformInfo &TTI) : F(F), AAR(AAR), TTI(TTI) {}

  bool run() const {

    // If AVX2 or higher is not present, then don't run optimization
    if (!TTI.isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2)) {
      return false;
    }

    // Build closures for all function arguments.
    SmallVector<BoolClosure, 8u> Closures;
    for (const auto &Arg : F.args())
      buildClosures(Arg, Closures);
    if (Closures.empty())
      return false;

    // Select the largest closure for multi-versioning.
    // TODO: we can do multi-versioning more than once if we have multiple
    // closures. Add support for this in future.
    size_t Idx = 0u;
    for (size_t I = 1u; I < Closures.size(); ++I)
      if (Closures[Idx].getNumUses() < Closures[I].getNumUses())
        Idx = I;

    // And finally multi-version function using the largest closure.
    doMultiVersioning(Closures[Idx]);
    return true;
  }
};

} // end of anonymous namespace

PreservedAnalyses MultiVersioningPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  auto &AAR = AM.getResult<AAManager>(F);
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);

  if (!BoolMultiVersioningImpl(F, AAR, TTI).run())
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<TargetIRAnalysis>();
  PA.preserve<AndersensAA>();
  PA.preserve<InlineAggAnalysis>();

  return PA;
}

namespace {

class MultiVersioningWrapper : public FunctionPass {
public:
  static char ID;

  MultiVersioningWrapper() : FunctionPass(ID) {
    initializeMultiVersioningWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto &AAR = getAnalysis<AAResultsWrapperPass>().getAAResults();
    auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

    return BoolMultiVersioningImpl(F, AAR, TTI).run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addPreserved<AAResultsWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<InlineAggressiveWrapperPass>();
    AU.addPreserved<TargetTransformInfoWrapperPass>();
    getAAResultsAnalysisUsage(AU);
  }
};

} // end of anonymous namespace

char MultiVersioningWrapper::ID = 0;

INITIALIZE_PASS_BEGIN(MultiVersioningWrapper, DEBUG_TYPE,
                      "Function multi-versioning", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(MultiVersioningWrapper, DEBUG_TYPE,
                    "Function multi-versioning", false, false)

FunctionPass *llvm::createMultiVersioningWrapperPass() {
  return new MultiVersioningWrapper();
}
