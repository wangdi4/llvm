//===- LocalBufferAnalysis.cpp - DPC++ kernel local buffer analysis -------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LocalBufferAnalysis.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DevLimits.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-local-buffer-analysis"

namespace llvm {

class LocalBufferInfoImpl {
public:
  LocalBufferInfoImpl(Module *M, CallGraph *CG);

  using TUsedLocals = LocalBufferInfo::TUsedLocals;

  const TUsedLocals &getDirectLocals(Function *F) { return LocalUsageMap[F]; }

  size_t getDirectLocalsSize(Function *F) { return DirectLocalSizeMap[F]; }

  size_t getLocalsSize(Function *F) { return LocalSizeMap[F]; }

private:
  void analyze(CallGraph *CG);

  /// Adds the given local value to the set of used locals of all functions
  /// that are using the given user directly. It recursively searches the first
  /// useres (and users of a users) that are functions.
  /// \param LocalVal local value (which is represented by a global value
  /// with address space 3).
  /// \param U direct user of pLocalVal.
  void updateLocalsMap(GlobalValue *LocalVal, User *U);

  /// Goes over all local values in the module and over all their direct users
  /// and maps between functions and the local values they use.
  /// \param M the module which need to go over its local values.
  void updateDirectLocals(Module &M);

  /// calculate direct local sizes used by functions in the module.
  void calculateDirectLocalsSize();

  /// Iterate all functions in module by postorder traversal, and for each
  /// function, add direct local sizes with the max size of local buffer needed
  /// by all of callees.
  void calculateLocalsSize(CallGraph *CG);

  /// A mapping between function pointer and the set of local values the
  /// function uses directly.
  using TUsedLocalsMap = LocalBufferInfo::TUsedLocalsMap;

  /// A mapping between function pointer and the local buffer size that the
  /// function uses.
  using TLocalSizeMap = DenseMap<Function *, size_t>;

  /// The llvm module this pass needs to update.
  Module *M;

  /// Map between function and the local values it uses directly.
  TUsedLocalsMap LocalUsageMap;

  /// Map between function and the local buffer size.
  TLocalSizeMap LocalSizeMap;

  /// Map between function and the local buffer size for local values used
  /// directly by this function.
  TLocalSizeMap DirectLocalSizeMap;
};

} // namespace llvm

LocalBufferInfoImpl::LocalBufferInfoImpl(Module *M, CallGraph *CG) : M(M) {
  analyze(CG);
}

void LocalBufferInfoImpl::updateLocalsMap(GlobalValue *LocalVal, User *U) {
  // Instruction, Operator and Constant are the only possible subtypes of U
  if (isa<Instruction>(U)) {
    Instruction *I = cast<Instruction>(U);

    // declaring variables for debugging purposes shouldn't affect local
    // buffers.
    if (MDNode *mdn = I->getMetadata("dbg_declare_inst")) {
      if (mdconst::extract<ConstantInt>(mdn->getOperand(0))->isAllOnesValue()) {
        return;
      }
    }
    // Parent of Instruction is BasicBlock
    // Parent of BasicBlock is Function
    Function *F = I->getFunction();
    // Add LocalVal to the set of local values used by F
    LocalUsageMap[F].insert(LocalVal);
  } else if (isa<Constant>(U)) {
    // Recursievly locate all Us of the constant value
    for (auto It = U->user_begin(); It != U->user_end(); ++It) {
      updateLocalsMap(LocalVal, *It);
    }
  } else {
    // Operator is an internal llvm class, so we do not expect it to be a U
    // of GlobalValue.
    llvm_unreachable("Unexpected user type");
  }
}

