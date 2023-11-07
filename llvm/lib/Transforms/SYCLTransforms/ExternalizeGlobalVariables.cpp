//===--- ExternalizeGlobalVariables.cpp -----------------------------------===//
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

#include "llvm/Transforms/SYCLTransforms/ExternalizeGlobalVariables.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-externalize-global-variables"

// The corresponding SPIR-V OpCode for the host_access property is documented
// in the SPV_INTEL_global_variable_host_access design document:
// https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/INTEL/SPV_INTEL_global_variable_host_access.asciidoc#decoration
constexpr uint32_t SPIRV_HOST_ACCESS_DECOR = 6168;

static bool externalizeDeviceGlobal(Module *M) {
  bool Changed = false;
  for (auto &GV : M->globals()) {
    if (GV.isConstant())
      continue;

    if (MDNode *DecoMD = GV.getMetadata("spirv.Decorations")) {
      for (const MDOperand &MDOp : DecoMD->operands()) {
        MDNode *Node = dyn_cast<MDNode>(MDOp);
        // Check if it's a device global has spirv host access decoration
        if (Node && Node->getNumOperands() == 3 &&
            mdconst::extract<ConstantInt>(Node->getOperand(0))
                    ->getZExtValue() == SPIRV_HOST_ACCESS_DECOR) {

          // Reset linkage type to external to make sure it will not be
          // optimized out and can be accepted during post optimization
          // processing.
          GV.setLinkage(GlobalValue::ExternalLinkage);

          // Add an alias metadata for later use.
          GV.addMetadata("spirv.Decorations.HostAccess", *Node);
          Changed = true;
          break;
        }
      }
    }
  }
  return Changed;
}

PreservedAnalyses
ExternalizeGlobalVariablesPass::run(Module &M, ModuleAnalysisManager &AM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool ExternalizeGlobalVariablesPass::runImpl(Module &M) {
  bool Changed = false;

  if (!CompilationUtils::isGeneratedFromOMP(M)) {
    return externalizeDeviceGlobal(&M);
  }

  // Externalize globals if IR is generated from OpenMP offloading. Now we
  // cannot get address of globals with internal/private linkage from LLJIT
  // (by design), but it's necessary by OpenMP to pass address of declare
  // target variables to the underlying OpenCL Runtime via
  // clSetKernelExecInfo. So we have to externalize globals for IR generated
  // from OpenMP.
  SmallSet<Value *, 8> TLSGlobals;
  SmallVector<unsigned, 8> ImplicitArgEnums;
  ImplicitArgsUtils::getImplicitArgEnums(ImplicitArgEnums, &M);

  for (unsigned I : ImplicitArgEnums) {
    GlobalVariable *GV = CompilationUtils::getTLSGlobal(&M, I);
    TLSGlobals.insert(GV);
  }

  for (auto &GVar : M.globals()) {
    bool SkipConvertLinkage =
        TLSGlobals.count(&GVar) || !GVar.hasName() ||
        (GVar.hasName() && GVar.getName().startswith("llvm.")) ||
        (GVar.getLinkage() != GlobalValue::InternalLinkage &&
         GVar.getLinkage() != GlobalValue::PrivateLinkage);
    if (SkipConvertLinkage)
      continue;

    LLVM_DEBUG(dbgs() << "Converting " << GVar.getName()
                      << " to external linkage.\n");
    GVar.setLinkage(GlobalValue::ExternalLinkage);
    Changed = true;
  }

  return Changed;
}
