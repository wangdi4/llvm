//===-- AutorunReplicator.cpp - Autorun Replicator ------------------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AutorunReplicator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Metadata.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

namespace {

class AutorunReplicatorLegacy : public ModulePass {
public:
  static char ID;

  AutorunReplicatorLegacy();

  StringRef getPassName() const override { return "AutorunReplicator"; }

  bool runOnModule(Module &M) override;

private:
  AutorunReplicatorPass Impl;
};

} // namespace

char AutorunReplicatorLegacy::ID = 0;

INITIALIZE_PASS(
    AutorunReplicatorLegacy, "dpcpp-kernel-autorun-replicator",
    "creates copies of autorun kernels requested by num_compute_units kernel"
    "attribute and resolves get_compute_id built-in",
    false, false)

AutorunReplicatorLegacy::AutorunReplicatorLegacy() : ModulePass(ID) {
  initializeAutorunReplicatorLegacyPass(*PassRegistry::getPassRegistry());
}

bool AutorunReplicatorLegacy::runOnModule(Module &M) { return Impl.runImpl(M); }

PreservedAnalyses AutorunReplicatorPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool AutorunReplicatorPass::runImpl(Module &M) {
  bool HasChanges = false;

  auto Kernels = DPCPPKernelMetadataAPI::KernelList(M);
  SmallVector<Function *, 32> Replicas;
  for (auto *Kernel : Kernels) {
    auto Kmd = DPCPPKernelMetadataAPI::KernelMetadataAPI(Kernel);

    if (Kmd.Autorun.hasValue() && Kmd.Autorun.get() &&
        Kmd.NumComputeUnits.hasValue())
      HasChanges |= createReplicas(Kernel, Replicas);
  }

  auto KList = Kernels.getList();
  for (auto *Kernel : Replicas)
    KList.push_back(Kernel);

  Kernels.set(KList);

  Function *GetComputeID = nullptr;
  for (auto &F : M) {
    if (!F.getName().equals("get_compute_id"))
      continue;

    SmallVector<CallInst *, 8> GetComputeIDCalls;
    for (auto *U : F.users())
      if (auto *GetComputeIDCall = dyn_cast<CallInst>(U))
        GetComputeIDCalls.push_back(GetComputeIDCall);

    for (auto *Call : GetComputeIDCalls)
      resolveGetComputeID(Call);

    GetComputeID = &F;
    HasChanges = true;
    break;
  }

  if (GetComputeID)
    GetComputeID->eraseFromParent();

  return HasChanges;
}

bool AutorunReplicatorPass::createReplicas(
    Function *F, SmallVectorImpl<Function *> &Replicas) {
  auto Kmd = DPCPPKernelMetadataAPI::KernelMetadataAPI(F);
  // assume original function as first replica
  ComputeIDs.insert(std::make_pair(F, ComputeID(0, 0, 0)));

  for (int X = 0; X < Kmd.NumComputeUnits.getXDim(); ++X) {
    for (int Y = 0; Y < Kmd.NumComputeUnits.getYDim(); ++Y) {
      for (int Z = 0; Z < Kmd.NumComputeUnits.getZDim(); ++Z) {
        // skip first replica
        if (X == 0 && Y == 0 && Z == 0)
          continue;

        ValueToValueMapTy VMap;
        Function *Cloned = CloneFunction(F, VMap);
        assert(Cloned && "Failed to clone function");
        ComputeIDs.insert(std::make_pair(Cloned, ComputeID(X, Y, Z)));

        auto Kimd = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(Cloned);

        if (Kimd.VectorizedKernel.hasValue() && Kimd.VectorizedKernel.get()) {
          ValueToValueMapTy VecVMap;
          Function *VecCloned = CloneFunction(F, VecVMap);
          assert(VecCloned && "Failed to clone function");

          auto Vkimd =
              DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(VecCloned);
          Kimd.VectorizedKernel.set(VecCloned);
          Vkimd.ScalarKernel.set(Cloned);
        }

        Replicas.push_back(Cloned);
      }
    }
  }

  return ((Kmd.NumComputeUnits.getXDim() + Kmd.NumComputeUnits.getYDim() +
           Kmd.NumComputeUnits.getZDim()) > 3);
}

void AutorunReplicatorPass::resolveGetComputeID(CallInst *GetComputeIDCall) {
  assert(GetComputeIDCall && "GetComputeIDCall is nullptr");
  assert(GetComputeIDCall->getCalledFunction() &&
         "getCalledFunction is nullptr");
  auto Iter = ComputeIDs.find(GetComputeIDCall->getFunction());
  assert(Iter != ComputeIDs.end() && "Cannot find function after clone");
  ComputeID ID = Iter->getSecond();
  if (auto *Dim = dyn_cast<ConstantInt>(GetComputeIDCall->getArgOperand(0))) {
    uint64_t DimIndex = Dim->getZExtValue();
    int Result;
    if (DimIndex == 0)
      Result = ID.XDimValue;
    else if (DimIndex == 1)
      Result = ID.YDimValue;
    else if (DimIndex == 2)
      Result = ID.ZDimValue;
    else {
      // TODO: ask PSG about get_compute_id behavior in case of invalid index of
      // the dimension. Currently let's return 0 like in get_global_id
      Result = 0;
    }

    BasicBlock::iterator II(GetComputeIDCall);
    IntegerType *ComputeIDTy = cast<IntegerType>(
        GetComputeIDCall->getCalledFunction()->getReturnType());
    ReplaceInstWithValue(GetComputeIDCall->getParent()->getInstList(), II,
                         ConstantInt::get(ComputeIDTy, Result));
  } else {
    llvm_unreachable("Non-constant arg passed to get_compute_id");
  }
}

ModulePass *llvm::createAutorunReplicatorLegacyPass() {
  return new AutorunReplicatorLegacy();
}
