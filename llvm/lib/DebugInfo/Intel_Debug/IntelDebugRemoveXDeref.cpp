//===- IntelDebugRemoveXDeref.cpp -----------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// During instruction selection, no-op address space casts are removed
// from the code but the DWARF location expressions are left unmodified.
// This code will recognize unnecessary address space casting in the debug
// location expressions and remove them.
//
// GDB has been modified to ignore the address space opcodes, but this only
// happens in GDB versions later than 12.1, and all prior versions fail to
// display variable variables and produce this error:
//   Unhandled dwarf expression opcode 0x18
//
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Intel_Debug/IntelDebugRemoveXDeref.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"

#include <iterator>

#define DEBUG_TYPE "intel-debug"

using namespace llvm;

static cl::opt<bool>
    EnableIntelDebugRemoveXDeref("enable-intel-debug-remove-xderef",
                                 cl::init(true), cl::ReallyHidden);

typedef SmallVector<uint64_t, 1> DebugExprOp;
typedef SmallVector<DebugExprOp, 1> DebugExpr;

static const unsigned DefaultAddressSpace = 0;
static const size_t MINIMUM_ELEMENTS_FOR_XDEREF = 4;

static void append(DebugExpr &DE, const DIExpression *E) {
  for (auto &EO : E->expr_ops()) {
    DebugExprOp DEO;
    DEO.push_back(EO.getOp());
    for (unsigned I = 0, E = EO.getNumArgs(); I < E; ++I)
      DEO.push_back(EO.getArg(I));
    DE.push_back(DEO);
  }
}

static DIExpression *create(LLVMContext &Context, DebugExpr &DE) {
  SmallVector<uint64_t> Elements;
  for (auto &Op : DE)
    for (auto &V : Op)
      Elements.push_back(V);
  return DIExpression::get(Context, Elements);
}

static bool maybeRemoveXDeref(DebugExpr &DE, unsigned SrcAS,
                              TargetMachine *TM) {
  bool Changed = false;
  for (size_t i = 0, e = DE.size(); i < e; ++i) {
    if (i + 2 < e && // Make sure we don't overrun the expression.
        DE[i + 0][0] == dwarf::DW_OP_constu &&
        DE[i + 1][0] == dwarf::DW_OP_swap &&
        DE[i + 2][0] == dwarf::DW_OP_xderef) {
      if (DE[i].size() != 2)
        break;
      unsigned DstAS = DE[i][1];
      if (SrcAS == DstAS || TM->isNoopAddrSpaceCast(SrcAS, DstAS)) {
        DebugExpr::iterator beg = std::next(DE.begin(), i);
        DebugExpr::iterator end = std::next(beg, 3);
        end = DE.erase(beg, end);
        // DE.insert(end, DebugExprOp({dwarf::DW_OP_deref}));
        e = DE.size();
        SrcAS = DstAS;
        Changed = true;
      }
    }
  }
  return Changed;
}

IntelDebugRemoveXDeref::IntelDebugRemoveXDeref(TargetMachine *TM) : TM(TM) {}

bool IntelDebugRemoveXDeref::run(Module &M) {
  if (!EnableIntelDebugRemoveXDeref)
    return false;

  // A target machine is required to determine where xderef should be removed.
  if (!TM)
    return false;

  // Currently, CLANG only emits XDeref opcodes for SPIRV.
  if (!M.getNamedMetadata("opencl.spir.version"))
    return false;

  bool Changed = false;
  for (GlobalVariable &GV : M.globals())
    Changed |= run(GV);
  for (Function &F : M)
    Changed |= run(F);
  return Changed;
}

bool IntelDebugRemoveXDeref::run(Function &F) {
  // Skip functions without debug information.
  if (!F.getSubprogram())
    return false;

  bool Changed = false;
  for (auto &I : instructions(F))
    if (auto *DVI = dyn_cast_or_null<DbgVariableIntrinsic>(&I))
      Changed |= run(*DVI);
  return Changed;
}

bool IntelDebugRemoveXDeref::run(DbgVariableIntrinsic &DVI) {
  DIExpression *E = DVI.getExpression();
  if (E->getNumElements() < MINIMUM_ELEMENTS_FOR_XDEREF)
    return false;

  LLVM_DEBUG(dbgs() << "DVI: " << DVI << "\n");

  unsigned SrcAS = DefaultAddressSpace;
  // Eventually we'll need to support variable location lists.
  if (!DVI.hasArgList()) {
    Value *Op = DVI.getVariableLocationOp(0);
    // Malformed IR can result in an invalid location operand. Check Op first.
    if (auto P = Op ? dyn_cast<PointerType>(Op->getType()) : nullptr)
      SrcAS = P->getPointerAddressSpace();
  }

  DebugExpr DE;
  append(DE, E);
  bool Changed = maybeRemoveXDeref(DE, SrcAS, TM);
  if (Changed) {
    DVI.setExpression(create(E->getContext(), DE));
    LLVM_DEBUG(dbgs() << " ==> " << DVI << "\n");
  }

  return Changed;
}

bool IntelDebugRemoveXDeref::run(GlobalVariable &GV) {
  bool Changed = false;
  SmallVector<DIGlobalVariableExpression *, 1> GVEs;
  GV.getDebugInfo(GVEs);

  for (auto *GVE : GVEs) {
    DIExpression *E = GVE->getExpression();
    if (E->getNumElements() < MINIMUM_ELEMENTS_FOR_XDEREF)
      continue;
    LLVM_DEBUG(dbgs() << "GVE: " << *GVE << "\n");
    DebugExpr DE;
    append(DE, E);
    unsigned SrcAS = DefaultAddressSpace;
    bool ChangedGVE = maybeRemoveXDeref(DE, SrcAS, TM);
    if (ChangedGVE) {
      DIExpression *NewE = create(E->getContext(), DE);
      GVE->replaceOperandWith(1, NewE);
      Changed = true;
      LLVM_DEBUG(dbgs() << " ==> " << *GVE << "\n");
    }
  }
  return Changed;
}
