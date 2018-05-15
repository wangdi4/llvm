//===---------------- AOSToSOA.cpp - DTransAOStoSOAPass -------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/AOSToSOA.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-aostosoa"

namespace {

class DTransAOSToSOAWrapper : public ModulePass {
private:
  dtrans::AOSToSOAPass Impl;

public:
  static char ID;

  DTransAOSToSOAWrapper() : ModulePass(ID) {
    initializeDTransAOSToSOAWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto &DTInfo = getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

    // This lambda function is to allow getting the DominatorTree analysis for a
    // specific function to allow analysis of loops for the dynamic allocation
    // of the structure.
    dtrans::AOSToSOAPass::DominatorTreeFuncType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    return Impl.runImpl(M, DTInfo, TLI, GetDT);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransAOSToSOAWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransAOSToSOAWrapper, "dtrans-aostosoa",
                      "DTrans array of structs to struct of arrays", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(DTransAOSToSOAWrapper, "dtrans-aostosoa",
                    "DTrans array of structs to struct of arrays", false, false)

ModulePass *llvm::createDTransAOSToSOAWrapperPass() {
  return new DTransAOSToSOAWrapper();
}

namespace llvm {
namespace dtrans {

bool AOSToSOAPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           const TargetLibraryInfo &TLI,
                           AOSToSOAPass::DominatorTreeFuncType &GetDT) {
  bool Changed = false;

  // Check whether there are any candidate structures that can be transformed.
  StructInfoVec CandidateTypes;
  gatherCandidateTypes(DTInfo, CandidateTypes);
  qualifyCandidates(CandidateTypes, DTInfo, GetDT);

  // TODO: Implement the optimization here.

  return Changed;
}

// Populate the \p CandidateTypes vector with all the structure types
// that meet the minimum safety conditions to be considered for transformation.
void AOSToSOAPass::gatherCandidateTypes(DTransAnalysisInfo &DTInfo,
                                        StructInfoVecImpl &CandidateTypes) {
  const dtrans::SafetyData AOSToSOASafetyConditions =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP | dtrans::VolatileData |
      dtrans::MismatchedElementAccess | dtrans::WholeStructureReference |
      dtrans::UnsafePointerStore | dtrans::FieldAddressTaken |
      dtrans::GlobalInstance | dtrans::HasInitializerList |
      dtrans::UnsafePtrMerge | dtrans::BadMemFuncSize |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::AddressTaken | dtrans::NoFieldsInStruct | dtrans::NestedStruct |
      dtrans::ContainsNestedStruct | dtrans::SystemObject |
      dtrans::LocalInstance;

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    if (TI->testSafetyData(AOSToSOASafetyConditions)) {
      DEBUG(dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported safety data: "
                   << TI->getLLVMType()->getStructName() << "\n");
      continue;
    }

    CandidateTypes.push_back(cast<StructInfo>(TI));
  }
}

// This routine examines all the candidates and performs additional safety
// checks on the type and usage to determine whether a type is supported for
// being transformed. The \p CandidateTypes list will be updated to only contain
// the elements that pass all the safety checks.
void AOSToSOAPass::qualifyCandidates(
    StructInfoVecImpl &CandidateTypes, DTransAnalysisInfo &DTInfo,
    AOSToSOAPass::DominatorTreeFuncType &GetDT) {
  if (!qualifyCandidatesTypes(CandidateTypes, DTInfo))
    return;

  if (!qualifyAllocations(CandidateTypes, DTInfo, GetDT))
    return;

  if (!qualifyHeuristics(CandidateTypes, DTInfo))
    return;

  DEBUG({
    for (auto *Candidate : CandidateTypes)
      dbgs() << "DTRANS-AOSTOSOA: Passed qualification tests: "
             << Candidate->getLLVMType()->getStructName() << "\n";
  });
}

// Check for any types that are not supported for the transformation.
// 1. Types that are used as arrays are not supported, for example
//     [4 x struct.test], because we would need to handle all the allocation
//     checks and transformation code for these arrays, as well.
// 2. Types that contain arrays are not supported. This restriction could be
//     relaxed in a future version.
// 3. Types that contain vectors are not supported.
//
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyCandidatesTypes(StructInfoVecImpl &CandidateTypes,
                                          DTransAnalysisInfo &DTInfo) {
  // Collect a set of structure types that are arrays composed of structure
  // types so that we can check if any of these match the candidate types.
  SmallPtrSet<dtrans::StructInfo *, 4> ArrayElemTypes;
  for (auto *TI : DTInfo.type_info_entries()) {
    if (!isa<dtrans::ArrayInfo>(TI))
      continue;

    Type *ElemTy = TI->getLLVMType()->getArrayElementType();
    while (isa<ArrayType>(ElemTy))
      ElemTy = ElemTy->getArrayElementType();

    if (!isa<StructType>(ElemTy))
      continue;

    auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(ElemTy));
    ArrayElemTypes.insert(StInfo);
  }

  StructInfoVec Qualified;
  for (auto *Candidate : CandidateTypes) {
    if (ArrayElemTypes.find(Candidate) != ArrayElemTypes.end()) {
      DEBUG(dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Array of type seen: "
                   << Candidate->getLLVMType()->getStructName() << "\n");
      continue;
    }

    // No arrays of the structure type were found, now check the structure
    // field types to verify all members are supported for the transformation.
    // Reject any that contain arrays or vectors. We don't need to check for
    // structures because those were rejected by the safety checks.
    bool Supported = true;
    for (auto &FI : Candidate->getFields()) {
      Type *Ty = FI.getLLVMType();
      if (Ty->isArrayTy() || Ty->isVectorTy()) {
        Supported = false;
        break;
      }
    }

    if (Supported)
      Qualified.push_back(Candidate);
    else
      DEBUG(dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported structure "
                      "element type: "
                   << Candidate->getLLVMType()->getStructName() << "\n");
  }

  std::swap(CandidateTypes, Qualified);
  return !CandidateTypes.empty();
}

// Check that the type is only allocated once by malloc or calloc.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyAllocations(StructInfoVecImpl &CandidateTypes,
                                      DTransAnalysisInfo &DTInfo,
                                      DominatorTreeFuncType &GetDT) {
  // Build a mapping from each allocated type to a single allocating
  // instruction, if one exists. If there are multiple allocations or an
  // unsupported allocation, map the type to 'nullptr'.
  DenseMap<dtrans::StructInfo *, Instruction *> TypeToAllocInstr;
  for (auto *Call : DTInfo.call_info_entries()) {
    auto *ACI = dyn_cast<dtrans::AllocCallInfo>(Call);
    if (!ACI || !ACI->getAliasesToAggregatePointer())
      continue;

    // We do not support transforming any allocations that are not
    // calloc/malloc. Invalidate the information for all types.
    if (ACI->getAllocKind() != dtrans::AK_Calloc &&
        ACI->getAllocKind() != dtrans::AK_Malloc) {
      for (auto *AllocatedTy : ACI->getPointerTypeInfoRef().getTypes()) {
        auto *Ty = AllocatedTy->getPointerElementType();
        auto *TI = DTInfo.getTypeInfo(Ty);
        if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
          DEBUG({
            if (std::find(CandidateTypes.begin(), CandidateTypes.end(),
                          StInfo) != CandidateTypes.end() &&
                (!TypeToAllocInstr.count(StInfo) ||
                 TypeToAllocInstr[StInfo] != nullptr))
              dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported "
                        "allocation function: "
                     << Ty->getStructName() << "\n"
                     << "  " << *ACI->getInstruction() << "\n";
          });

          TypeToAllocInstr[StInfo] = nullptr;
        }
      }

      continue;
    }

    // For supported allocations, update the association between the type and
    // allocating instruction.
    for (auto *AllocatedTy : ACI->getPointerTypeInfoRef().getTypes()) {
      auto *Ty = AllocatedTy->getPointerElementType();
      auto *TI = DTInfo.getTypeInfo(Ty);
      if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
        if (TypeToAllocInstr.count(StInfo)) {
          DEBUG({
            if (std::find(CandidateTypes.begin(), CandidateTypes.end(),
                          StInfo) != CandidateTypes.end() &&
                TypeToAllocInstr[StInfo] != nullptr)
              dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Too many allocations: "
                     << Ty->getStructName() << "\n";
          });
          TypeToAllocInstr[StInfo] = nullptr;
          continue;
        }

        TypeToAllocInstr[StInfo] = ACI->getInstruction();
      }
    }
  }

  // Select the types that passed the single allocation location test. Also,
  // populate a set of instructions  by function that need to be checked to
  // verify the allocation is not within a loop. We group these by function so
  // that the LoopInfo for a function only needs to be calculated one time.
  //
  // Note: Currently this does not reject a type if there is no dynamic
  // allocation of the  type. This may need to be revisited when implementing
  // the transformation.
  StructInfoVec Qualified;
  DenseMap<Function *, DenseSet<std::pair<Instruction *, dtrans::StructInfo *>>>
      AllocPathMap;
  SmallVector<std::pair<Function *, Instruction *>, 4> CallChain;
  for (auto *TyInfo : CandidateTypes) {
    if (TypeToAllocInstr.count(TyInfo)) {
      if (TypeToAllocInstr[TyInfo] == nullptr)
        continue;

      // Verify the call chain to the instruction consists of a single path
      Instruction *I = TypeToAllocInstr[TyInfo];
      CallChain.clear();
      if (!collectCallChain(I, CallChain)) {
        dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Multiple call paths: "
               << TyInfo->getLLVMType()->getStructName() << "\n";
        continue;
      }

      // Save the instruction and all it's caller to the list of locations that
      // will need to be checked for being within loops.
      AllocPathMap[I->getParent()->getParent()].insert(
          std::make_pair(I, TyInfo));
      for (auto &FuncInstrPair : CallChain)
        AllocPathMap[FuncInstrPair.first].insert(
            std::make_pair(FuncInstrPair.second, TyInfo));

      Qualified.push_back(TyInfo);
    }
  }

  std::swap(CandidateTypes, Qualified);
  if (CandidateTypes.empty())
    return false;

  // check the function's loops to see if the allocation (or call to the
  // allocation) instruction is within a loop
  for (auto &FuncToAllocPath : AllocPathMap) {
    Function *F = FuncToAllocPath.first;
    DominatorTree &DT = (GetDT)(*F);
    LoopInfo LI(DT);

    if (LI.size())
      for (auto &InstTypePair : FuncToAllocPath.second)
        if (LI.getLoopFor(InstTypePair.first->getParent())) {
          StructInfo *StInfo = InstTypePair.second;
          DEBUG(dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Allocation in loop: "
                       << StInfo->getLLVMType()->getStructName()
                       << "\n  Function: " << F->getName() << "\n");
          auto *It =
              std::find(CandidateTypes.begin(), CandidateTypes.end(), StInfo);
          if (It != CandidateTypes.end())
            CandidateTypes.erase(It);
        }
  }

  return !CandidateTypes.empty();
}

