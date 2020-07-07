//=---- OCLVecClone.cpp - Vector function to loop transform -*- C++ -*----=//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// OCLVecClone pass is an OpenCL specialization of the VecClone pass which does
/// the following:
/// 1. Emits the vector-variant attributes (languageSpecificInitializations)
///    that activates VecClone.
/// 2. Updates all the uses of the TID calls with TID + new induction variable
///    and moves the TID call ut of the loop that is emitted by VecClone
///    (handleLanguageSpecifics).
///
/// Example:
/// original kernel:
///   i = get_global_id();
///   A[i] = ...
///
/// after OCLVecClone pass:
///   i = get_global_id();
///   for (j = 0; j < VF; j++){
///      A[i+j] = ...
///   }
///
/// 3. Updates the metadata that later passes use.
// ===--------------------------------------------------------------------=== //
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "MetadataAPI.h"
#include "NameMangleAPI.h"
#include "OCLPrepareKernelForVecClone.h"
#include "OCLVecClone.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"

#include <string>

#define DEBUG_TYPE "OCLVecClone"
#define SV_NAME "ocl-vecclone"
#define SV_NAME1 "ocl-reqd-sub-group-size"

using namespace llvm;
using namespace Intel::MetadataAPI;

static cl::opt<std::string> ReqdSubGroupSizes("reqd-sub-group-size", cl::init(""),
                                        cl::Hidden,
                                        cl::desc("Per-kernel required subgroup"
                                                 "size. Comma separated list of"
                                                 " name(num)"));

static cl::opt<bool> LT2GigWorkGroupSize(
    "less-than-two-gig-max-work-group-size", cl::init(true), cl::Hidden,
    cl::desc("Max work group size is less than 2 Gig elements."));

static cl::opt<bool> LT2GigGlobalWorkSize(
    "less-than-two-gig-max-global-work-size", cl::init(false), cl::Hidden,
    cl::desc("Max global work size (global_work_offset + total work items) is "
             "less than 2 Gig elements."));

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
} // namespace llvm

