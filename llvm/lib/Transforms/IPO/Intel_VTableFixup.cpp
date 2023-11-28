//===---- Intel_VTableFixup.cpp -------------------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass removes references to unresolved symbols from the initializers
// of typeinfo and vtable data structures.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_VTableFixup.h"

using namespace llvm;

#define DEBUG_TYPE "vtable-fixup"

// Recursively process initializers and replace references
// to undefined symbols with null values.
static Constant *processInitializer(Constant *Init) {
  if (auto *AggrInit = dyn_cast<ConstantAggregate>(Init)) {
    if (!isa<ConstantArray>(AggrInit) && !isa<ConstantStruct>(AggrInit))
      return nullptr;

    SmallVector<Constant *, 8> Data;
    for (auto *Op : AggrInit->operand_values()) {
      auto *NewOp = processInitializer(cast<Constant>(Op));
      if (!NewOp)
        return nullptr;
      Data.push_back(NewOp);
    }
    if (auto *ArrayInit = dyn_cast<ConstantArray>(AggrInit))
      return ConstantArray::get(ArrayInit->getType(), Data);
    else if (auto *StructInit = dyn_cast<ConstantStruct>(AggrInit))
      return ConstantStruct::get(StructInit->getType(), Data);

    return nullptr;
  }

  if (auto *CEInit = dyn_cast<ConstantExpr>(Init)) {
    unsigned Opcode = CEInit->getOpcode();
    if (Opcode == Instruction::AddrSpaceCast ||
        Opcode == Instruction::BitCast ||
        Opcode == Instruction::GetElementPtr) {
      auto *Op0 = CEInit->getOperand(0);
      if (auto *NewOp = processInitializer(Op0)) {
        if (NewOp == Op0)
          return CEInit;

        SmallVector<Constant *, 8> Ops;
        Ops.push_back(NewOp);
        for (unsigned i = 1, e = CEInit->getNumOperands(); i != e; i++)
          Ops.push_back(CEInit->getOperand(i));
        return CEInit->getWithOperands(Ops);
      }
    }

    return nullptr;
  }

  if (isa<ConstantData>(Init))
    return Init;

  auto *GValue = dyn_cast<GlobalValue>(Init);
  if (!GValue)
    return nullptr;

  if (!GValue->isDeclaration())
    return GValue;

  return ConstantPointerNull::get(GValue->getType());
}

static bool runImpl(Module &M) {
  bool Changed = false;
  LLVM_DEBUG(dbgs() << "Running IntelVTableFixupPass on Module " <<
             M.getName() << "\n");
  for (auto &GV : M.globals()) {
    StringRef GVName = GV.getName();
    // "_ZTV" prefix is used for vtables.
    // Typeinfo descriptors ("_ZTI") may reference CXX ABI typeinfo
    // descriptors, which are not defined in device environments
    // that we currently support with offload, so we also fix them up
    // the same way as vtables.
    if (!GVName.startswith("_ZTV") && !GVName.startswith("_ZTI"))
      continue;

    if (!GV.hasInitializer()) {
      LLVM_DEBUG(dbgs() <<
                 "uninitialized typeinfo or vtable " << GVName << "\n");
      continue;
    }

    Constant *Init = GV.getInitializer();
    LLVM_DEBUG(dbgs() << "processing initializer of " << GVName << "\n");
    Constant *NewInit = processInitializer(Init);
    if (!NewInit || NewInit == Init)
      continue;
    LLVM_DEBUG(dbgs() << "replacing initializer:\n" << *Init <<
               "\nwith initializer:\n" << *NewInit << "\n");
    GV.setInitializer(NewInit);
    Changed = true;
  }
  return Changed;
}

PreservedAnalyses IntelVTableFixupPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  runImpl(M);
  return PreservedAnalyses::all();
}
