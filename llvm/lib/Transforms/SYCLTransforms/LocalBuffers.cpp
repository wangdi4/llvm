//=== LocalBuffers.cpp -  Map GlobalVariable __local to local buffer -========//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/LocalBuffers.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/ReplaceConstant.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-local-buffers"

PreservedAnalyses LocalBuffersPass::run(Module &M, ModuleAnalysisManager &AM) {
  LocalBufferInfo *LBInfo = &AM.getResult<LocalBufferAnalysis>(M);
  if (!runImpl(M, LBInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
bool LocalBuffersPass::runImpl(Module &M, LocalBufferInfo *LBInfo) {
  using namespace SYCLKernelMetadataAPI;

  this->M = &M;
  this->LBInfo = LBInfo;
  LBInfo->computeSize();
  const auto &LocalsMap = LBInfo->getDirectLocalsMap();
  if (LocalsMap.empty())
    return false;

  HasTLSGlobals = CompilationUtils::hasTLSGlobals(M);
  Context = &M.getContext();

  DIFinder = DebugInfoFinder();
  DIFinder.processModule(M);

  // Get all kernels
  auto KernelsFunctionSet = CompilationUtils::getAllKernels(M);
  if (KernelsFunctionSet.empty())
    return false;

  SmallVector<Constant *, 16> GVs;
  for (GlobalVariable &GV : M.globals())
    if (GV.getAddressSpace() == CompilationUtils::ADDRESS_SPACE_LOCAL)
      GVs.push_back(&GV);
  convertUsersOfConstantsToInstructions(GVs);

  // Run on all defined function in the module
  for (auto &Func : M) {
    Function *F = &Func;
    if (!F || F->isDeclaration()) {
      // Function is not defined inside module
      continue;
    }

    // pipes ctor is not a kernel
    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(F))
      continue;

    if (!LocalsMap.lookup(F).empty())
      runOnFunction(F);
    if (KernelsFunctionSet.count(F)) {
      // We have a kernel, update metadata
      KernelInternalMetadataAPI(F).LocalBufferSize.set(
          LBInfo->getLocalsSize(F));
    }
  }

  updateDICompileUnitGlobals();

  // Most of local variable GVs are already handled. Erase them.
  for (GlobalVariable &GV : make_early_inc_range(M.globals())) {
    if (cast<PointerType>(GV.getType())->getAddressSpace() ==
        CompilationUtils::ADDRESS_SPACE_LOCAL) {
      GV.removeDeadConstantUsers();
      // May still have use in another global variable.
      [[maybe_unused]] auto HasOnlyConstantUser = [](GlobalVariable &GV) {
        for (User *U : GV.users()) {
          for (auto It = df_begin(U), E = df_end(U); It != E; ++It)
            if (!isa<Constant>(*It))
              return false;
        }
        return true;
      };
      assert((GV.use_empty() || HasOnlyConstantUser(GV)) &&
             "local variable isn't handled");
      if (GV.use_empty() && GV.isDiscardableIfUnused())
        GV.eraseFromParent();
    }
  }

  return true;
}

void LocalBuffersPass::attachDebugInfoToLocalAddr(GlobalVariable *GV,
                                                  Value *LocalAddr) {
  SmallVector<DIGlobalVariableExpression *, 1> DIGVExprs;
  GV->getDebugInfo(DIGVExprs);

  DIBuilder DIB(*M, false);

  for (auto *DIGVExpr : DIGVExprs) {
    auto *DIGV = DIGVExpr->getVariable();
    if (DIGV->getScope() != SP)
      continue;
    auto *DIExpr = DIGVExpr->getExpression();
    DIExpr = DIExpression::prepend(DIExpr, DIExpression::DerefBefore);

    // Create a function-level Debug local variable
    auto *DILV = DIB.createAutoVariable(
        DIGV->getScope(), DIGV->getName(), DIGV->getFile(), DIGV->getLine(),
        DIGV->getType(), true, DINode::FlagArtificial);

    DIB.insertDbgValueIntrinsic(LocalAddr, DILV, DIExpr,
                                DILocation::get(*Context, 0, 0, SP),
                                InsertPoint);
  }

  GVEToRemove.insert(DIGVExprs.begin(), DIGVExprs.end());
}

void LocalBuffersPass::updateDICompileUnitGlobals() {
  SmallVector<Metadata *> LiveGVs;
  for (auto *DICU : DIFinder.compile_units()) {
    LiveGVs.clear();

    for (auto *DIG : DICU->getGlobalVariables()) {
      if (!GVEToRemove.count(DIG))
        LiveGVs.push_back(DIG);
    }

    DICU->replaceGlobalVariables(MDTuple::get(*Context, LiveGVs));
  }
}

// Substitutes a pointer to local buffer, with argument passed within kernel
// parameters
void LocalBuffersPass::parseLocalBuffers(Function *F, Value *LocalMem) {
  IRBuilder<> Builder(InsertPoint);
  const auto &LocalsMap = LBInfo->getDirectLocalsMap();

  // Iterate through local buffers directly used in F.
  for (GlobalVariable *GV : LocalsMap.lookup(F)) {
    // Retrieve the offset of the local buffer.
    size_t Offset = LBInfo->getLocalGVToOffset(GV);
    Type *Ty = CompilationUtils::getSLMBufferElementType(*Context);
    auto *Idx = ConstantInt::get(Type::getInt32Ty(*Context), Offset);
    auto *LocalAddr = Builder.CreateGEP(Ty, LocalMem, Idx, "");

    // Add bitcast to required/original pointer type.
    auto *Replacement = Builder.CreatePointerCast(LocalAddr, GV->getType());

    for (auto *U : make_early_inc_range(GV->users()))
      if (auto *I = dyn_cast<Instruction>(U); I && I->getFunction() == F)
        I->replaceUsesOfWith(GV, Replacement);

    if (SP)
      attachDebugInfoToLocalAddr(GV, LocalAddr);
  }
}

void LocalBuffersPass::runOnFunction(Function *F) {
  InsertPoint = &*inst_begin(F);
  SP = F->getSubprogram();

  // Getting the implicit arguments
  Value *LocalMem = 0;
  if (HasTLSGlobals) {
    LocalMem =
        CompilationUtils::getTLSGlobal(M, ImplicitArgsUtils::IA_SLM_BUFFER);
    assert(LocalMem && "TLS LocalMem is not found.");
    // Load the LocalMem pointer from TLS GlobalVariable
    IRBuilder<> Builder(InsertPoint);
    LocalMem =
        Builder.CreateLoad(cast<GlobalVariable>(LocalMem)->getValueType(),
                           LocalMem, "LocalMemBase");
  } else {
    CompilationUtils::getImplicitArgs(F, &LocalMem, nullptr, nullptr, nullptr,
                                      nullptr, nullptr);
  }

  assert(LocalMem && "LocalMem should not be nullptr.");
  parseLocalBuffers(F, LocalMem);
}
