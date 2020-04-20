//=DPCPPKernelVecClone.cpp - Vector function to loop transform -*- C++ -*----=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelVecClone.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPPrepareKernelForVecClone.h"

#define SV_NAME "dpcpp-kernel-vec-clone"

#define DEBUG_TYPE SV_NAME

using namespace llvm;

char DPCPPKernelVecClone::ID = 0;

// In DPCPP header we have only one type of TID which is local TID.
static cl::opt<bool> LT2GigWorkGroupSize(
    "dpcpp-kernel-less-than-two-gig-max-work-group-size", cl::init(true),
    cl::Hidden, cl::desc("Max work group size is less than 2 Gig elements."));

static const char lv_name[] = SV_NAME;
INITIALIZE_PASS_BEGIN(DPCPPKernelVecClone, SV_NAME, lv_name,
                      false /* not modifies CFG */, true /* transform pass */)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(DPCPPKernelVecClone, SV_NAME, lv_name,
                    false /* not modifies CFG */, true /* transform pass */)

namespace llvm {

/// The actions to take for the TID builtin functions.
enum class FnAction {
  MoveAndUpdateUses,       // Moves to entry block + update uses
  MoveAndUpdateUsesForDim, // Moves to entry block + update uses for a
  // specific dimension
  MoveOnly,            // Moves to entry block only
  AssertIfEncountered, // Assert false.
};

DPCPPKernelVecClone::DPCPPKernelVecClone() : ModulePass(ID), Impl() {
  initializeVecClonePass(*PassRegistry::getPassRegistry());
}

bool DPCPPKernelVecClone::runOnModule(Module &M) {
  Impl.TTIWP = &getAnalysis<TargetTransformInfoWrapperPass>();
  return Impl.runImpl(M);
}

DPCPPKernelVecCloneImpl::DPCPPKernelVecCloneImpl() : VecCloneImpl() {}

// Update references of functions
// Remove the "dpcpp_kernel_recommended_vector_length" attribute from the
// original kernel. "dpcpp_kernel_recommended_vector_length" attribute is used
// only by Kernel VecClone.
static void updateReferences(Function &F, Function *Clone) {
  // Remove "sycl_kernel" property from the Clone kernel.
  // This preserves the property that only original kernel is indicated
  // with this attribute. It can't be kept as vectorized kernel would get
  // its own treatment by WGLoopCreator pass (which breaks design).
  Clone->removeFnAttr("sycl_kernel");

  // Get VL from the attribute from the original kernel.
  unsigned VectorLength;
  bool err =
      to_integer(F.getFnAttribute("dpcpp_kernel_recommended_vector_length")
                     .getValueAsString(),
                 VectorLength);
  // Silence the warning to avoid querying attribute twice.
  (void)err;
  assert(err && "Can't read dpcpp_kernel_recommended_vector_length data!");
  // Set the "vector_width" attribute to the cloned kernel.
  Clone->addFnAttr("vectorized_kernel");
  Clone->addFnAttr("vectorized_width", utostr(VectorLength));
  // Set the attribute that points to the orginal kernel of the clone.
  Clone->addFnAttr("scalar_kernel", F.getName());

  // Set "vector_width" for the original kernel.
  F.addFnAttr("vectorized_width", utostr(1));
  F.addFnAttr("scalar_kernel");
  F.addFnAttr("vectorized_kernel", Clone->getName());
}

// Updates all the uses of TID calls with TID + new induction variable.
static void updateAndMoveTID(Instruction *TIDCallInstr, PHINode *Phi,
                             BasicBlock *EntryBlock) {
  IRBuilder<> IRB(Phi);
  IRB.SetInsertPoint(Phi->getNextNode());
  // Update the uses of the TID with TID+ind.
  Instruction *InductionSExt =
      cast<Instruction>(IRB.CreateSExtOrTrunc(Phi, TIDCallInstr->getType()));
  Instruction *Add = BinaryOperator::CreateNUWAdd(
      InductionSExt, UndefValue::get(InductionSExt->getType()), "add");
  Add->insertAfter(InductionSExt);
  TIDCallInstr->replaceAllUsesWith(Add);
  Add->setOperand(1, TIDCallInstr);
  // Move TID call outside of the loop.
  TIDCallInstr->moveBefore(EntryBlock->getTerminator());
}

// Check if a Value represents the following truncation pattern implemented
// using shl and ashr instructions -
// %1 = shl %0, <TruncatedToBitSize>
// %2 = ashr %1, <TruncatedToBitSize>
static bool isShlAshrTruncationPattern(Value *V, unsigned TruncatedToBitSize) {
  if (!isa<Instruction>(V))
    return false;

  Instruction *I = cast<Instruction>(V);

  // Capture only 'shl' instructions.
  if (I->getOpcode() != Instruction::Shl)
    return false;

  // Check if shift is done by a constant int value.
  if (!isa<ConstantInt>(I->getOperand(1)))
    return false;

  // Check if shift is happening to TruncatedToBitSize.
  if (cast<ConstantInt>(I->getOperand(1))->getZExtValue() != TruncatedToBitSize)
    return false;

  // 'shl' is expected to have only a single user, the 'ashr' instruction.
  if (I->getNumUses() != 1)
    return false;

  User *ShlSingleUsr = *I->user_begin();
  if (auto *ShlSingleUsrInst = dyn_cast<Instruction>(ShlSingleUsr)) {
    if (ShlSingleUsrInst->getOpcode() == Instruction::AShr) {
      Value *AshrByVal = ShlSingleUsrInst->getOperand(1);
      if (isa<ConstantInt>(AshrByVal) &&
          cast<ConstantInt>(AshrByVal)->getZExtValue() == TruncatedToBitSize)
        return true;
    }
  }

  // Invalid single user of 'shl'.
  return false;
}

static void optimizedUpdateAndMoveTID(Instruction *TIDCallInst, PHINode *Phi,
                                      BasicBlock *EntryBlock) {
  IRBuilder<> IRB(Phi);
  IRB.SetInsertPoint(Phi->getNextNode());
  // TODO: assertions for type of TIDCallInst and Phi
  // Truncate TID to Phi's type (we know TID call return value is in 32-bit
  // range, check LT2GBWorkGroupSize and LT2GigGlobalWorkSize).
  Instruction *TIDTrunc =
      cast<Instruction>(IRB.CreateTrunc(TIDCallInst, Phi->getType()));
  // Generates TID+ind.
  Instruction *Add = cast<Instruction>(IRB.CreateNUWAdd(TIDTrunc, Phi, "add"));
  unsigned AddTypeSize = Add->getType()->getPrimitiveSizeInBits();
  // Sign extend result to 64-bit (TIDCallInst's type)
  Instruction *AddSExt =
      cast<Instruction>(IRB.CreateSExt(Add, TIDCallInst->getType()));
  // Replace all uses of TID with AddSExt, except truncating sequences that go
  // back to same size as Add. NOTE: This will also exclude TIDTrunc since Add
  // and Phi are of same type.
  TIDCallInst->replaceUsesWithIf(AddSExt, [Add, AddTypeSize](Use &U) {
    User *Usr = U.getUser();
    if (auto *UsrTruncInst = dyn_cast<TruncInst>(Usr)) {
      if (UsrTruncInst->getDestTy() == Add->getType())
        return false;
    }
    if (isShlAshrTruncationPattern(Usr, AddTypeSize))
      return false;
    return true;
  });

  // All the remaining users of TID are either TIDTrunc, trunc instructions or
  // truncating sequences (shl + ashr) from incoming IR which go back to same
  // size as Add. The last two cases are trivial and can be removed, with all
  // their uses replaced directly by Add (or AddSExt).
  SmallVector<std::pair<Instruction * /* From */, Instruction * /* To */>, 4>
      ReplacePairs;
  for (auto *User : TIDCallInst->users()) {
    assert((isa<TruncInst>(User) ||
            isShlAshrTruncationPattern(User, AddTypeSize)) &&
           "Invalid remaining user of TID.");
    Instruction *UserInst = cast<Instruction>(User);

    if (UserInst == TIDTrunc)
      continue;

    if (isa<TruncInst>(UserInst)) {
      ReplacePairs.emplace_back(UserInst, Add);
    } else {
      Instruction *AshrInst = cast<Instruction>(*UserInst->user_begin());
      ReplacePairs.emplace_back(AshrInst, AddSExt);
      // The shl instruction using TID call needs to be only removed. A
      // replacement is not needed since its only user (ashr) is already
      // removed.
      ReplacePairs.emplace_back(UserInst, nullptr);
    }
  }

  for (auto &ReplacePair : ReplacePairs) {
    Instruction *From = ReplacePair.first;
    Instruction *To = ReplacePair.second;

    assert((To || From->hasNUses(0)) &&
           "Invalid instruction to replace/remove.");

    if (To)
      From->replaceAllUsesWith(To);
    From->eraseFromParent();
  }

  // Reset the operand of TID's trunc, after all uses of TID are replaced.
  assert(TIDTrunc->getOperand(0) == TIDCallInst && "TIDTrunc is corrupted.");
  // Move TID and its trunc call outside of the loop.
  TIDCallInst->moveBefore(EntryBlock->getTerminator());
  TIDTrunc->moveBefore(EntryBlock->getTerminator());
}

void DPCPPKernelVecCloneImpl::handleLanguageSpecifics(Function &F, PHINode *Phi,
                                                      Function *Clone,
                                                      BasicBlock *EntryBlock) {
  // The FunctionsAndActions array has only the Kernel function built-ins that
  // are uniform.
  std::pair<std::string, FnAction> FunctionsAndActions[] = {
      {"__builtin_get_local_id", FnAction::MoveAndUpdateUsesForDim}};

  // Collect all Kernel function built-ins.
  for (const auto &Pair : FunctionsAndActions) {
    const auto &FuncName = Pair.first;
    auto Action = Pair.second;

    // Early exit if the function is not present.
    Function *Func = Clone->getParent()->getFunction(FuncName);
    if (!Func)
      continue;

    for (User *U : Func->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      assert(CI && "Unexpected use of Kernel function built-ins.");
      Function *parentFunc = CI->getParent()->getParent();
      if (parentFunc != Clone)
        continue;

      switch (Action) {
      case FnAction::MoveAndUpdateUsesForDim: {
        ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
        assert(C && "The function argument must be constant");
        unsigned dim = C->getValue().getZExtValue();
        assert(dim < 3 && "Dimension is not in range");
        if (dim == 0) {
          // Currently, only zero dimension is vectorized.
          if (FuncName == "__builtin_get_local_id" && LT2GigWorkGroupSize)
            optimizedUpdateAndMoveTID(CI, Phi, EntryBlock);
          else
            updateAndMoveTID(CI, Phi, EntryBlock);
        } else
          CI->moveBefore(EntryBlock->getTerminator());
        break;
      }
      case FnAction::MoveAndUpdateUses:
        updateAndMoveTID(CI, Phi, EntryBlock);
        break;
      case FnAction::MoveOnly:
        // All the other Kernel function built-ins should just be moved at
        // the entry block.
        CI->moveBefore(EntryBlock->getTerminator());
        break;
      case FnAction::AssertIfEncountered:
        assert(false && "Case hasn't been ported from OpenCL");
      default:
        llvm_unreachable("Unexpected Action");
      };
    }
  }

  updateReferences(F, Clone);

  // TODO: Assign vector variants for functions inside the kernel.
}

void DPCPPKernelVecCloneImpl::languageSpecificInitializations(Module &M) {
  SmallVector<Function *, 8> WorkList;

  for (auto &F : M) {
    if (F.hasFnAttribute("sycl_kernel"))
      WorkList.push_back(&F);
  }

  if (WorkList.empty()) {
    LLVM_DEBUG(dbgs() << lv_name << ":"
                      << "No kernels found!\n");
    return;
  }

  for (auto *F : WorkList) {
    // TODO: we might want to have certain conditions that would result
    // in no vectoriation, but until then...
    DPCPPPrepareKernelForVecClone PK(F, TTIWP->getTTI(*F));
    PK.run();
  }
}

ModulePass *createDPCPPKernelVecClonePass() {
  return new DPCPPKernelVecClone();
}

} // end namespace llvm
