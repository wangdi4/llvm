//=---------------------- VectorVariantFillIn.cpp -*- C++ -*-----------------=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/VectorVariantFillIn.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

#define DEBUG_TYPE "VectorVariantFillIn"

using namespace llvm;

bool DPCPPEnableVectorVariantPasses = true;
static cl::opt<bool, true> EnableVectorVariantPassesOpt(
    "dpcpp-enable-vector-variant-passes",
    cl::location(DPCPPEnableVectorVariantPasses), cl::Hidden,
    cl::desc(
        "Enable vector-variant/vector_function_ptrs attributes processing."));

namespace llvm {

PreservedAnalyses VectorVariantFillIn::run(Module &M,
                                           ModuleAnalysisManager &MAM) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

// This pass replaces __intel_create_simd_variant instructions with bitcasted
// actual function pointers and updates vector_function_ptrs global variables
// with actual function pointers.
//
// clang-format off
//
// Example:
//
//   define dso_local i32 @foo() {
//   entry:
//     %0 = tail call i32 (i32, float)* @__intel_create_simd_variant_p0f_i32i32f32f_p0f_i32i32f32f(i32 (i32, float)* nonnull @_Z3fooif) #0
//   }
//   attributes #0 = { "vector-variants"="_ZGVbN8lu__Z3fooif" }
//
// After the pass:
//
//   %0 = bitcast <2 x i32> (<2 x i32>, <2 x float>)* @_ZGVbN8lu__Z3fooif to i32 (i32, float)*
//
// clang-format on
//
// Also this pass updates function pointers global variables referenced in
// vector_function_ptrs attributes witch actual pointers tables.
//
bool VectorVariantFillIn::runImpl(Module &M) {
  if (!DPCPPEnableVectorVariantPasses)
    return false;

  bool Modified = false;
  DenseSet<CallInst *> InstToRemove;
  DenseSet<GlobalVariable *> UpdatedGV;

  for (auto &Fn : M) {
    Attribute Attr = Fn.getFnAttribute(Attribute::VectorFunctionPtrsStrAttr);
    if (Attr.isValid()) {
      LLVM_DEBUG(dbgs() << "Process function: " << Fn.getName() << "\n");
      StringRef Var = Attr.getValueAsString();

      while (!Var.empty()) {
        if (Var[0] == ',')
          Var = Var.slice(1, Var.size());

        size_t PosBegin = Var.find('(');
        size_t PosEnd = Var.find(')');
        assert(PosBegin != StringRef::npos && PosEnd != StringRef::npos &&
               "Expecting brackets in vector_function_ptrs attribute");

        StringRef VarName = Var.slice(0, PosBegin);
        StringRef VariantsStr = Var.slice(PosBegin + 1, PosEnd);

        SmallVector<StringRef, 4> Variants;
        VariantsStr.split(Variants, ",");

        // Update global variable initializer.
        GlobalVariable *GV = M.getGlobalVariable(VarName);
        if (GV && GV->hasInitializer() && !UpdatedGV.count(GV)) {
          SmallVector<Constant *, 2> Init;
          for (auto Variant : Variants) {
            Function *F = M.getFunction(Variant);
            assert(F && "Function expected to be exist");

            Constant *BitCast = ConstantExpr::getBitCast(F, Fn.getType());
            Init.push_back(BitCast);
          }

          ArrayType *ArrTy = ArrayType::get(Fn.getType(), Init.size());
          Constant *FPtrArray = ConstantArray::get(ArrTy, Init);

          // This variable is a table of pointers and it's address will be used
          // to re-initialize the global variable that corresponds to the
          // pointer.
          GlobalVariable *GVNewInit = new GlobalVariable(
              M, ArrTy, GV->isConstant(), GV->getLinkage(), FPtrArray, "", GV,
              GV->getThreadLocalMode(), GV->getAddressSpace());

          Value *NewVal = GVNewInit;
          // If the type between GV and GVNewInit is different, add bitcast.
          if (GV->getType() != GVNewInit->getType())
            NewVal = ConstantExpr::getBitCast(GVNewInit, GV->getType());

          SmallVector<User *, 16> UsersToUpdate(GV->users());
          for (User *U : UsersToUpdate) {
            if (Constant *C = dyn_cast<Constant>(U)) {
              if (!isa<GlobalValue>(C)) {
                LLVM_DEBUG(dbgs() << "Replace operand in Constant: " << *C
                                  << "\n (" << *GV << "\n  ->\n"
                                  << *NewVal << "\n");
                C->handleOperandChange(GV, NewVal);
                continue;
              }
            }
            U->replaceUsesOfWith(GV, NewVal);
          }

          Constant *Initializer = GV->getInitializer();
          GV->setInitializer(nullptr);
          Initializer->destroyConstant();

          std::string Name = GV->getName().str();
          GV->eraseFromParent();
          GVNewInit->setName(Name);

          UpdatedGV.insert(GVNewInit);
          Modified = true;
        }

        Var = Var.slice(PosEnd + 1, Var.size());
      }
    }

    // Process all __intel_create_simd_variant instructions.
    for (auto &Inst : instructions(Fn)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      Function *Callee = Call.getCalledFunction();

      if (!Callee ||
          !Callee->getName().startswith("__intel_create_simd_variant") ||
          !Call.hasFnAttr("vector-variants"))
        continue;

      // Replace the simd variant creation with an explicit function pointer.
      Attribute Attr = Call.getCallSiteOrFuncAttr("vector-variants");
      Function *Fn = M.getFunction(Attr.getValueAsString());
      assert(Fn && "Function expected to be exist");

      BitCastInst *BitCast = new BitCastInst(Fn, Call.getType(), "", &Call);
      BitCast->setDebugLoc(Call.getDebugLoc());
      for (User *U : Call.users())
        U->replaceUsesOfWith(&Call, BitCast);

      InstToRemove.insert(&Call);
      Modified = true;
    }
  }

  for (CallInst *Inst : InstToRemove)
    Inst->eraseFromParent();

  return Modified;
}

} // namespace llvm

// For legacy pass manager
namespace {
class VectorVariantFillInLegacy : public ModulePass {
public:
  static char ID;

  VectorVariantFillInLegacy() : ModulePass(ID) {
    initializeVectorVariantFillInLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    return llvm::VectorVariantFillIn().runImpl(M);
  }
};
} // namespace

INITIALIZE_PASS(VectorVariantFillInLegacy, "dpcpp-kernel-vector-variant-fillin",
                "Fill-in addresses of vector variants", false, false)

char VectorVariantFillInLegacy::ID = 0;

ModulePass *llvm::createVectorVariantFillInLegacyPass() {
  return new VectorVariantFillInLegacy();
}
