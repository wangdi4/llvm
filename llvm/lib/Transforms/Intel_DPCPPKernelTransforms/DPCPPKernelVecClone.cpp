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
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
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

template <> struct GraphTraits<User *> {
  using NodeRef = User *;
  using ChildIteratorType = Value::user_iterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }

  static inline ChildIteratorType child_begin(NodeRef N) {
    return N->user_begin();
  }

  static inline ChildIteratorType child_end(NodeRef N) {
    return N->user_end();
  }
};

using DefUseTreeChildSet = SmallPtrSet<Instruction *, 8>;
using DefUseTree = SmallDenseMap<Instruction *, DefUseTreeChildSet>;

/// The actions to take for the TID builtin functions.
enum class FnAction {
  MoveAndUpdateUses,       // Moves to entry block + update uses
  MoveAndUpdateUsesForDim, // Moves to entry block + update uses for a
  // specific dimension
  MoveOnly,            // Moves to entry block only
  AssertIfEncountered, // Assert false.
};

static ConstantInt *createVFConstant(LLVMContext &C, const DataLayout &DL,
                                     size_t VF) {
  assert(((DL.getPointerSizeInBits() == 64) ||
          (DL.getPointerSizeInBits() == 32)) &&
         "Unexpected ptr size!");
  return ConstantInt::get(Type::getIntNTy(C, DL.getPointerSizeInBits()), VF);
}

// TODO: this built-in should be processed in ported ResolveSubGroupWICall pass
// TODO: scalar remainder processing is incorrect (VectorLength==1) due to the
// unported masking
static bool ReplaceMaxSGSizeCall(Module &M) {
  if (Function *F = M.getFunction("__builtin_get_max_sub_group_size")) {
    std::vector<std::pair<Instruction *, Value *>> InstToReplace;
    // Collecting all __builtin_get_max_sub_group_size calls to replace them by
    // a constant VectorLength
    for (User *U : F->users()) {
      if (CallInst *CI = dyn_cast<CallInst>(U)) {
        unsigned VectorLength = 1;
        assert(CI->getFunction()->hasFnAttribute("vectorized_width") &&
               "Calling __builtin_get_max_sub_group_size function doesn't have "
               "a vectorized_width attribute. Try to inline it!");
        bool err = to_integer(CI->getFunction()
                                  ->getFnAttribute("vectorized_width")
                                  .getValueAsString(),
                              VectorLength);
        // Silence the warning to avoid querying attribute twice.
        (void)err;
        assert(err && "Can't read vectorized_width data!");
        Value *ConstVF = CastInst::CreateTruncOrBitCast(
            createVFConstant(M.getContext(), M.getDataLayout(), VectorLength),
            IntegerType::get(M.getContext(), 32), "max.sg.size", CI);
        InstToReplace.emplace_back(CI, ConstVF);
      }
    }

    // Replacing __builtin_get_max_sub_group_size calls
    for (const auto &pair : InstToReplace) {
      Instruction *from = pair.first;
      Value *to = pair.second;
      from->replaceAllUsesWith(to);
      from->eraseFromParent();
    }
    return true;
  }
  return false;
}

// TODO: replacing of this built-in by 0 after VecClone should be processed in
// ported ResolveSubGroupWICall pass
// TODO: scalar remainder processing is incorrect (sub_group.get_local_id()[0]
// == 0) due to the unported masking
static bool ReplaceSGLocalId(Module &M) {
  if (Function *F = M.getFunction("__builtin_get_sub_group_local_id")) {
    std::vector<std::pair<Instruction *, Value *>> InstToReplace;
    Value *ConstInt =
        ConstantInt::get(IntegerType::get(M.getContext(), 32), 0);
    // Collecting all __builtin_get_sub_group_local_id calls to replace them by
    // a constant integer == 0
    for (User *U : F->users()) {
      if (CallInst *CI = dyn_cast<CallInst>(U)) {
        InstToReplace.emplace_back(CI, ConstInt);
      }
    }

    // Replacing __builtin_get_sub_group_local_id calls
    for (const auto &pair : InstToReplace) {
      Instruction *from = pair.first;
      Value *to = pair.second;
      from->replaceAllUsesWith(to);
      from->eraseFromParent();
    }
    return true;
  }
  return false;
}


DPCPPKernelVecClone::DPCPPKernelVecClone() : ModulePass(ID), Impl() {
  initializeVecClonePass(*PassRegistry::getPassRegistry());
}

