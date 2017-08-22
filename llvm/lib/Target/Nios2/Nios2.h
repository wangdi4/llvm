//===-- Nios2.h - Top-level interface for Nios2 representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Nios2 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2_H

#include "llvm/IR/Function.h"
#include "MCTargetDesc/Nios2MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/PassRegistry.h"

namespace llvm {
  // Enumerates memory levels in a Titan Trail device with Nios2 cores
  enum Nios2MemLevel {
    Nios2_No_Mem, // unknown memory level
    Nios2_L1_Mem, // tightly-coupled memory
    Nios2_L2_Mem, // memory shared across cores within a column (~256K)
    Nios2_L3_Mem, // memory shared across columns (~512K)
    Nios2_L4_Mem  // FPGA DDR (~4G) or System memory
  };

  class Nios2TargetMachine;
  class FunctionPass;

#ifdef ENABLE_GPRESTORE
  FunctionPass *createNios2EmitGPRestorePass(Nios2TargetMachine &TM);
#endif
  FunctionPass *createNios2DelaySlotFillerPass(Nios2TargetMachine &TM);
  FunctionPass *createNios2DelJmpPass(Nios2TargetMachine &TM);

  FunctionPass* createNios2LowerAllocaPass(const TargetMachine *TM);

  void initializeNios2LowerAllocaPass(PassRegistry &);

namespace nios2 {
  class Util {
  public:
    // returns a Type instance for the size_t target-dependent type
    static Type* getSizetTy(Module& m);
    static Type* getSizetTy(LLVMContext& ctx, const DataLayout& dl);

    // returns a Function instance for the
    // void* nios2_unsafe_malloc_l2 (size_t size, int align)
    static Function* getUnsafeMallocBuiltin(Module& m, Nios2MemLevel mem);

    // returns a Function instance for the
    // void nios2_unsafe_free_l2 (void* ptr);
    static Function* getGetFreeMemBuiltin(Module& m, Nios2MemLevel mem);
  };
} // end namespace nios2;

} // end namespace llvm;

#endif
