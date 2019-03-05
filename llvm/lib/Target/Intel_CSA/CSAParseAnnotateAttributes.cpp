//===- CSAParseAnnotateAttributes.cpp - CSA Parser for annotate attributes===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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
// The special string will start with __csa_attr
// A new attribute with the same name as the special string will be added to
// list of function attributes to 'foo'
//
//===----------------------------------------------------------------------===//

#include "CSASubtarget.h"
#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/CallSite.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <set>

#include "CSAUtils.h"
using namespace llvm;

#define DEBUG_TYPE "csa-parse-annotate"
#define REMARK_NAME "csa-parse-annotate-remark"
#define PASS_DESC                                                              \
  "CSA: Parse annotations and add a corresponding function attribute"

namespace llvm {
Pass *createCSAParseAnnotateAttributesPass();
} // namespace llvm

namespace {
struct CSAParseAnnotateAttributes : public ModulePass {
  static char ID;

  explicit CSAParseAnnotateAttributes() : ModulePass(ID) {}
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
  bool runOnModule(Module &M) override;
};
} // namespace

char CSAParseAnnotateAttributes::ID = 0;

Pass *llvm::createCSAParseAnnotateAttributesPass() {
  return new CSAParseAnnotateAttributes();
}

// This code is based on a blog post by Brandon Holt
// http://bholt.org/posts/llvm-quick-tricks.html
bool CSAParseAnnotateAttributes::runOnModule(Module &M) {
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
           ->getOperand(0))->getAsCString();
      F->addFnAttr(Annotation);
      LLVM_DEBUG(errs() << "adding annotation " << Annotation << " to "
                        << F->getName() << "\n");
    }
  }
  Annotations->eraseFromParent();
  return true;
}

