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

#include "ReplaceScalarWithMaskPass.h"

#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace DPCPPKernelMetadataAPI;

namespace intel {

  char ReplaceScalarWithMask::ID = 0;

  // Register ReplaceScalarWithMask Pass for oclopt.
  OCL_INITIALIZE_PASS(ReplaceScalarWithMask, "B-ReplaceScalarWithMask",
    "Barrier Pass - Replace scalar kernel with masked kernel when the \
kernel has barrier path and subgroup calls", false, false)

  bool ReplaceScalarWithMask::runOnModule(Module &M) {
    bool Changed = false;

    SGHelper Helper;
    Helper.initialize(M);

    for (auto *ScalarKernel : KernelList(&M)) {
      if (ScalarKernel->hasOptNone())
        continue;
      auto SKIMD = KernelInternalMetadataAPI(ScalarKernel);
      if ((SKIMD.NoBarrierPath.hasValue() && SKIMD.NoBarrierPath.get()) ||
          !(SKIMD.VectorizedMaskedKernel.hasValue() &&
            SKIMD.VectorizedMaskedKernel.get()))
        continue;

      Changed = true;

      Function *MaskKernel = SKIMD.VectorizedMaskedKernel.get();
      assert(SKIMD.VectorizedKernel.hasValue() &&
             "vectoized kernel doesn't exist!");
      Function *VectorKernel = SKIMD.VectorizedKernel.get();

      // Will insert all mask generation logic before first instruction.
      auto *FistInst = &*MaskKernel->getEntryBlock().begin();

      // Get current sub-group size.
      Value *SGSize = Helper.createGetSubGroupSize(FistInst);
      Type *IndTy = LoopUtils::getIndTy(&M);
      if (SGSize->getType() != IndTy) {
        SGSize = new ZExtInst(SGSize, IndTy, "sg.size.zext", FistInst);
        cast<Instruction>(SGSize)->setDebugLoc(FistInst->getDebugLoc());
      }

      // Generate mask and replace the mask argument with generated mask.
      Value *Mask = LoopUtils::generateRemainderMask(
          KernelInternalMetadataAPI(MaskKernel).VectorizedWidth.get(), SGSize,
          FistInst);
      (MaskKernel->arg_end() - 1)->replaceAllUsesWith(Mask);

      // Replace the scalar kernel body with masked kernel body.
      ScalarKernel->deleteBody();
      ScalarKernel->getBasicBlockList().splice(ScalarKernel->begin(),
                                               MaskKernel->getBasicBlockList());

      // Move the name and users of arguments to the new version.
      for (Function::arg_iterator I = ScalarKernel->arg_begin(),
                                  E = ScalarKernel->arg_end(),
                                  I2 = MaskKernel->arg_begin();
           I != E; ++I, ++I2) {
        I2->replaceAllUsesWith(&*I);
        I->takeName(&*I2);
      }

      // Clone metadatas from the old function, including debug info descriptor.
      SmallVector<std::pair<unsigned, MDNode *>, 1> MDs;
      MaskKernel->getAllMetadata(MDs);
      for (auto MD : MDs)
        ScalarKernel->addMetadata(MD.first, *MD.second);

      // Set the masked kernel to itself.
      SKIMD.VectorizedMaskedKernel.set(ScalarKernel);
      // Unset scalar kernel.
      SKIMD.ScalarKernel.set(nullptr);
      // Restore vectorized kernel.
      SKIMD.VectorizedKernel.set(VectorKernel);

      // Remove mask kernel.
      MaskKernel->eraseFromParent();
    }

    return Changed;
  }

} // namespace intel

extern "C" {
  void* createReplaceScalarWithMaskPass() {
    return new intel::ReplaceScalarWithMask();
  }
}
