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

#include "LoopUtils/LoopUtils.h"
#include "MetadataAPI.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"

using namespace Intel::MetadataAPI;

namespace intel {

  char ReplaceScalarWithMask::ID = 0;

  bool ReplaceScalarWithMask::runOnModule(Module& module) {
    bool changed = false;
    m_util.init(&module);
    LLVMContext* pContext = &module.getContext();
    auto kernels = KernelList(&module);

    for (auto pFunc : kernels) {
      auto skimd = KernelInternalMetadataAPI(pFunc);
      if ((skimd.NoBarrierPath.hasValue() && skimd.NoBarrierPath.get()) ||
          !skimd.VectorizedMaskedKernel.hasValue())
        continue;
      changed = true;
      auto maskedKernel = skimd.VectorizedMaskedKernel.get();
      auto maskedKimd = KernelInternalMetadataAPI(maskedKernel);
      // Set the vectorized width
      unsigned vectWidth = maskedKimd.VectorizedWidth.get();

      // Save the relevant information from the vectorized kernel in skimd
      // Prior to erasing this information
      unsigned int vectorizeOnDim = maskedKimd.VectorizationDimension.get();
      unsigned int canUniteWG = maskedKimd.CanUniteWorkgroups.get();

      // Update metadata before inlining the masked kernel.
      skimd.VectorizedMaskedKernel.set(nullptr);
      skimd.VectorizedWidth.set(vectWidth);
      skimd.VectorizationDimension.set(vectorizeOnDim);
      skimd.CanUniteWorkgroups.set(canUniteWG);

      // Prepare mask argument.
      auto *pEntry = &maskedKernel->getEntryBlock();
      auto *pNewEntry = BasicBlock::Create(*pContext, "", maskedKernel, pEntry);
      Value* sgSize = m_util.createGetSGSize(pNewEntry);
      Type* IndTy = LoopUtils::getIndTy(&module);
      Instruction* loopLen = new ZExtInst(sgSize, IndTy, "", pNewEntry);
      Value* mask = LoopUtils::generateRemainderMask(vectWidth, loopLen, pNewEntry);
      BranchInst::Create(pEntry, pNewEntry);
      Value* maskArg = maskedKernel->arg_end()-1;
      maskArg->replaceAllUsesWith(mask);

      // Inline the masked kernel to scalar kernel.
      LoopUtils::inlineMaskToScalar(pFunc, maskedKernel);
    }
    return changed;
  }

} // namespace intel

extern "C" {
  void* createReplaceScalarWithMaskPass() {
    return new intel::ReplaceScalarWithMask();
  }
}