// If there is a single call chain that reaches the instruction, \p I,
// add the function to the \p CallChain, and return 'true'. Otherwise, return
// 'false'.
bool AOSToSOAPass::collectCallChain(
    Instruction *I,
    SmallVectorImpl<std::pair<Function *, Instruction *>> &CallChain) {
  Function *F = I->getParent()->getParent();
  Instruction *Callsite = nullptr;

  for (auto *U : F->users()) {
    if (auto *Call = dyn_cast<CallInst>(*&U)) {
      // Check that we only have a single call path to the routine.
      if (Callsite != nullptr)
        return false;

      Callsite = Call;
    } else {
      return false;
    }
  }

  // Verify the top of the callchain is the 'main' routine
  if (!Callsite)
    return F->getName() == "main";

  CallChain.push_back(
      std::make_pair(Callsite->getParent()->getParent(), Callsite));
  return collectCallChain(Callsite, CallChain);
}

// Filter the \p CandidateTypes list based on whether the type meets
// the criteria of the profitability heuristics.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyHeuristics(StructInfoVecImpl &CandidateTypes,
                                     DTransAnalysisInfo &DTInfo) {
  // TODO: Implement checks for usage frequency to determine whether
  // a candidate should be converted.

  return !CandidateTypes.empty();
}

PreservedAnalyses AOSToSOAPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  DominatorTreeFuncType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  bool Changed = runImpl(M, DTransInfo, TLI, GetDT);

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

} // end namespace dtrans
} // end namespace llvm
