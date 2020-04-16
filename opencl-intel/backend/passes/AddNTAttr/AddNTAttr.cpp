// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#include "AddNTAttr.h"

#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "cl-add-nt-attr"

namespace intel {

OCL_INITIALIZE_PASS_BEGIN(AddNTAttr, "cl-add-nt-attr", "add non-temporal attribute", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BasicAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
OCL_INITIALIZE_PASS_END(AddNTAttr, "cl-add-nt-attr", "add non-temporal attribute", false, false)

char AddNTAttr::ID = 0;

AddNTAttr::AddNTAttr() : FunctionPass(ID), m_F(nullptr) {
  initializeAddNTAttrPass(*PassRegistry::getPassRegistry());
}

void AddNTAttr::getAnalysisUsage(AnalysisUsage &aU) const {
  aU.addRequired<BasicAAWrapperPass>();
  aU.addRequired<AAResultsWrapperPass>();
}

bool AddNTAttr::runOnFunction(Function &func) {
  auto &aaRet = getAnalysis<AAResultsWrapperPass>().getAAResults();
  bool changed = false;
  m_F = &func;
  SmallVector<StoreInst *, 4> siVec;
  SmallVector<Instruction *, 4> riVec;
  for (auto &iI : instructions(func)) {
    StoreInst *sI = dyn_cast<StoreInst>(&iI);
    if (sI)
      siVec.push_back(sI);
    else if (iI.mayReadFromMemory())
      riVec.push_back(&iI);
  }
  // A simple and strict pattern recognition for enabling NT safely,
  // If "Store" and "Read" are "mayAlias", it is not safe to enable NT
  // Traverse the function to make sure there isn't below pattern,
  //
  // No read operation on the %addr
  // %1 = store i32 val, %addr
  // No read operation on the %addr
  //
  // TODO: this recognition is strict and maybe lost the opportunity
  // so it is probably to use "Dominance" to analyze "RAW" mode, e.g.
  //
  // %0 = store i32 val, %addr
  // %1 = load %addr
  //
  // seems NT enabling is reasonable for above case, but there isn't this case now.
  //
  for (StoreInst *sI : siVec) {
    // Enable NT if no Read/Call exist
    bool mayAlias = false;
    for (Instruction *rI : riVec) {
      // Disable NT by default if Read or Call exist
      mayAlias = true;
      // if the Call and the Store are "mayAlias"
      if (auto *call = dyn_cast<CallBase>(rI))
        mayAlias = isModOrRefSet(aaRet.getModRefInfo(sI, call));
      // if the Load and the Store are "mayAlias"
      else if (LoadInst *lI = dyn_cast<LoadInst>(rI)) {
        MemoryLocation loc = MemoryLocation::get(lI);
        mayAlias = isModOrRefSet(aaRet.getModRefInfo(sI, loc));
      } else {
        // For Read instruction which are not Load and Call,
        // Check Alias info between its operands and the Store's pointer.
        // Disable NT If operands are empty, e.g. Fence.
        for (Value *op : rI->operands()) {
          if (!op->getType()->isPointerTy())
            continue;
          mayAlias = !aaRet.isNoAlias(sI->getPointerOperand(), op);
          if (mayAlias) break;
        }
      }
      // If Read(Call) and Store are "mayAlias", early bail out
      if (mayAlias) break;
    }
    if (!mayAlias) changed |= setNTAttr(sI);
  }
  if (changed)
    LLVM_DEBUG( dbgs() << "Add NT attr for " << func.getName() << "\n");
  return changed;
}

bool AddNTAttr::setNTAttr(StoreInst *sI) {
  if (sI->getMetadata("nontemporal")) {
    LLVM_DEBUG( dbgs() << "Already have NT attr in " << sI->getName() << "\n");
    return false;
  }
  SmallVector<Metadata *, 1> metas;
  LLVMContext &context = m_F->getContext();
  metas.push_back(ConstantAsMetadata::get(
        ConstantInt::get(Type::getInt32Ty(context), 1)));
  MDNode *node = MDNode::get(context, metas);
  sI->setMetadata(m_F->getParent()->getMDKindID("nontemporal"), node);
  return true;
}

}// namespace intel

extern "C" {
  FunctionPass* createAddNTAttrPass() {
    return new intel::AddNTAttr();
  }
}