void LocalBufferInfoImpl::updateDirectLocals(Module &M) {
  // Get a list of all the global values in the module
  Module::GlobalListType &Globals = M.getGlobalList();

  // Find globals that appear in the origin kernel as local variables and add
  // update mapping accordingly
  for (Module::GlobalListType::iterator It = Globals.begin(), E = Globals.end();
       It != E; ++It) {
    GlobalValue *Val = &*It;

    const PointerType *TP = cast<PointerType>(Val->getType());
    if (TP->getAddressSpace() != CompilationUtils::ADDRESS_SPACE_LOCAL) {
      // LOCL_VALUE_ADDRESS_SPACE = '3' is a magic number for global variables
      // that were in origin local kernel variable!
      continue;
    }

    // If we reached here, then Val is a global value that was originally a
    // local value.
    for (GlobalValue::user_iterator UI = Val->user_begin(),
                                    UE = Val->user_end();
         UI != UE; ++UI) {
      updateLocalsMap(Val, *UI);
    }
  } // Find globals done.
}

void LocalBufferInfoImpl::calculateDirectLocalsSize() {
  DataLayout DL(M);
  for (auto &F : *M) {
    size_t DirectLocalSize = 0;

    for (GlobalValue *LocalGV : LocalUsageMap[&F]) {
      assert(LocalGV &&
             "locals container contains something other than GlobalValue!");

      // Calculate required buffer size.
      size_t ArraySize = DL.getTypeAllocSize(LocalGV->getValueType());
      assert(0 != ArraySize && "local buffer size is zero!");

      // Advance total implicit size.
      DirectLocalSize += ADJUST_SIZE_TO_MAXIMUM_ALIGN(ArraySize);
    }

    // Update direct local size of this function.
    DirectLocalSizeMap[&F] = DirectLocalSize;
  }
}

void LocalBufferInfoImpl::calculateLocalsSize(CallGraph *CG) {
  calculateDirectLocalsSize();
  CompilationUtils::calculateMemorySizeWithPostOrderTraversal(
      *CG, DirectLocalSizeMap, LocalSizeMap);
}

void LocalBufferInfoImpl::analyze(CallGraph *CG) {
  LocalUsageMap.clear();
  LocalSizeMap.clear();
  DirectLocalSizeMap.clear();

  // Initialize localUsageMap
  updateDirectLocals(*M);

  calculateLocalsSize(CG);
}

LocalBufferInfo::LocalBufferInfo(Module *M, CallGraph *CG) {
  Impl.reset(new LocalBufferInfoImpl(M, CG));
}

LocalBufferInfo::LocalBufferInfo(LocalBufferInfo &&Other) {
  Impl = std::move(Other.Impl);
}

LocalBufferInfo &LocalBufferInfo::operator=(LocalBufferInfo &&Other) {
  Impl = std::move(Other.Impl);
  return *this;
}

LocalBufferInfo::~LocalBufferInfo() = default;

const LocalBufferInfo::TUsedLocals &
LocalBufferInfo::getDirectLocals(Function *F) {
  return Impl->getDirectLocals(F);
}

size_t LocalBufferInfo::getDirectLocalsSize(Function *F) {
  return Impl->getDirectLocalsSize(F);
}

size_t LocalBufferInfo::getLocalsSize(Function *F) {
  return Impl->getLocalsSize(F);
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey LocalBufferAnalysis::Key;

LocalBufferInfo LocalBufferAnalysis::run(Module &M,
                                         AnalysisManager<Module> &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  LocalBufferInfo WPAResult(&M, CG);

  return WPAResult;
}

INITIALIZE_PASS_BEGIN(LocalBufferAnalysisLegacy, DEBUG_TYPE,
                      "Provide local values analysis info", false, true)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(LocalBufferAnalysisLegacy, DEBUG_TYPE,
                    "Provide local values analysis info", false, true)

char LocalBufferAnalysisLegacy::ID = 0;

LocalBufferAnalysisLegacy::LocalBufferAnalysisLegacy() : ModulePass(ID) {
  initializeLocalBufferAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

bool LocalBufferAnalysisLegacy::runOnModule(Module &M) {
  CallGraph *CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
  LocalBufferInfo *LBAResult = new LocalBufferInfo(&M, CG);

  Result.reset(LBAResult);

  return false;
}

ModulePass *llvm::createLocalBufferAnalysisLegacyPass() {
  return new LocalBufferAnalysisLegacy();
}
