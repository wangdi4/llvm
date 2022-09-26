//=DPCPPKernelVecClone.cpp - Vector function to loop transform -*- C++ -*----=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// DPCPPKernelVecClone pass is an OpenCL/DPC++ specialization of the VecClone
/// pass which does the following:
/// 1. Emits the vector-variant attributes (languageSpecificInitializations)
///    that activates VecClone.
/// 2. Updates all the uses of the TID calls with TID + new induction variable
///    and moves the TID call out of the loop that is emitted by VecClone
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelVecClone.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPPrepareKernelForVecClone.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/VectorizerUtils.h"

#define SV_NAME "dpcpp-kernel-vec-clone"

#define DEBUG_TYPE SV_NAME

using namespace llvm;
using namespace llvm::CompilationUtils;
using namespace llvm::DPCPPKernelMetadataAPI;

// In DPCPP header we have only one type of TID which is local TID.
static cl::opt<bool> LT2GigWorkGroupSize(
    "dpcpp-less-than-two-gig-max-work-group-size", cl::init(true), cl::Hidden,
    cl::desc("Max work group size is less than 2 Gig elements."));

enum GlobalWorkSizeLT2GState : uint8_t { GWS_FALSE, GWS_TRUE, GWS_AUTO };
static cl::opt<GlobalWorkSizeLT2GState> LT2GigGlobalWorkSize(
    "dpcpp-less-than-two-gig-max-global-work-size", cl::init(GWS_AUTO),
    cl::Hidden,
    cl::desc("Max global work size (global_work_offset + total work items) is "
             "less than 2 Gig elements."),
    cl::values(clEnumValN(GWS_AUTO, "auto", ""),
               clEnumValN(GWS_TRUE, "true", ""),
               clEnumValN(GWS_FALSE, "false", "")));

extern bool DPCPPEnableDirectFunctionCallVectorization;
extern bool DPCPPEnableSubgroupDirectCallVectorization;

extern cl::opt<VFISAKind> IsaEncodingOverride;

// Static container storing all the vector info entries.
// Each entry would be a tuple of three strings:
// 1. scalar variant name
// 2. "kernel-call-once" | ""
// 3. mangled vector variant name
static std::vector<std::tuple<std::string, std::string, std::string>>
    ExtendedVectInfos;

using DefUseTreeChildSet = SmallPtrSet<Instruction *, 8>;
using DefUseTree = SmallDenseMap<Instruction *, DefUseTreeChildSet>;

namespace {

/// The actions to take for the TID builtin functions.
enum class FnAction {
  MoveAndUpdateUses,       // Moves to entry block + update uses
  MoveAndUpdateUsesForDim, // Moves to entry block + update uses for a
  // specific dimension
  MoveOnly,            // Moves to entry block only
  AssertIfEncountered, // Assert false.
  UpdateOnly,          // Update use with ind
};

class DPCPPKernelVecCloneLegacy : public ModulePass {
private:
  DPCPPKernelVecCloneImpl Impl;

public:
  static char ID;

  explicit DPCPPKernelVecCloneLegacy(
      ArrayRef<VectItem> VectInfos = {},
      VFISAKind ISA = VFISAKind::SSE, bool IsOCL = false);

  bool runOnModule(Module &M) override {
    auto *VD = getAnalysisIfAvailable<VectorizationDimensionAnalysisLegacy>();
    Impl.setVectorizationDimensionMap(VD ? &VD->getResult() : nullptr);
    return Impl.runImpl(M);
  }

  /// Returns the name of the pass.
  llvm::StringRef getPassName() const override {
    return "DPCPPKernelVecCloneLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // For SYCL program, we always do vectorization on dim 0, so the pass won't
    // be added into the pipeline.
    AU.addUsedIfAvailable<VectorizationDimensionAnalysisLegacy>();
    AU.addPreserved<VectorizationDimensionAnalysisLegacy>();
  }
};

} // namespace

char DPCPPKernelVecCloneLegacy::ID = 0;

static const char lv_name[] = SV_NAME;
INITIALIZE_PASS_BEGIN(DPCPPKernelVecCloneLegacy, SV_NAME, lv_name,
                      false /* not modifies CFG */, false /* is_analysis */)
