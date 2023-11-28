//===- ParseAnnotateAttributes.cpp - Parser for annotate attributes===========//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass parses the list of global annotations added by the user as follows
// __attribute__((annotate(<Spl String added to LLVM global annotations list>))
// void foo() { ... }
// A new attribute with the same name as the annotation string will be added to
// list of function attributes to 'foo'.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "parse-annotate"
#define REMARK_NAME "parse-annotate-remark"
#define PASS_DESC "Parse annotations and add a corresponding function attribute"

namespace llvm {
Pass *createParseAnnotateAttributesPass();

class ParseAnnotateAttributes : public PassInfoMixin<ParseAnnotateAttributes> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

// This code is based on a blog post by Brandon Holt
// http://bholt.org/posts/llvm-quick-tricks.html
static bool runParseAnnotateAttributes(Module &M) {
  auto Annotations = M.getNamedGlobal("llvm.global.annotations");
  if (!Annotations) {
    LLVM_DEBUG(errs() << "No annotations found\n");
    return false;
  }
  auto t1 = cast<ConstantArray>(Annotations->getOperand(0));
  for (unsigned i = 0; i < t1->getNumOperands(); ++i) {
    auto t2 = cast<ConstantStruct>(t1->getOperand(i));
    if (auto F = dyn_cast<Function>(t2->getOperand(0)->getOperand(0))) {
      auto Annotation =
          cast<ConstantDataArray>(
              cast<GlobalVariable>(t2->getOperand(1)->getOperand(0))
                  ->getOperand(0))
              ->getAsCString();
      F->addFnAttr(Annotation);
      LLVM_DEBUG(errs() << "adding annotation " << Annotation << " to "
                        << F->getName() << "\n");
    }
  }
  Annotations->eraseFromParent();
  return true;
}

PreservedAnalyses ParseAnnotateAttributes::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  if (!runParseAnnotateAttributes(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

struct ParseAnnotateAttributesLegacy : public ModulePass {
  static char ID;

  explicit ParseAnnotateAttributesLegacy();
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
  bool runOnModule(Module &M) override;
};

ParseAnnotateAttributesLegacy::ParseAnnotateAttributesLegacy()
    : ModulePass(ID) {
  initializeParseAnnotateAttributesLegacyPass(*PassRegistry::getPassRegistry());
}

bool ParseAnnotateAttributesLegacy::runOnModule(Module &M) {
  return runParseAnnotateAttributes(M);
}

} // namespace llvm

Pass *llvm::createParseAnnotateAttributesPass() {
  return new ParseAnnotateAttributesLegacy();
}

char ParseAnnotateAttributesLegacy::ID = 0;

INITIALIZE_PASS(ParseAnnotateAttributesLegacy, DEBUG_TYPE, PASS_DESC,
                false /* modifies CFG */, false /* transform pass */)
