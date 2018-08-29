// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "StripIntelIP.h"

#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"

using namespace llvm;
using namespace Intel::MetadataAPI;

namespace intel {

char StripIntelIP::ID = 0;

OCL_INITIALIZE_PASS(
    StripIntelIP, "strip-intel-ip",
    "strips the Module of all data not essential to"
    "functioning of OpenCL CPU RT",
    /* Only looks at CFG */false, /* Analysis Pass */false)

StripIntelIP::StripIntelIP() : ModulePass(ID) {
  initializeStripIntelIPPass(*PassRegistry::getPassRegistry());
}

// deleteBody and filter Metadata.
static void stripFunction(Function *Func) {
  SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
  Func->getAllMetadata(MDs);

  SmallSet<unsigned, 16> MDKindIDsToSpare;

  KernelMetadataAPI KernelMetadata(Func);
  KernelInternalMetadataAPI kernelInternalMetadata(Func);
  const auto &KMDNames = KernelMetadata.getMDNames();
  const auto &KIMDNames = kernelInternalMetadata.getMDNames();

  for (auto MDName : KMDNames)
    MDKindIDsToSpare.insert(Func->getContext().getMDKindID(MDName));
  for (auto MDName : KIMDNames)
    MDKindIDsToSpare.insert(Func->getContext().getMDKindID(MDName));

  // deleteBody will strip all of the Metadata,
  // so we go through the hassle of setting it back.
  Func->deleteBody();

  for (auto &MD : MDs) {
    if (MDKindIDsToSpare.count(MD.first) > 0)
      Func->setMetadata(MD.first, MD.second);
  }

  // No need to spare attributes.
  AttributeList EmptyAttrs;
  Func->setAttributes(EmptyAttrs);
}

static void emptyFunctionBodies(
    const SmallPtrSetImpl<Function*> &FuncsToEmpty) {
  for (auto *F : FuncsToEmpty) {
    stripFunction(F);
    auto *ExitBB = BasicBlock::Create(F->getContext(), "exit", F);
    IRBuilder<> builder(F->getContext());
    builder.SetInsertPoint(ExitBB);
    // kernels are void by spec.
    builder.CreateRetVoid();
  }
}

bool StripIntelIP::runOnModule(Module &M) {
  // Split all Functions into 3 categories:
  // Those that should be emptied, but remain defined.
  // These are external functions exported from Module.
  SmallPtrSet<Function*, 8> KernelDefs;
  // Kernel wrappers, which is a decl by design,
  // but require Metadata and Attributes filtering.
  SmallPtrSet<Function*, 8> KernelDecls;
  // Those that should be deleted entirely.
  SmallVector<Function*, 8> FuncsToRemove;

  SmallVector<GlobalVariable*, 16> GlobalsToRemove;
  SmallVector<NamedMDNode*, 8> NamedMDNodesToRemove;

  // Collection Stage 1. Collect functions whose bodies we need
  // to empty and those we need to save attached Metadata for.
  for (auto *F : KernelList(&M)) {
    assert(F->isDeclaration() && "Decl is expected!");
    KernelDecls.insert(F);
    auto KIMD = KernelInternalMetadataAPI(F);
    assert(KIMD.KernelWrapper.hasValue() && "always expect wrapper");
    auto *FWrapper = KIMD.KernelWrapper.get();
    KernelDefs.insert(FWrapper);

    if (!KIMD.VectorizedKernel.hasValue())
      continue;
    auto *FVec = KIMD.VectorizedKernel.get();
    if (FVec) {
      KernelDecls.insert(FVec);
      auto VKIMD = KernelInternalMetadataAPI(FVec);
      auto *FVecWrapper = VKIMD.KernelWrapper.get();
      KernelDefs.insert(FVecWrapper);
    }
  }

  // Collection stage 2. Schedule other functions for removal.
  for (auto &F : M) {
    if (KernelDefs.count(&F) > 0)
      continue;
    if (KernelDecls.count(&F) > 0)
      continue;
    FuncsToRemove.push_back(&F);
  }

  // Collection stage 3. Spare only LLVM intrinsic GVs.
  // Global ctors/dtors is what we care most deeply about
  // as it is used by MCJIT.
  for (auto &GV : M.globals()) {
    if (GV.hasName() && GV.getName().startswith("llvm."))
      continue;
    GlobalsToRemove.push_back(&GV);
  }

  // Collection stage 4. Spare only OpenCL-specific Metadata.
  // Note, that IntelProprietaryFlag will be scheduled for deletion as well.
  SmallSet<StringRef, 16> NamedMDNodesToSpare;
  ModuleInternalMetadataAPI ModuleInternalMetadata(&M);
  for (auto MDNameToSpare : ModuleInternalMetadata.getMDNames()) {
    NamedMDNodesToSpare.insert(MDNameToSpare);
  }

  ModuleMetadataAPI ModuleMetadata(&M);
  for (auto MDNameToSpare : ModuleMetadata.getMDNames()) {
    NamedMDNodesToSpare.insert(MDNameToSpare);
  }

  NamedMDNodesToSpare.insert("opencl.kernels");
  for (auto &NamedMD : M.named_metadata()) {
    if (!(NamedMDNodesToSpare.count(NamedMD.getName()) > 0))
      NamedMDNodesToRemove.push_back(&NamedMD);
  }

  // Strip stage 1. Empty bodies of the functions we need to be exported.
  emptyFunctionBodies(KernelDefs);
  for (auto *F : KernelDecls)
    stripFunction(F);

  // Strip stage 2. Strip GV Initializers.
  // Initializers must be properly destroyed as they can
  // reference functions.
  for (auto *GV : GlobalsToRemove) {
    if (GV->hasInitializer()) {
      Constant *Init = GV->getInitializer();
      GV->setInitializer(nullptr);
      if (isSafeToDestroyConstant(Init))
        Init->destroyConstant();
    }
  }

  // Strip stage 3. Nuke remaining functions.
  for (auto *F : FuncsToRemove) {
    stripFunction(F);
  }
  // Now the only function uses are @llvm.* GVs.
  for (auto *F : FuncsToRemove) {
    if (F->use_empty())
      F->eraseFromParent();
  }

  // Strip stage 4. Nuke GVs.
  for (auto *GV : GlobalsToRemove) {
    GV->eraseFromParent();
  }

  // Strip stage 5. Nuke Named Metadata.
  for (auto *NamedMDNode : NamedMDNodesToRemove)
    M.eraseNamedMetadata(NamedMDNode);

  return true; // We always make changes.
}

} // namespace intel

extern "C" {
    llvm::ModulePass *createStripIntelIPPass() {
    return new intel::StripIntelIP();
  }
}