INITIALIZE_PASS_DEPENDENCY(VectorizationDimensionAnalysisLegacy)
INITIALIZE_PASS_END(DPCPPKernelVecCloneLegacy, SV_NAME, lv_name,
                    false /* not modifies CFG */, false /* is_analysis */)

DPCPPKernelVecCloneLegacy::DPCPPKernelVecCloneLegacy(
    ArrayRef<VectItem> VectInfos, VFISAKind ISA, bool IsOCL)
    : ModulePass(ID), Impl(VectInfos, ISA, IsOCL) {
  initializeDPCPPKernelVecCloneLegacyPass(*PassRegistry::getPassRegistry());
}

ModulePass *llvm::createDPCPPKernelVecClonePass(ArrayRef<VectItem> VectInfos,
                                                VFISAKind ISA,
                                                bool IsOCL) {
  return new DPCPPKernelVecCloneLegacy(VectInfos, ISA, IsOCL);
}

DPCPPKernelVecClonePass::DPCPPKernelVecClonePass(ArrayRef<VectItem> VectInfos,
                                                 VFISAKind ISA,
                                                 bool IsOCL)
    : Impl(VectInfos, ISA, IsOCL) {}

PreservedAnalyses DPCPPKernelVecClonePass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  Impl.setVectorizationDimensionMap(
      AM.getCachedResult<VectorizationDimensionAnalysis>(M));
  return Impl.runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

// "recommended_vector_length" metadata is removed in DPCPPKernelPostVec.
// "recommened_vector_length" metadata is used only by DPCPPKernelVecClone.
// The rest DPCPP kernel transform passes recognize the "vector_width" metadata.
// Thus, we add "vector_width" metadata to original kernel and cloned kernel.
static void updateKernelMetadata(Function &F, Function *Clone, unsigned VecDim,
                                 bool CanUniteWorkgroups) {
  KernelInternalMetadataAPI KIMD(&F);
  // Get VL from the metadata from the original kernel.
  unsigned VectorLength =
      KIMD.RecommendedVL.hasValue() ? KIMD.RecommendedVL.get() : 1;

  KernelInternalMetadataAPI CKIMD(Clone);
  // Set the "vector_width" metadata to the cloned kernel.
  CKIMD.VectorizedWidth.set(VectorLength);
  CKIMD.VectorizationDimension.set(VecDim);
  // Set the metadata that points to the orginal kernel of the clone.
  CKIMD.ScalarKernel.set(&F);
  CKIMD.CanUniteWorkgroups.set(CanUniteWorkgroups);

  // Set "vector_width" for the original kernel.
  KIMD.VectorizedWidth.set(1);

  if (F.getFunctionType() == Clone->getFunctionType())
    KIMD.VectorizedKernel.set(Clone);
  else
    KIMD.VectorizedMaskedKernel.set(Clone);
}