bool DPCPPKernelVecClone::runOnModule(Module &M) {
  Impl.TTIWP = &getAnalysis<TargetTransformInfoWrapperPass>();
  auto Res = Impl.runImpl(M);
  // Handling get_sub_group_local_id after updateAndMoveTID
  Res |= ReplaceSGLocalId(M);
  // Handling get_max_sub_group_size
  Res |= ReplaceMaxSGSizeCall(M);
  return (Res);
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

// Find all paths with shl/op.../ashr pattern by DFS, where op can be
// arbitrary number of add/sub/mul with constant values.
//
// Algorithm:
// For all Shl instructions of TIDCall, we traverse all its subtrees using
// DFS to find the pattern paths. If a subtree contains a unsupported node
// (e.g., any instructions other than shl/add/sub/mul/ashr), then we abandon
// the whole subtree even if it contains some shl/op.../ashr pattern.
// For example, given IR as follows,
//
//  %tid = call get_global_id()
//  %shl = shl i64 %tid, 32
//
//  %add = add i64 %shl, (1<<32)
//  %call = call @dummy(i64 %add)
//  %shr = ashr exact i64 %add, 32
//
//  %sub = sub i64 %shl, (1<<32)
//  %shr2 = ashr exact i64 %sub.shl, 32
//
// we can obtain its corresponding def-use tree,
//
//          %tid
//            |
//          %shl
//         /    \
//      %add    %sub
//      /   \     \
//   %call  %shr  %shr2
//
// The right subtree of %shl (%shl - %sub - %shr2) is returned to eliminate
// shl and ashr, while the left subtree is kept as-is even it contains a
// shl/add/ashr pattern (%shl - %add - %shr). Because if we eliminate the
// shl and shift back the constant addend in %add, %call will receive a wrong
// parameter.
//
// TODO:
// 1) If a shl is followed by a mul, then they can be combined by InstConmbine
//    pass, and a path may start with mul. We need to detect and handle this
//    pattern. E.g.,
//      %shl = shl i64 %gid, 32
//      %mul = mul i64 %shl, 2
//    can be combined to
//      %mul = mul i64 %gid, (2<<32)
// 2) A path may also terminates with an icmp instruction, then there will be
//    no ashr. We need also detect these patterns. E.g.,
//      %shr = ashr exact i64 %add, 32
//      %cmp = icmp eq i64 %shr, 1
//    can be combined to
//      %cmp = icmp eq i64 %add, (1<<32)
// 3) If a path starts with mul as in 1) and terminates with icmp as in 2),
//    then there will be no shl and ashr, and we may not handle such case.
static void findAllShlAShrPaths(
    Instruction *TIDCallInst, unsigned TruncatedToBitSize, DefUseTree &Paths) {
  for (User *ShlU : TIDCallInst->users()) {
    Instruction *Shl = dyn_cast<Instruction>(ShlU);
    if (!Shl || Shl->getOpcode() != Instruction::Shl)
      continue;

    for (User *U : Shl->users()) {
      DefUseTree CurrPaths;
      // I don't know why it's designed as such, but df_iterator will increase
      // itself by 1 after calling skipChildren(), so we cannot use ++It in the
      // for-clause (which makes the code really ugly).
      for (auto It = df_begin(U), End = df_end(U); It != End;) {
        Instruction *I = cast<Instruction>(*It);
        switch (I->getOpcode()) {
        case Instruction::Add:
        case Instruction::Sub:
        case Instruction::Mul:
          if (!isa<ConstantInt>(I->getOperand(0)) &&
              !isa<ConstantInt>(I->getOperand(1)))
            // add/sub/mul has no constant operands, thus abandon this subtree.
            goto Continue;
          break;
        case Instruction::AShr: {
          ConstantInt *AShrByVal = dyn_cast<ConstantInt>(I->getOperand(1));
          if (AShrByVal && AShrByVal->getZExtValue() == TruncatedToBitSize) {
            Instruction *LastI = Shl;
            // Add the path starting from shl and ending with ashr to CurrPaths
            for (unsigned i = 0, len = It.getPathLength(); i < len; i++) {
              Instruction *CurrI = cast<Instruction>(It.getPath(i));
              CurrPaths[LastI].insert(CurrI);
              LastI = CurrI;
            }
            It.skipChildren();
            continue;
          }
          // The 2nd oprand of Shl isn't expected, thus abandon this subtree.
          goto Continue;
        }
        default:
          // If we encounter any other instructions, abandon this subtree.
          goto Continue;
        }
        ++It;
      }

      // Merge the current paths into the result.
      Paths[TIDCallInst].insert(Shl);
      for (auto &KV : CurrPaths)
        Paths[KV.first].insert(KV.second.begin(), KV.second.end());
Continue:
      ;
    }
  }
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

  DefUseTree ShlAShrPaths;
  findAllShlAShrPaths(TIDCallInst, AddTypeSize, ShlAShrPaths);

  DefUseTreeChildSet &ShlAShrPathHeaders = ShlAShrPaths[TIDCallInst];

  // Replace all uses of TID with AddSExt, except truncating sequences that go
  // back to same size as Add. NOTE: This will also exclude TIDTrunc since Add
  // and Phi are of same type.
  TIDCallInst->replaceUsesWithIf(AddSExt, [Add](Use &U) {
    User *Usr = U.getUser();
    if (auto *UsrTruncInst = dyn_cast<TruncInst>(Usr))
      if (UsrTruncInst->getDestTy() == Add->getType())
        return false;
    return true;
  });

  // All the remaining users of TID are either TIDTrunc, truncating sequences
  // (shl + add/sub/mul... + ashr), or trunc instructions from incoming IR
  // which go back to same size as Add. The last case is trivial and can be
  // removed, with all their uses replaced directly by Add (or AddSExt).
  SmallVector<std::pair<Instruction * /* From */, Instruction * /* To */>, 4>
      ReplacePairs;
  for (auto *User : TIDCallInst->users()) {
    Instruction *UserInst = cast<Instruction>(User);

    if (UserInst == TIDTrunc)
      continue;

    if (isa<TruncInst>(UserInst)) {
      ReplacePairs.emplace_back(UserInst, Add);
    }
  }

  // Then, we eliminate shl and ashr in truncating sequences.
  for (auto Shl : ShlAShrPathHeaders) {
    for (auto ShlUser : ShlAShrPaths[Shl]) {
      // Replace Shl with AddSExt.
      unsigned ShlPos = ShlUser->getOperand(0) == Shl ? 0 : 1;
      ShlUser->setOperand(ShlPos, AddSExt);

      SmallVector<Instruction *, 8> WorkList;
      WorkList.push_back(ShlUser);
      do {
        Instruction *I = WorkList.pop_back_val();
        unsigned OpCode = I->getOpcode();
        // Eliminate AShr by replacing it with its 0-th operand.
        if (OpCode == Instruction::AShr) {
          ReplacePairs.emplace_back(I, cast<Instruction>(I->getOperand(0)));
          continue;
        }

        auto &Users = ShlAShrPaths[I];
        WorkList.append(Users.begin(), Users.end());

        // When id < 2^32,
        //   ((id << 32) * c) >> 32 == id * c,
        // so, we do nothing for mul instructions.
        if (OpCode == Instruction::Mul)
          continue;

        // Shift back all constant values in add/sub instructions.
        unsigned OpPos = 0;
        ConstantInt *Val = dyn_cast<ConstantInt>(I->getOperand(0));
        if (!Val) {
          Val = cast<ConstantInt>(I->getOperand(1));
          OpPos = 1;
        }
        ConstantInt *NewVal =
            ConstantInt::get(Val->getType(), Val->getSExtValue() >> AddTypeSize);
        I->setOperand(OpPos, NewVal);
      } while (!WorkList.empty());
    }
    if (Shl->getNumUses() == 0)
      Shl->eraseFromParent();
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
      {"__builtin_get_local_id", FnAction::MoveAndUpdateUsesForDim},
      {"__builtin_get_sub_group_local_id", FnAction::MoveAndUpdateUses}};

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
          // If the get-id calls return i32 (e.g., on 32-bit target), there's
          // no truncation, so we don't need to do special optimization.
          bool TIDIsInt32 = CI->getType()->isIntegerTy(32);
          if (!TIDIsInt32 &&
              FuncName == "__builtin_get_local_id" && LT2GigWorkGroupSize)
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
        // All the other Kernel function built-ins, if they have constant
        // arguments or don't have argument, then should just be moved at
        // the entry block.
        CI->moveBefore(EntryBlock->getTerminator());
        if (CI->arg_empty() || isa<Constant>(CI->getArgOperand(0)))
          CI->moveBefore(EntryBlock->getTerminator());
        break;
      case FnAction::AssertIfEncountered:
        assert(false && "Case hasn't been ported from OpenCL");
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
