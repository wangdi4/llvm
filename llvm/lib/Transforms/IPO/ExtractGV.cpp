//===-- ExtractGV.cpp - Global Value extraction pass ----------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass extracts global values
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/ExtractGV.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include <algorithm>

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
#define EXTRACT_DTRANS_MD "extract-dtrans-md"
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

/// Make sure GV is visible from both modules. Delete is true if it is
/// being deleted from this module.
/// This also makes sure GV cannot be dropped so that references from
/// the split module remain valid.
static void makeVisible(GlobalValue &GV, bool Delete) {
  bool Local = GV.hasLocalLinkage();
  if (Local || Delete) {
    GV.setLinkage(GlobalValue::ExternalLinkage);
    if (Local)
      GV.setVisibility(GlobalValue::HiddenVisibility);
    return;
  }

  if (!GV.hasLinkOnceLinkage()) {
    assert(!GV.isDiscardableIfUnused());
    return;
  }

  // Map linkonce* to weak* so that llvm doesn't drop this GV.
  switch (GV.getLinkage()) {
  default:
    llvm_unreachable("Unexpected linkage");
  case GlobalValue::LinkOnceAnyLinkage:
    GV.setLinkage(GlobalValue::WeakAnyLinkage);
    return;
  case GlobalValue::LinkOnceODRLinkage:
    GV.setLinkage(GlobalValue::WeakODRLinkage);
    return;
  }
}

/// If deleteS is true, this pass deletes the specified global values.
/// Otherwise, it deletes as much of the module as possible, except for the
/// global values specified.
ExtractGVPass::ExtractGVPass(std::vector<GlobalValue *> &GVs, bool deleteS,
                             bool keepConstInit)
    : Named(GVs.begin(), GVs.end()), deleteStuff(deleteS),
      keepConstInit(keepConstInit) {}

PreservedAnalyses ExtractGVPass::run(Module &M, ModuleAnalysisManager &) {
  // Visit the global inline asm.
  if (!deleteStuff)
    M.setModuleInlineAsm("");

  // For simplicity, just give all GlobalValues ExternalLinkage. A trickier
  // implementation could figure out which GlobalValues are actually
  // referenced by the Named set, and which GlobalValues in the rest of
  // the module are referenced by the NamedSet, and get away with leaving
  // more internal and private things internal and private. But for now,
  // be conservative and simple.

  // Visit the GlobalVariables.
  for (GlobalVariable &GV : M.globals()) {
    bool Delete = deleteStuff == (bool)Named.count(&GV) &&
                  !GV.isDeclaration() && (!GV.isConstant() || !keepConstInit);
    if (!Delete) {
      if (GV.hasAvailableExternallyLinkage())
        continue;
      if (GV.getName() == "llvm.global_ctors")
        continue;
    }

    makeVisible(GV, Delete);

    if (Delete) {
      // Make this a declaration and drop it's comdat.
      GV.setInitializer(nullptr);
      GV.setComdat(nullptr);
    }
  }

  // Visit the Functions.
  for (Function &F : M) {
    bool Delete = deleteStuff == (bool)Named.count(&F) && !F.isDeclaration();
    if (!Delete) {
      if (F.hasAvailableExternallyLinkage())
        continue;
    }

    makeVisible(F, Delete);

    if (Delete) {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
      // Get any existing DTrans type metadata before the body is deleted
      // because deleting the body will keep the DTrans type attributes on
      // the function, but remove the DTrans type metadata attachment.
      MDNode *MD = dtransOP::TypeMetadataReader::getDTransMDNode(F);
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
      // Make this a declaration and drop it's comdat.
      F.deleteBody();
      F.setComdat(nullptr);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
      // Restore the DTrans type metadata onto the function declaration.
      if (MD)
        dtransOP::DTransTypeMetadataBuilder::addDTransMDNode(F, MD);
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
    }
  }

  // Visit the Aliases.
  for (GlobalAlias &GA : llvm::make_early_inc_range(M.aliases())) {
    bool Delete = deleteStuff == (bool)Named.count(&GA);
    makeVisible(GA, Delete);

    if (Delete) {
      Type *Ty = GA.getValueType();
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
      // If there is DTrans type metadata attached to the target alias, then
      // we will need the target to set up DTrans type metadata after creating
      // a new declaration.
      auto *AliasTarget = GA.getAliasee();
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION

      GA.removeFromParent();
      llvm::Value *Declaration;
      if (FunctionType *FTy = dyn_cast<FunctionType>(Ty)) {
        Declaration = Function::Create(FTy, GlobalValue::ExternalLinkage,
                                       GA.getAddressSpace(), GA.getName(), &M);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
        // If there was DTrans type metadata, set it on the new function
        // declaration that is being created to replace the alias of the
        // function.
        if (auto *TargFunc = dyn_cast<Function>(AliasTarget))
          if (TargFunc->getValueType() == Ty) {
            dtransOP::DTransTypeMetadataBuilder::copyDTransFuncMetadata(
                TargFunc, cast<Function>(Declaration));
          } else {
            DEBUG_WITH_TYPE(EXTRACT_DTRANS_MD, {
              if (dtransOP::TypeMetadataReader::getDTransMDNode(*TargFunc))
                dbgs() << "\nDTrans type metadata dropped during "
                          "extraction: "
                       << *Declaration
                       << " - Source was: " << TargFunc->getName() << "\n";
            });
          }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
      } else {
        Declaration = new GlobalVariable(
            M, Ty, false, GlobalValue::ExternalLinkage, nullptr, GA.getName());
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
        // Propagate DTrans type metadata to the replacement declaration, if
        // we can. We avoid doing this for constant expressions for now
        // because it may require more than just copying metadata
        // information. For example, if the alias is a GEPOperator, then the
        // type for the replacement declaration would need to determine the
        // type of the element indexed. Because this functionality is only
        // being used when running llvm-extract to trim down test cases, we
        // can probably live with losing the metadata in some cases.
        if (isa<GlobalVariable>(AliasTarget)) {
          MDNode *MD =
              dtransOP::TypeMetadataReader::getDTransMDNode(*AliasTarget);
          if (MD)
            dtransOP::DTransTypeMetadataBuilder::addDTransMDNode(*Declaration,
                                                                 MD);
        } else {
          DEBUG_WITH_TYPE(EXTRACT_DTRANS_MD, {
            Value *SrcVal = AliasTarget->stripInBoundsConstantOffsets();
            if (dtransOP::TypeMetadataReader::getDTransMDNode(*SrcVal))
              dbgs() << "\nDTrans type metadata dropped during "
                        "extraction: "
                     << *Declaration << " - Source was: " << *SrcVal << "\n";
          });
        }
#endif // INTEL_FEATURE_SW_DTRANS
#endif // INTEL_CUSTOMIZATION
      }
      GA.replaceAllUsesWith(Declaration);
      delete &GA;
    }
  }

  // Visit the IFuncs.
  for (GlobalIFunc &IF : llvm::make_early_inc_range(M.ifuncs())) {
    bool Delete = deleteStuff == (bool)Named.count(&IF);
    makeVisible(IF, Delete);

    if (!Delete)
      continue;

    auto *FuncType = dyn_cast<FunctionType>(IF.getValueType());
    IF.removeFromParent();
    llvm::Value *Declaration =
        Function::Create(FuncType, GlobalValue::ExternalLinkage,
                         IF.getAddressSpace(), IF.getName(), &M);
    IF.replaceAllUsesWith(Declaration);
    delete &IF;
  }

  return PreservedAnalyses::none();
}
