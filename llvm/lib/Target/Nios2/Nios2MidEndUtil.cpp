//===- Nios2MidEndUtil.cpp - misc nios2-specific middle-end utilities -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This implements miscellaneous utility functions used in nios2-specific
// middle-end code which operates on high-level LLVM IR rather than CodeGen IR
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

#include "Nios2.h"

using namespace llvm;

Type* nios2::Util::getSizetTy(Module& m) {
  return getSizetTy(m.getContext(), m.getDataLayout());
}

Type* nios2::Util::getSizetTy(LLVMContext& ctx, const DataLayout& dl) {
  const int intptr_t_size = dl.getTypeAllocSize(
    Type::getInt32PtrTy(ctx)->getPointerTo());
  return Type::getIntNTy(ctx, intptr_t_size*8);
}

Function* llvm::nios2::Util::getUnsafeMallocBuiltin(Module& m, Nios2MemLevel mem) {
  Function* builtin = nullptr;

  switch (mem) {
  case Nios2_L2_Mem:
    {
      FunctionType* f_ty = FunctionType::get(
        Type::getInt8Ty(m.getContext())->getPointerTo(),
        { getSizetTy(m), Type::getInt32Ty(m.getContext()) },
        false);
      Value* f_val = m.getOrInsertFunction("nios2_unsafe_malloc_l2", f_ty);
      builtin = cast<Function>(f_val);
    }
    break;
  case Nios2_L3_Mem:
    llvm_unreachable("L3 allocation NYI");
    break;
  default:
    llvm_unreachable("must be either L2 or L3");
  }
  return builtin;
}

Function* llvm::nios2::Util::getGetFreeMemBuiltin(Module& m, Nios2MemLevel mem) {
  Function* builtin = nullptr;

  switch (mem) {
  case Nios2_L2_Mem:
    {
      FunctionType* f_ty = FunctionType::get(getSizetTy(m), false);
      Value* f_val = m.getOrInsertFunction("nios_get_free_l2_mem_size", f_ty);
      builtin = cast<Function>(f_val);
    }
    break;
  case Nios2_L3_Mem:
    llvm_unreachable("L3 free NYI");
    break;
  default:
    llvm_unreachable("must be L2 or L3");
  }
  return builtin;
}
