// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

#include "AutorunReplicator.h"

#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel {

char AutorunReplicator::ID = 0;
OCL_INITIALIZE_PASS(
    AutorunReplicator, "autorun-replicator",
    "creates copies of autorun kernels requested by num_compute_units kernel"
    "attribute and resolves get_compute_id built-in",
    /* Only looks at CFG */true, /* Analysis Pass */false)

AutorunReplicator::AutorunReplicator() : ModulePass(ID) {}

bool AutorunReplicator::runOnModule(Module &M) {
  bool hasChanges = false;

  auto Kernels = KernelList(M);
  SmallVector<Function *, 32> Replicas;
  for (auto *Kernel: Kernels) {
    auto kmd = KernelMetadataAPI(Kernel);

    if (kmd.Autorun.hasValue() && kmd.Autorun.get() &&
        kmd.NumComputeUnits.hasValue()) {
      hasChanges |= createReplicas(Kernel, Replicas);
    }
  }

  auto KList = Kernels.getList();
  for (auto *Kernel: Replicas) {
    KList.push_back(Kernel);
  }
  Kernels.set(KList);

  Function *GetComputeID = nullptr;
  for (auto &F : M) {
    if (!F.getName().equals("get_compute_id")) {
      continue;
    }

    SmallVector<CallInst *, 8> GetComputeIDCalls;
    for (auto *U: F.users()) {
      if (auto *GetComputeIDCall = dyn_cast<CallInst>(U)) {
        GetComputeIDCalls.push_back(GetComputeIDCall);
      }
    }

    for (auto *Call: GetComputeIDCalls) {
      resolveGetComputeID(Call);
    }

    GetComputeID = &F;
    hasChanges = true;
    break;
  }

  if (GetComputeID) {
    GetComputeID->eraseFromParent();
  }

  return hasChanges;
}

bool AutorunReplicator::createReplicas(Function *F,
                                       SmallVectorImpl<Function *> &Replicas) {
  auto kmd = KernelMetadataAPI(F);
  // assume original function as first replica
  m_ComputeIDs[F] = ComputeID(0, 0, 0);

  for (int x = 0; x < kmd.NumComputeUnits.getXDim(); ++x) {
    for (int y = 0; y < kmd.NumComputeUnits.getYDim(); ++y) {
      for (int z = 0; z < kmd.NumComputeUnits.getZDim(); ++z) {
        // skip first replica
        if (x == 0 && y == 0 && z == 0) {
          continue;
        }

        ValueToValueMapTy VMap;
        Function *Cloned = CloneFunction(F, VMap);
        assert(Cloned && "Failed to clone function");
        m_ComputeIDs[Cloned] = ComputeID(x, y, z);

        auto kimd = KernelInternalMetadataAPI(Cloned);

        if (kimd.VectorizedKernel.hasValue() && kimd.VectorizedKernel.get()) {
          ValueToValueMapTy VecVMap;
          Function *VecCloned = CloneFunction(F, VecVMap);
          assert(VecCloned && "Failed to clone function");

          auto vkimd = KernelInternalMetadataAPI(VecCloned);
          kimd.VectorizedKernel.set(VecCloned);
          vkimd.ScalarizedKernel.set(Cloned);
        }

        Replicas.push_back(Cloned);
      }
    }
  }

  return ((kmd.NumComputeUnits.getXDim() + kmd.NumComputeUnits.getYDim() +
           kmd.NumComputeUnits.getZDim()) > 3);
}

void AutorunReplicator::resolveGetComputeID(CallInst *GetComputeIDCall) {
  assert(GetComputeIDCall && "GetComputeIDCall is nullptr");
  assert(GetComputeIDCall->getCalledFunction() &&
         "getCalledFunction is nullptr");

  ComputeID ID = m_ComputeIDs[GetComputeIDCall->getFunction()];
  if (auto *Dim = dyn_cast<ConstantInt>(GetComputeIDCall->getArgOperand(0))) {
    uint64_t DimIndex = Dim->getZExtValue();
    int Result;
    if (DimIndex == 0) {
      Result = ID.XDimValue;
    } else if (DimIndex == 1) {
      Result = ID.YDimValue;
    } else if (DimIndex == 2) {
      Result = ID.ZDimValue;
    } else {
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

} // namespace intel

extern "C" {
  ModulePass *createAutorunReplicatorPass() {
    return new intel::AutorunReplicator();
  }
}