namespace intel {

using DefUseTreeChildSet = SmallPtrSet<Instruction *, 8>;
using DefUseTree = SmallDenseMap<Instruction *, DefUseTreeChildSet>;

using ContainerTy = std::vector<std::pair<std::string, VectorVariant>>;
static ContainerTy OCLBuiltinVecInfo();
using ReturnInfoTy = std::vector<std::pair<std::string, VectorKind>>;
static ReturnInfoTy PopulateOCLBuiltinReturnInfo();

char OCLVecClone::ID = 0;
static const char lv_name[] = "OCLVecClone";
OCL_INITIALIZE_PASS_BEGIN(OCLVecClone, SV_NAME, lv_name,
                          false /* modifies CFG */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLVecClone, SV_NAME, lv_name,
                        false /* modififies CFG */, false /* transform pass */)

OCLVecClone::OCLVecClone(const Intel::CPUId *CPUId)
    : ModulePass(ID), Impl(CPUId) {
  initializeVecClonePass(*PassRegistry::getPassRegistry());
}

OCLVecClone::OCLVecClone() : OCLVecClone(nullptr) {}

bool OCLVecClone::runOnModule(Module &M) {
  Impl.setDimChooser(getAnalysisIfAvailable<ChooseVectorizationDimensionModulePass>());
  return Impl.runImpl(M);
}

OCLVecCloneImpl::OCLVecCloneImpl(const Intel::CPUId *CPUId)
    : VecCloneImpl(), CPUId(CPUId) {
  V_INIT_PRINT;
}

OCLVecCloneImpl::OCLVecCloneImpl() : VecCloneImpl() {}

// Remove the "ocl_recommended_vector_length" metadata from the original kernel.
// "ocl_recommened_vector_length" metadata is used only by OCLVecClone. The rest
// of the Volcano passes recognize the "vector_width" metadata. Thus, we add
// "vector_width" metadata to the original kernel and the cloned kernel.
static void updateMetadata(Function &F, Function *Clone,
                           unsigned VecDim, bool CanUniteWorkgroups) {
  auto FMD = KernelInternalMetadataAPI(&F);
  auto KMD = KernelMetadataAPI(&F);
  auto CloneMD = KernelInternalMetadataAPI(Clone);
  // Get VL from the metadata from the original kernel.
  unsigned VectorLength = FMD.OclRecommendedVectorLength.get();
  // Set the "vector_width" metadata to the cloned kernel.
  CloneMD.VectorizedKernel.set(nullptr);
  CloneMD.VectorizedWidth.set(VectorLength);
  CloneMD.VectorizationDimension.set(VecDim);
  // Set the metadata that points to the orginal kernel of the clone.
  CloneMD.ScalarizedKernel.set(&F);
  CloneMD.CanUniteWorkgroups.set(CanUniteWorkgroups);

  // Set "vector_width" for the original kernel.
  FMD.VectorizedWidth.set(1);
  FMD.ScalarizedKernel.set(nullptr);

  if (F.getFunctionType() == Clone->getFunctionType())
    FMD.VectorizedKernel.set(Clone);
  else // Vectorized kernel with mask
    FMD.VectorizedMaskedKernel.set(Clone);
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

void OCLVecCloneImpl::handleLanguageSpecifics(Function &F, PHINode *Phi,
                                              Function *Clone,
                                              BasicBlock *EntryBlock) {
  // The FunctionsAndActions array has only the OpenCL function built-ins that
  // are uniform.
  std::pair<std::string, FnAction> FunctionsAndActions[] = {
      std::make_pair(CompilationUtils::mangledGetGID(),
                     FnAction::MoveAndUpdateUsesForDim),
      std::make_pair(CompilationUtils::mangledGetLID(),
                     FnAction::MoveAndUpdateUsesForDim),
      std::make_pair(CompilationUtils::mangledGetSubGroupLID(),
                     FnAction::MoveAndUpdateUses),
      std::make_pair(CompilationUtils::mangledGetGlobalSize(),
                     FnAction::MoveOnly),
      std::make_pair(CompilationUtils::mangledGetGlobalOffset(),
                     FnAction::MoveOnly),
      std::make_pair(CompilationUtils::mangledGetGroupID(), FnAction::MoveOnly),
      std::make_pair(CompilationUtils::mangledGetSubGroupSize(),
                     FnAction::MoveOnly),
      std::make_pair(CompilationUtils::mangledGetLocalSize(),
                     FnAction::MoveOnly),
      std::make_pair(CompilationUtils::mangledGetEnqueuedLocalSize(),
                     FnAction::MoveOnly),
      std::make_pair(CompilationUtils::mangledGetGlobalLinearId(),
                     FnAction::AssertIfEncountered),
      std::make_pair(CompilationUtils::mangledGetLocalLinearId(),
                     FnAction::AssertIfEncountered)};

  unsigned VecDim;
  bool CanUniteWorkgroups;
  if (DimChooser) {
    VecDim = DimChooser->getVectorizationDim(&F);
    CanUniteWorkgroups = DimChooser->getCanUniteWorkgroups(&F);
  } else {
    VecDim = 0;
    CanUniteWorkgroups = false;
  }

  // Collect all OpenCL function built-ins.
  for (const auto &Pair : FunctionsAndActions) {
    const auto &FuncName = Pair.first;
    auto Action = Pair.second;
    Function *Func = Clone->getParent()->getFunction(FuncName);
    if (!Func)
      continue;

    for (User *U : Func->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      assert(CI && "Unexpected use of OpenCL function built-ins.");
      Function *parentFunc = CI->getParent()->getParent();
      if (parentFunc != Clone)
        continue;
      switch (Action) {
      case FnAction::MoveAndUpdateUsesForDim: {
        ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
        assert(C && "The function argument must be constant");
        unsigned dim = C->getValue().getZExtValue();
        assert(dim < 3 && "Argument is not in range");
        if (dim == VecDim) {
          if (FuncName == CompilationUtils::mangledGetLID() &&
              LT2GigWorkGroupSize)
            optimizedUpdateAndMoveTID(CI, Phi, EntryBlock);
          else if (FuncName == CompilationUtils::mangledGetGID() &&
                   LT2GigGlobalWorkSize)
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
        // All the other OpenCL function built-ins should just be moved at
        // the entry block.
        CI->moveBefore(EntryBlock->getTerminator());
        break;
      case FnAction::AssertIfEncountered:
        assert(
            Func && FuncName != CompilationUtils::mangledGetGlobalLinearId() &&
            FuncName != CompilationUtils::mangledGetLocalLinearId() &&
            "get_global_linear_id() and get_local_linear_id() should have been "
            "resolved in earlier passes");
      default:
        llvm_unreachable("Unexpected Action");
      }
    }
  }

  updateMetadata(F, Clone, VecDim, CanUniteWorkgroups);

  std::vector<std::pair<const char*, std::string>> VectInfoStr = {
    #include "VectInfo.gen"
    {"intel_sub_group_ballot", VectorVariant{VectorVariant::ISAClass::XMM, true, 4,
     {VectorKind::vector()}, "", "intel_sub_group_ballot_vf4"}.toString()},
    {"intel_sub_group_ballot", VectorVariant{VectorVariant::ISAClass::XMM, true, 8,
     {VectorKind::vector()}, "", "intel_sub_group_ballot_vf8"}.toString()},
    {"intel_sub_group_ballot", VectorVariant{VectorVariant::ISAClass::XMM, true, 16,
     {VectorKind::vector()}, "", "intel_sub_group_ballot_vf16"}.toString()},
  };
  ContainerTy VectInfo;
  for (auto &vi : VectInfoStr) {
    VectInfo.emplace_back(vi.first, std::move(vi.second));
  }
  static ContainerTy VecInfo = std::move(VectInfo);

  for (auto &Inst : instructions(Clone)) {
    auto *Call = dyn_cast<CallInst>(&Inst);
    if (!Call)
      continue;

    Function *CalledFunc = Call->getCalledFunction();
    if (!CalledFunc)
      continue;

    auto FnName = CalledFunc->getName();
    unsigned VF = KernelInternalMetadataAPI(Clone).VectorizedWidth.get();

    // May be more than one entry, e.g. mask/unmasked (although currently that's
    // not the case).
    auto MatchingVariants = make_filter_range(
        VecInfo,
        [FnName, VF](std::pair<std::string, VectorVariant> Info) -> bool {
          return Info.first == FnName && Info.second.getVlen() == VF;
        });

    if (MatchingVariants.begin() == MatchingVariants.end())
      continue;

    std::string Variants;
    assert(!Call->hasFnAttr("vector-variants") &&
           "Unexpected vector-variants attribute for OpenCL builtin!");

    // This condition isn't expected to happen, but do the right thing anyway.
    if (Call->hasFnAttr("vector-variants"))
      Variants = std::string(Call->getFnAttr("vector-variants").getValueAsString());

    // Indicates the call must have mask arg.
    bool HasMask = true;
    // Indicates the call must not mask arg.
    bool NotHasMask = true;
    for (auto &Variant : MatchingVariants) {
      if (!Variants.empty())
        Variants += ',';

      Variants +=  Variant.second.toString();
      if (Variant.second.isMasked())
        NotHasMask = false;
      else
        HasMask = false;
    }

    AttributeList AL = Call->getAttributes();

    AL = AL.addAttribute(Call->getContext(), AttributeList::FunctionIndex,
                         "vector-variants", Variants);
    // TODO: So far the functions that have their vector variants assigned here
    // are essentially "kernel-call-once" functions.
    AL = AL.addAttribute(Call->getContext(), AttributeList::FunctionIndex,
                         CompilationUtils::ATTR_KERNEL_CALL_ONCE);
    if (HasMask)
      AL = AL.addAttribute(Call->getContext(), AttributeList::FunctionIndex,
                           CompilationUtils::ATTR_HAS_VPLAN_MASK);
    else if (!NotHasMask) {
      unsigned ParamsNum = Call->arg_size();
      AL = AL.addAttribute(Call->getContext(), AttributeList::FunctionIndex,
                           "call-params-num", std::to_string(ParamsNum));
    }
    Call->setAttributes(AL);
  }
}

static ReturnInfoTy PopulateOCLBuiltinReturnInfo() {
  // sub_group_get_local_id isn't here due to special processing in VecClone
  // pass. Void-argument functions returning uniform values are implicitly
  // known as uniform too.
  ReturnInfoTy RetInfo;

  // Work group uniform built-ins
  RetInfo.push_back({"_Z14work_group_alli", VectorKind::uniform()});
  RetInfo.push_back({"_Z14work_group_anyi", VectorKind::uniform()});
  const char WorkGroupTypes[] = {'i', 'j', 'l', 'm', 'f', 'd'};
  for (char Type : WorkGroupTypes) {
    RetInfo.push_back({std::string("_Z20work_group_broadcast") + Type + "m", VectorKind::uniform()});
    RetInfo.push_back({std::string("_Z20work_group_broadcast") + Type + "mm", VectorKind::uniform()});
    RetInfo.push_back({std::string("_Z20work_group_broadcast") + Type + "mmm", VectorKind::uniform()});

    for (auto Op : {"add", "min", "max"})
      RetInfo.push_back({std::string("_Z21work_group_reduce_") + Op + Type, VectorKind::uniform()});
  }

  // Sub group uniform built-ins
  RetInfo.push_back({std::string("_Z13sub_group_alli"), VectorKind::uniform()});
  RetInfo.push_back({std::string("_Z13sub_group_anyi"), VectorKind::uniform()});

  RetInfo.push_back({std::string("_Z22intel_sub_group_balloti"), VectorKind::uniform()});

  const char SubGroupTypes[] = {'i', 'j', 'l', 'm', 'f', 'd'};
  for (char Type : SubGroupTypes) {
    RetInfo.push_back({std::string("_Z19sub_group_broadcast") + Type + 'j', VectorKind::uniform()});
    for (auto Op : {"add", "min", "max"})
      RetInfo.push_back({std::string("_Z20sub_group_reduce_") + Op + Type,  VectorKind::uniform()});
  }

  const char IntelSubGroupTypes[] = {'c', 'h', 's', 't'};
  for (char Type : IntelSubGroupTypes) {
    RetInfo.push_back({std::string("_Z25intel_sub_group_broadcast") + Type + 'j', VectorKind::uniform()});
    for (auto Op : {"add", "min", "max"})
      RetInfo.push_back({std::string("_Z26intel_sub_group_reduce_") + Op + Type,  VectorKind::uniform()});
  }

  // Pipe functions
  RetInfo.push_back({std::string("__work_group_reserve_write_pipe"), VectorKind::uniform()});
  RetInfo.push_back({std::string("__work_group_reserve_read_pipe"), VectorKind::uniform()});

  return RetInfo;
}

void OCLVecCloneImpl::languageSpecificInitializations(Module &M) {
  OCLPrepareKernelForVecClone PK(CPUId);

  // FIXME: Longer term plan is to make the return value propery part of
  // VectorVariant encoding
  //
  // Also, note that we're annotating declarations here. It's legal because:
  //   - The uniformity is true for all the VFs possible
  //   - The attribute doesn't have any other existing meaning so we are free to
  //     choose what is suitable. Not putting too much effort into designing the
  //     attribute due to it being a temporary solution (see FIXME above).
  static auto OCLBuiltinReturnInfo = PopulateOCLBuiltinReturnInfo();
  for (auto &Entry : OCLBuiltinReturnInfo) {
    StringRef ScalarFnName = Entry.first;
    Function *Fn = M.getFunction(ScalarFnName);
    if (!Fn)
      continue;

    assert(Entry.second.isUniform() && "Only uniforms are supported by now!");
    Fn->addFnAttr("opencl-vec-uniform-return");
  }
  // Process async_work_group copies separately as it is easier to detect
  // them via unmangling as the number of overloads is high.
  for (auto &F : M) {
    if (!F.isDeclaration())
      continue;
    if (CompilationUtils::isAsyncWorkGroupCopy(std::string(F.getName())) ||
        CompilationUtils::isAsyncWorkGroupStridedCopy(std::string(F.getName())))
      F.addFnAttr("opencl-vec-uniform-return");
  }

  // Mark "kernel-uniform-call" (see LangRef for more details).
  CompilationUtils::FunctionSet oclSyncBuiltins;
  CompilationUtils::getAllSyncBuiltinsDclsForKernelUniformCallAttr(oclSyncBuiltins, &M);
  // process call sites
  for (auto *F : oclSyncBuiltins) {
    for (auto *U : F->users()) {
      if (auto *CI = dyn_cast<CallInst>(U)) {
        CI->setAttributes(CI->getAttributes()
          .addAttribute(CI->getContext(), AttributeList::FunctionIndex, "kernel-uniform-call"));
      }
    }
  }

  auto Kernels = KernelList(*&M).getList();

  // Checks for some common module errors.
  if (Kernels.empty()) {
    V_PRINT(wrapper, "Failed to find kernel annotation. Aborting!\n");
    return;
  }

  unsigned NumOfKernels = Kernels.size();
  if (NumOfKernels == 0) {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return;
  }

  for (Function *F : Kernels) {
    auto FMD = KernelInternalMetadataAPI(F);
    unsigned VectorLength = FMD.OclRecommendedVectorLength.get();
    if (VectorLength > 1)
      PK.run(F);
  }
}

char OCLReqdSubGroupSize::ID = 0;
static const char lv_name1[] = "OCLReqdSubGroupSize";
OCL_INITIALIZE_PASS_BEGIN(OCLReqdSubGroupSize, SV_NAME1, lv_name1,
                          true /* CFG unchanged */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLReqdSubGroupSize, SV_NAME1, lv_name1,
                        true /* CFG unchanged */, false /* transform pass */)

OCLReqdSubGroupSize::OCLReqdSubGroupSize() : ModulePass(ID) {}

bool OCLReqdSubGroupSize::runOnModule(Module &M) {
  // Split name1(n1),name2(n2),name3(n3)... into
  //    name1(n1)
  //    name2(n2)
  //    name3(n3)
  //    ...
  StringRef Sizes(ReqdSubGroupSizes);
  SmallVector<StringRef, 3> VSizes;
  Sizes.split(VSizes, ',', -1, false /* KeepEmpty */);

  // Match up each Kernel against each name(num)
  auto Kernels = KernelList(*&M).getList();
  for (Function *F : Kernels) {
    auto KMD = KernelMetadataAPI(F);
    StringRef FName(F->getName());
    auto FNameLen = FName.size();

    // Process each SubGrpSize specifier represented in "name(num)"
    for (auto &SubGrpSize : VSizes) {
      auto Len = SubGrpSize.size();
      if (!SubGrpSize.startswith(FName))
        continue; // Name should match
      if (SubGrpSize.rfind('(') != FNameLen || SubGrpSize.find(')') != Len-1)
        continue; // ( and ) should be found in the correct locations.
      auto SubStr = SubGrpSize.substr(FNameLen+1, Len-FNameLen-2); // "num"
      size_t ReqdSubGrpSize = 0;
      if (SubStr.getAsInteger(10 /* radix */, ReqdSubGrpSize))
        continue;
      // Process valid values only.
      if (ReqdSubGrpSize != 0 && ReqdSubGrpSize != 1 &&
          ReqdSubGrpSize != 2 && ReqdSubGrpSize != 4 &&
          ReqdSubGrpSize != 8 && ReqdSubGrpSize != 16 &&
          ReqdSubGrpSize != 32 && ReqdSubGrpSize != 64)
        continue;
      // Set required sub group size to the kernel.
      KMD.setReqdIntelSGSize(ReqdSubGrpSize);
      // We could actually transform LLVM IR to set the kernel
      // attribute, but it won't be recaptured by any OCL passes.
      // Sub group size info is also stored in the kernel property
      // at the build time, but that is not referenced by vectorizer
      // either.
    }
  }
  return false;
}
} // namespace intel

extern "C" Pass *createOCLVecClonePass(const Intel::CPUId *CPUId) {
  return new intel::OCLVecClone(CPUId);
}

extern "C" Pass *createOCLReqdSubGroupSizePass() {
  return new intel::OCLReqdSubGroupSize();
}