// Updates all the uses of TID calls with TID + new induction variable.
static void updateAndMoveTID(Instruction *TIDCallInstr, PHINode *Phi,
                             BasicBlock *EntryBlock) {
  IRBuilder<> IRB(&*Phi->getParent()->getFirstInsertionPt());
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

static void updateTID(Instruction *TIDCallInstr, PHINode *Phi) {
  Instruction *IP = &*Phi->getParent()->getFirstInsertionPt();
  IRBuilder<> IRB(IP);
  // Update the uses of the TID with ind.
  Instruction *InductionSExt =
      cast<Instruction>(IRB.CreateSExtOrTrunc(Phi, TIDCallInstr->getType()));
  TIDCallInstr->replaceAllUsesWith(InductionSExt);
}

static void optimizedUpdateTID(Instruction *TIDCallInstr, PHINode *Phi) {
  Instruction *IP = &*Phi->getParent()->getFirstInsertionPt();
  IRBuilder<> IRB(IP);
  SmallVector<ZExtInst *> WorkList;

  for (auto *User : TIDCallInstr->users()) {
    if (auto *ZExt = dyn_cast<ZExtInst>(User))
      WorkList.push_back(ZExt);
  }

  for (auto *ZExt : WorkList) {
    Instruction *SExt = cast<Instruction>(
      IRB.CreateSExt(TIDCallInstr, ZExt->getType(),
                     TIDCallInstr->getName() + ".sext"));
    ZExt->replaceAllUsesWith(SExt);
    ZExt->eraseFromParent();
  }

  // Now process TID.
  updateTID(TIDCallInstr, Phi);
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

// Utility to check if TID call has trunc or shl users.
static bool HasTruncOrShlUsers(CallInst *CI) {
  for (auto *U : CI->users()) {
    for (auto It = df_begin(U), E = df_end(U); It != E;) {
      if (auto *I = dyn_cast<Instruction>(*It)) {
        switch (I->getOpcode()) {
        case Instruction::Add:
        case Instruction::Sub:
        case Instruction::PHI:
        case Instruction::Select:
          It++;
          break;
        case Instruction::Shl:
        case Instruction::Trunc:
          return true;
        default:
          It.skipChildren();
          break;
        }
      }
    }
  }

  return false;
}

// Utility to check if TID call matches the below pattern -
// %tid = call i64 get_global_id(i32 0)
// %cmp = icmp ult i64 %tid, INT32_MAX+1
// call void @llvm.assume(i1 %cmp)
// Assume and TID call are expected to be in the same BB.
static bool TIDFitsInInt32(const CallInst *CI) {
  for (auto *User : CI->users()) {
    auto *CmpUser = dyn_cast<ICmpInst>(User);
    if (!CmpUser)
      continue;
    CmpInst::Predicate Pred;
    uint64_t UB = INT32_MAX + 1ULL;
    using namespace llvm::PatternMatch;
    if (match(CmpUser,
              m_OneUse(m_ICmp(Pred, m_Specific(CI), m_SpecificInt(UB)))) &&
        Pred == ICmpInst::ICMP_ULT) {
      auto *SingleUsr = *(CmpUser->user_begin());
      if (match(SingleUsr, m_Intrinsic<Intrinsic::assume>(m_Specific(CmpUser))))
        if (cast<IntrinsicInst>(SingleUsr)->getParent() == CI->getParent())
          return true;
    }
  }
  // Pattern match failed.
  return false;
}

// Check if we can optimize get_sub_group_local_id.
// We know this value is capped by a reasonable max VF
// (value not even close to 2GB, like 8, 16, 32, 64).
// We can use sext instead of zext to widen i32 type to i64.
// This would help vectorizer to not think unsigned 32 bit wrap around
// is possible in this sequence:
// %1 = tail call i32 @_Z22get_sub_group_local_idv()
// %conv.i.i = zext i32 %1 to i64
static bool isOptimizableSubgroupLocalId(const CallInst *CI) {
  using namespace llvm::PatternMatch;
  for (auto *User : CI->users()) {
    if (match(User, m_ZExt(m_Specific(CI))))
      return true;
  }
  return false;
}

DPCPPKernelVecCloneImpl::DPCPPKernelVecCloneImpl(ArrayRef<VectItem> VectInfos,
                                                 VFISAKind ISA,
                                                 bool IsOCL)
    : VecCloneImpl(), VectInfos(VectInfos), ISA(ISA), IsOCL(IsOCL) {
  if (IsaEncodingOverride.getNumOccurrences())
    this->ISA = IsaEncodingOverride.getValue();
}

void DPCPPKernelVecCloneImpl::handleLanguageSpecifics(Function &F, PHINode *Phi,
                                                      Function *Clone,
                                                      BasicBlock *EntryBlock,
                                                      const VFInfo &Variant) {
  // The FunctionsAndActions array has only the Kernel function built-ins that
  // are uniform.
  std::pair<std::string, FnAction> FunctionsAndActions[] = {
      {mangledGetGID(), FnAction::MoveAndUpdateUsesForDim},
      {mangledGetLID(), FnAction::MoveAndUpdateUsesForDim},
      {mangledGetSubGroupLocalId(), FnAction::UpdateOnly},
      {mangledGetGlobalSize(), FnAction::MoveOnly},
      {mangledGetGlobalOffset(), FnAction::MoveOnly},
      {mangledGetGroupID(), FnAction::MoveOnly},
      {mangledGetSubGroupSize(), FnAction::MoveOnly},
      {mangledGetLocalSize(), FnAction::MoveOnly},
      {mangledGetEnqueuedLocalSize(), FnAction::MoveOnly},
      {mangledGetGlobalLinearId(), FnAction::AssertIfEncountered},
      {mangledGetLocalLinearId(), FnAction::AssertIfEncountered}};

  bool IsKernel = find(Kernels, &F) != Kernels.end();

  unsigned VecDim = 0;
  bool CanUniteWorkgroups = false;
  if (VDMap && IsKernel) {
    const auto It = VDMap->find(&F);
    assert(It != VDMap->end() && "VectorizeDimInfo is not found");
    VecDim = It->second.getVectorizeDim();
    CanUniteWorkgroups = It->second.getCanUniteWorkGroups();
  }

  // Collect all Kernel function built-ins.
  SmallVector<Instruction *, 4> InstsToRemove;
  for (const auto &Pair : FunctionsAndActions) {
    const auto &FuncName = Pair.first;
    auto Action = Pair.second;

    // Early exit if the function is not present.
    Function *Func = Clone->getParent()->getFunction(FuncName);
    if (!Func)
      continue;

    for (User *U : Func->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      assert(CI && "Unexpected use of built-in function");
      if (CI->getFunction() != Clone)
        continue;

      assert((Action >= FnAction::MoveAndUpdateUses &&
              Action <= FnAction::UpdateOnly) &&
             "Unexpected Action");

      switch (Action) {
      case FnAction::MoveAndUpdateUsesForDim: {
        ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
        assert(C && "The function argument must be constant");
        unsigned dim = C->getValue().getZExtValue();
        if (dim == VecDim) {
          // If the get-id calls return i32 (e.g., on 32-bit target), there's
          // no truncation, so we don't need to do special optimization.
          bool TIDIsInt32 = CI->getType()->isIntegerTy(32);
          if (!TIDIsInt32 &&
              ((FuncName == mangledGetLID() && LT2GigWorkGroupSize) ||
               (FuncName == mangledGetGID() &&
                ((LT2GigGlobalWorkSize == GWS_TRUE) ||
                 (LT2GigGlobalWorkSize == GWS_AUTO &&
                  (IsOCL || (TIDFitsInInt32(CI) && HasTruncOrShlUsers(CI))))))))
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
      case FnAction::UpdateOnly: {
        bool TIDIsInt32 = CI->getType()->isIntegerTy(32);
        if (TIDIsInt32 && isOptimizableSubgroupLocalId(CI))
          optimizedUpdateTID(CI, Phi);
        else
          updateTID(CI, Phi);
        InstsToRemove.push_back(CI);
        break;
      }
      case FnAction::MoveOnly:
        // All the other Kernel function built-ins, if they have constant
        // arguments or don't have argument, then should just be moved at
        // the entry block.
        if (CI->arg_empty() || isa<Constant>(CI->getArgOperand(0)))
          CI->moveBefore(EntryBlock->getTerminator());
        break;
      case FnAction::AssertIfEncountered:
        assert(
            Func && FuncName != mangledGetGlobalLinearId() &&
            FuncName != mangledGetLocalLinearId() &&
            "get_global_linear_id() and get_local_linear_id() should have been "
            "resolved in earlier passes");
        break;
      };
    }
  }

  for (auto *I : InstsToRemove)
    I->eraseFromParent();

  const unsigned VF = Variant.getVF();

  if (IsKernel)
    updateKernelMetadata(F, Clone, VecDim, CanUniteWorkgroups);
  else
    Clone->addFnAttr("widened-size", std::to_string(VF));

  // Load all vector info into ExtendedVectInfos, at most once.
  static llvm::once_flag InitializeVectInfoFlag;
  llvm::call_once(InitializeVectInfoFlag, [&]() {
    initializeVectInfoOnce(VectInfos, ExtendedVectInfos);
  });

  for (auto &Inst : instructions(Clone)) {
    auto *Call = dyn_cast<CallInst>(&Inst);
    if (!Call)
      continue;

    Function *CalledFunc = Call->getCalledFunction();
    if (!CalledFunc)
      continue;

    auto FnName = CalledFunc->getName();

    // May be more than one entry, e.g. mask/unmasked (although currently that's
    // not the case).
    auto MatchingVariants = make_filter_range(
        ExtendedVectInfos,
        [FnName,
         VF](const std::tuple<std::string, std::string, std::string> &Info)
            -> bool {
          return std::get<0>(Info) == FnName &&
                 VFABI::demangleForVFABI(std::get<2>(Info)).getVF() == VF;
        });

    if (MatchingVariants.begin() == MatchingVariants.end())
      continue;

    std::string Variants;
    assert(!Call->hasFnAttr("vector-variants") &&
           "Unexpected vector-variants attribute for OpenCL builtin!");

    // This condition isn't expected to happen, but do the right thing anyway.
    if (Call->hasFnAttr("vector-variants"))
      Variants = std::string(
          Call->getCallSiteOrFuncAttr("vector-variants").getValueAsString());

    // Indicates the call must have mask arg.
    bool HasMask = true;
    // Indicates the call must not mask arg.
    bool NotHasMask = true;
    // Indicates the call must have kernel-call-once attribute
    bool KernelCallOnce = true;
    for (auto &Variant : MatchingVariants) {
      if (!Variants.empty())
        Variants += ',';

      Variants += std::get<2>(Variant);
      if (VFABI::demangleForVFABI(std::get<2>(Variant)).isMasked())
        NotHasMask = false;
      else
        HasMask = false;
      if (std::get<1>(Variant) != KernelAttribute::CallOnce)
        KernelCallOnce = false;
    }

    // On avx1/sse42, only builtins with "kernel-call-once" attribute have VF 32
    // and 64 implementations.
    if ((ISA == VFISAKind::SSE || ISA == VFISAKind::AVX) && VF >= 32 &&
        !KernelCallOnce)
      continue;

    AttributeList AL = Call->getAttributes();

    AL = AL.addFnAttribute(Call->getContext(), "vector-variants", Variants);
    // TODO: So far the functions that have their vector variants assigned here
    // are essentially "kernel-call-once" functions.
    if (KernelCallOnce)
      AL = AL.addFnAttribute(Call->getContext(), KernelAttribute::CallOnce);
    if (HasMask)
      AL = AL.addFnAttribute(Call->getContext(), KernelAttribute::HasVPlanMask);
    else if (!NotHasMask) {
      unsigned ParamsNum = Call->arg_size();
      AL = AL.addFnAttribute(Call->getContext(), KernelAttribute::CallParamNum,
                           std::to_string(ParamsNum));
    }
    Call->setAttributes(AL);
  }
}

using ReturnInfoTy = std::vector<std::pair<std::string, VFParamKind>>;

static ReturnInfoTy PopulateOCLBuiltinReturnInfo() {
  // sub_group_get_local_id isn't here due to special processing in VecClone
  // pass. Void-argument functions returning uniform values are implicitly
  // known as uniform too.
  ReturnInfoTy RetInfo;

  // Work group uniform built-ins
  RetInfo.push_back({"_Z14work_group_alli", VFParamKind::OMP_Uniform});
  RetInfo.push_back({"_Z14work_group_anyi", VFParamKind::OMP_Uniform});
  const char WorkGroupTypes[] = {'i', 'j', 'l', 'm', 'f', 'd'};
  for (char Type : WorkGroupTypes) {
    RetInfo.push_back({std::string("_Z20work_group_broadcast") + Type + "m",
                       VFParamKind::OMP_Uniform});
    RetInfo.push_back({std::string("_Z20work_group_broadcast") + Type + "mm",
                       VFParamKind::OMP_Uniform});
    RetInfo.push_back({std::string("_Z20work_group_broadcast") + Type + "mmm",
                       VFParamKind::OMP_Uniform});

    for (auto Op : {"add", "min", "max", "mul"})
      RetInfo.push_back({std::string("_Z21work_group_reduce_") + Op + Type,
                         VFParamKind::OMP_Uniform});
  }
  const char WorkGroupIntegerTypes[] = {'c', 'h', 's', 't', 'i', 'j', 'l', 'm'};
  for (char Type : WorkGroupIntegerTypes) {
    for (const auto *Op : {"bitwise_and", "bitwise_xor"})
      RetInfo.push_back({std::string("_Z29work_group_reduce_") + Op + Type,
                        VFParamKind::OMP_Uniform});
    RetInfo.push_back({std::string("_Z28work_group_reduce_bitwise_or") + Type,
                  VFParamKind::OMP_Uniform});
  }
  RetInfo.push_back({std::string("_Z29work_group_reduce_logical_andi"),
                    VFParamKind::OMP_Uniform});
  RetInfo.push_back({std::string("_Z28work_group_reduce_logical_ori"),
                    VFParamKind::OMP_Uniform});
  RetInfo.push_back({std::string("_Z29work_group_reduce_logical_xori"),
                    VFParamKind::OMP_Uniform});

  // Sub group uniform built-ins
  RetInfo.push_back({std::string("_Z13sub_group_alli"), VFParamKind::OMP_Uniform});
  RetInfo.push_back({std::string("_Z13sub_group_anyi"), VFParamKind::OMP_Uniform});

  RetInfo.push_back(
      {std::string("_Z22intel_sub_group_balloti"), VFParamKind::OMP_Uniform});

  const char SubGroupTypes[] = {'i', 'j', 'l', 'm', 'f', 'd'};
  for (char Type : SubGroupTypes) {
    RetInfo.push_back({std::string("_Z19sub_group_broadcast") + Type + 'j',
                       VFParamKind::OMP_Uniform});
    for (auto Op : {"add", "min", "max"})
      RetInfo.push_back({std::string("_Z20sub_group_reduce_") + Op + Type,
                         VFParamKind::OMP_Uniform});
  }

  const char IntelSubGroupTypes[] = {'c', 'h', 's', 't'};
  for (char Type : IntelSubGroupTypes) {
    RetInfo.push_back(
        {std::string("_Z25intel_sub_group_broadcast") + Type + 'j',
         VFParamKind::OMP_Uniform});
    for (auto Op : {"add", "min", "max"})
      RetInfo.push_back({std::string("_Z26intel_sub_group_reduce_") + Op + Type,
                         VFParamKind::OMP_Uniform});
  }

  // Pipe functions
  RetInfo.push_back(
      {std::string("__work_group_reserve_write_pipe"), VFParamKind::OMP_Uniform});
  RetInfo.push_back(
      {std::string("__work_group_reserve_read_pipe"), VFParamKind::OMP_Uniform});

  return RetInfo;
}

void DPCPPKernelVecCloneImpl::languageSpecificInitializations(Module &M) {
  // FIXME: Longer term plan is to make the return value propery part of
  // VFInfo encoding
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

    assert(Entry.second == VFParamKind::OMP_Uniform && "Only uniforms are supported by now!");
    Fn->addFnAttr("opencl-vec-uniform-return");
  }

  // Process async_work_group copies separately as it is easier to detect
  // them via unmangling as the number of overloads is high.
  for (auto &F : M) {
    if (!F.isDeclaration())
      continue;
    if (isAsyncWorkGroupCopy(F.getName()) ||
        isAsyncWorkGroupStridedCopy(F.getName()))
      F.addFnAttr("opencl-vec-uniform-return");
  }

  // Mark "kernel-uniform-call" (see LangRef for more details).
  FuncSet SyncBuiltins = getAllSyncBuiltinsDeclsForKernelUniformCallAttr(M);
  // process call sites
  for (auto *F : SyncBuiltins) {
    for (auto *U : F->users())
      if (auto *CI = dyn_cast<CallInst>(U))
        CI->setAttributes(CI->getAttributes().addFnAttribute(
            CI->getContext(), "kernel-uniform-call"));
  }

  Kernels = getKernels(M).getList();

  if (Kernels.empty()) {
    LLVM_DEBUG(dbgs() << lv_name << ":"
                      << "No kernels found!\n");
    return;
  }

  DPCPPPrepareKernelForVecClone PK(ISA);
  for (auto *F : Kernels) {
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    if (KIMD.RecommendedVL.get() > 1)
      PK.run(*F);
  }
}
