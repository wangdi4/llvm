//==--- Intel_TransformFPGAReg.cpp -*- C++ -*---==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

// TransformFPGAReg pass transforms llvm.(ptr.)annotation.* representation
// of __fpga_reg builtin to the proprietary builtin representation, namely the
// llvm.fpga.reg.* intrinsic calls
//
// Since llvm.annotation and llvm.ptr.annotation only return Int and PtrToInt
// respectively, values of other types are bitcasted to the needed type before
// the intrinsic call and bitcasted back after that. E.g. a struct/union in
// source code results in: 1) bitcast to i8*, 2) llvm.ptr.annotation.p0i8 call,
// 3) bitcast to original POD type
//
// In turn, the proprietary builtin has additional types to represent POD,
// e.g. llvm.fpga.reg.p0s_struct.*. However, this does not force us into
// stripping the surrounding bitcasts and generating the POD-typed intrinsic.
// At the time of writing, this is considered unnecessary for the consumer.
// If needed at some point later, this may be performed in a separate pass

#include "llvm/Transforms/Scalar/Intel_TransformFPGAReg.h"

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace {

class TransformFPGAReg : public ModulePass {
public:
  static char ID;
  TransformFPGAReg() : ModulePass(ID) {
    initializeTransformFPGARegPass(
        *PassRegistry::getPassRegistry());
  }
  bool runOnModule(Module &M) override;
};
} // end anonymous namespace

char TransformFPGAReg::ID = 0;
INITIALIZE_PASS(TransformFPGAReg, "transform-fpga-reg",
             "Transform __fpga_reg builtin representation", false, false)

ModulePass *llvm::createTransformFPGARegPass() {
  return new TransformFPGAReg();
}

bool TransformFPGAReg::runOnModule(Module &M) {
  bool HasChanged = false;
  for (auto &GV : M.getGlobalList()) {
    if (!GV.isConstant())
      continue;

    auto *CDA = dyn_cast<ConstantDataArray>(GV.getInitializer());
    if (!(CDA && CDA->isCString() &&
            CDA->getAsCString() == "__builtin_intel_fpga_reg"))
      continue;

    // This global constant is usually accessed via a
    // GetElementPtrConstantExpr. This "latches" the
    // constant to its 'actual' users in functions
    for (auto *GVAccessor : GV.users()) {
      // Now come the 'actual' users
      for (Value::user_iterator UI = GVAccessor->user_begin();
               UI != GVAccessor->user_end(); ) {
        // The fpga_reg string literal can be accessed by
        // llvm.annotation.* and llvm.ptr.annotation.*
        // intrinsic calls, thus representing a builtin
        auto *II = dyn_cast<IntrinsicInst>(*UI++);
        if (!II)
          continue;
        if (II->getIntrinsicID() == Intrinsic::annotation ||
                II->getIntrinsicID() == Intrinsic::ptr_annotation) {
          // We will be creating an fpga.reg intrinsic of the
          // same return type, with the same variable argument
          Type *TysForDecl[] = { II->getFunctionType()->getReturnType() };
          Value *Args[] = { II->getArgOperand(0) };

          Function *FPGARegBuiltinFn = Intrinsic::getDeclaration(
              &M, Intrinsic::fpga_reg, TysForDecl);
          assert(FPGARegBuiltinFn &&
                     "Failure to generate llvm.fpga.reg declaration");
          CallInst *FPGARegBuiltinCall = CallInst::Create(
              FPGARegBuiltinFn, Args, "", II);

          // The iterator that pointed to the instruction has already
          // been incremented. We can safely replace & erase it
          II->replaceAllUsesWith(FPGARegBuiltinCall);
          II->eraseFromParent();
          HasChanged = true;
        }
      }
    }
  }
  return HasChanged;
}

PreservedAnalyses TransformFPGARegPass::run(Module &M,
                                            ModuleAnalysisManager &) {
  TransformFPGAReg TFPGAReg;
  TFPGAReg.runOnModule(M);
  return PreservedAnalyses::all();
}
