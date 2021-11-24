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
#include "OCLVecClone.h"

#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "NameMangleAPI.h"
#include "VectorizerCommon.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Threading.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPPrepareKernelForVecClone.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include <mutex>
#include <string>

#define DEBUG_TYPE "OCLVecClone"
#define SV_NAME "ocl-vecclone"
#define SV_NAME1 "ocl-reqd-sub-group-size"

#define SG_BLOCK_READ_PREFIX "intel_sub_group_block_read"
#define SG_BLOCK_WRITE_PREFIX "intel_sub_group_block_write"
#define KERNEL_CALL_ONCE "kernel-call-once"
#define PRIM_TYPE(prim_type_enum)                                              \
  (new reflection::PrimitiveType(prim_type_enum))
#define PRIM_POINTER_TYPE(pointee_type, attrs)                                 \
  (new reflection::PointerType(                                                \
      PRIM_TYPE(pointee_type),                                                 \
      std::vector<reflection::TypeAttributeEnum> attrs))
#define CONST_GLOBAL_PTR(pointee_type)                                         \
  PRIM_POINTER_TYPE(pointee_type,                                              \
                    ({reflection::ATTR_GLOBAL, reflection::ATTR_CONST}))
#define GLOBAL_PTR(pointee_type)                                               \
  PRIM_POINTER_TYPE(pointee_type, ({reflection::ATTR_GLOBAL}))
#define VECTOR_TYPE(element_type, len)                                         \
  (new reflection::VectorType(element_type, len))
#define INT2_TYPE VECTOR_TYPE(PRIM_TYPE(reflection::PRIMITIVE_INT), 2)

using namespace llvm;
using namespace llvm::NameMangleAPI;
using namespace DPCPPKernelMetadataAPI;
using namespace PatternMatch;

// Static container storing all the vector info entries.
// Each entry would be a tuple of three strings:
// 1. scalar variant name
// 2. "kernel-call-once" | ""
// 3. mangled vector variant name
static std::vector<std::tuple<std::string, std::string, std::string>> g_VecInfo;
// Guardian flag to make sure g_VecInfo is initialized only once.
static llvm::once_flag initVecInfoOnce;

static cl::opt<std::string> ReqdSubGroupSizes("reqd-sub-group-size", cl::init(""),
                                        cl::Hidden,
                                        cl::desc("Per-kernel required subgroup"
                                                 "size. Comma separated list of"
                                                 " name(num)"));

static cl::opt<bool> LT2GigWorkGroupSize(
    "less-than-two-gig-max-work-group-size", cl::init(true), cl::Hidden,
    cl::desc("Max work group size is less than 2 Gig elements."));

enum GlobalWorkSizeLT2GState : uint8_t {GWS_FALSE, GWS_TRUE, GWS_AUTO};
static cl::opt<GlobalWorkSizeLT2GState> LT2GigGlobalWorkSize(
    "less-than-two-gig-max-global-work-size", cl::init(GWS_AUTO), cl::Hidden,
    cl::desc("Max global work size (global_work_offset + total work items) is "
             "less than 2 Gig elements."),
    cl::values(clEnumValN(GWS_AUTO, "auto", ""),
               clEnumValN(GWS_TRUE, "true", ""),
               clEnumValN(GWS_FALSE, "false", "")));

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

using ReturnInfoTy = std::vector<std::pair<std::string, VectorKind>>;
static ReturnInfoTy PopulateOCLBuiltinReturnInfo();

char OCLVecClone::ID = 0;
static const char lv_name[] = "OCLVecClone";
OCL_INITIALIZE_PASS_BEGIN(OCLVecClone, SV_NAME, lv_name,
                          false /* modifies CFG */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLVecClone, SV_NAME, lv_name,
                        false /* modififies CFG */, false /* transform pass */)

OCLVecClone::OCLVecClone(const Intel::OpenCL::Utils::CPUDetect *CPUId,
                         bool IsOCL)
    : ModulePass(ID), Impl(CPUId, IsOCL) {
  initializeVecClonePass(*PassRegistry::getPassRegistry());
}

OCLVecClone::OCLVecClone() : OCLVecClone(nullptr, false) {}

bool OCLVecClone::runOnModule(Module &M) {
  Impl.setDimChooser(getAnalysisIfAvailable<ChooseVectorizationDimensionModulePass>());
  return Impl.runImpl(M);
}

OCLVecCloneImpl::OCLVecCloneImpl(const Intel::OpenCL::Utils::CPUDetect *CPUId,
                                 bool IsOCL)
    : VecCloneImpl(), CPUId(CPUId), IsOCL(IsOCL) {
  V_INIT_PRINT;
}

OCLVecCloneImpl::OCLVecCloneImpl() : VecCloneImpl() {}

// Remove the "recommended_vector_length" metadata from the original kernel.
// "recommened_vector_length" metadata is used only by OCLVecClone. The rest
// of the Volcano passes recognize the "vector_width" metadata. Thus, we add
// "vector_width" metadata to the original kernel and the cloned kernel.
static void updateKernelMetadata(Function &F, Function *Clone,
                           unsigned VecDim, bool CanUniteWorkgroups) {
  auto FMD = KernelInternalMetadataAPI(&F);
  auto KMD = KernelMetadataAPI(&F);
  auto CloneMD = KernelInternalMetadataAPI(Clone);
  // Get VL from the metadata from the original kernel.
  unsigned VectorLength = FMD.RecommendedVL.get();
  // Set the "vector_width" metadata to the cloned kernel.
  CloneMD.VectorizedKernel.set(nullptr);
  CloneMD.VectorizedWidth.set(VectorLength);
  CloneMD.VectorizationDimension.set(VecDim);
  // Set the metadata that points to the orginal kernel of the clone.
  CloneMD.ScalarKernel.set(&F);
  CloneMD.CanUniteWorkgroups.set(CanUniteWorkgroups);

  // Set "vector_width" for the original kernel.
  FMD.VectorizedWidth.set(1);
  FMD.ScalarKernel.set(nullptr);

  if (F.getFunctionType() == Clone->getFunctionType())
    FMD.VectorizedKernel.set(Clone);
  else // Vectorized kernel with mask
    FMD.VectorizedMaskedKernel.set(Clone);
}

// Updates all the uses of TID calls with TID + new induction variable.
static void updateAndMoveTID(Instruction *TIDCallInstr, PHINode *Phi,
                             BasicBlock *EntryBlock) {
  Instruction *IP = &*Phi->getParent()->getFirstInsertionPt();
  IRBuilder<> IRB(IP);
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

static reflection::TypeVector
widenParameters(reflection::TypeVector scalar_params, unsigned int vf) {
  reflection::TypeVector vector_params;
  for (auto param : scalar_params) {
    if (auto *vec_param =
            reflection::dyn_cast<reflection::VectorType>(param.get())) {
      int widen_len = vec_param->getLength() * vf;
      vector_params.emplace_back(
          VECTOR_TYPE(vec_param->getScalarType(), widen_len));
    } else {
      vector_params.emplace_back(VECTOR_TYPE(param, vf));
    }
  }

  return vector_params;
}

static void
pushSGBlockBuiltinDivergentVectInfo(const Twine &basename, unsigned int len,
                                    unsigned int vf,
                                    reflection::TypeVector scalar_params) {
  // Get mangled name of scalar variant
  reflection::FunctionDescriptor scalar_func{
      (basename + (len == 1 ? "" : Twine(len))).str(), scalar_params,
      reflection::width::SCALAR};
  std::string scalar_mangle_name = mangle(scalar_func);

  // Get mangled name of vector variant
  reflection::TypeVector vector_params = widenParameters(scalar_params, vf);
  size_t v_num = vector_params.size();
  // Add mask param
  auto *mask = VECTOR_TYPE(PRIM_TYPE(reflection::PRIMITIVE_UINT), vf);
  vector_params.push_back(mask);
  reflection::FunctionDescriptor vector_func{
      (basename + Twine(len) + "_" + Twine(vf)).str(), vector_params,
      reflection::width::NONE};
  std::string vector_mangle_name = mangle(vector_func);

  // Get vector variant string repr
  VectorVariant variant{VectorVariant::ISAClass::XMM,
                        true,
                        vf,
                        std::vector<VectorKind>(v_num, VectorKind::vector()),
                        scalar_mangle_name,
                        vector_mangle_name};
  std::string variant_repr = variant.toString();

  g_VecInfo.push_back({scalar_mangle_name, KERNEL_CALL_ONCE, variant_repr});
}

static void pushSGBlockBuiltinDivergentVectInfo(
    StringRef type_suffix, reflection::TypePrimitiveEnum type,
    std::vector<unsigned int> lens, std::vector<unsigned int> vfs) {
  for (unsigned int len : lens) {
    for (unsigned int vf : vfs) {
      // sub_group_block_read(const __global T*)
      pushSGBlockBuiltinDivergentVectInfo(SG_BLOCK_READ_PREFIX + type_suffix,
                                          len, vf, {CONST_GLOBAL_PTR(type)});
      // sub_group_block_read(readonly image2d_t, int2)
      pushSGBlockBuiltinDivergentVectInfo(
          SG_BLOCK_READ_PREFIX + type_suffix, len, vf,
          {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RO_T), INT2_TYPE});
      // sub_group_block_read(readwrite image2d_t, int2)
      pushSGBlockBuiltinDivergentVectInfo(
          SG_BLOCK_READ_PREFIX + type_suffix, len, vf,
          {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE});

      if (len == 1) {
        // sub_group_block_write(__global T*, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + type_suffix, len, vf,
            {GLOBAL_PTR(type), PRIM_TYPE(type)});
        // sub_group_block_write(writeonly image2d_t, int2, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + type_suffix, len, vf,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_WO_T), INT2_TYPE,
             PRIM_TYPE(type)});
        // sub_group_block_write(readwrite image2d_t, int2, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + type_suffix, len, vf,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE,
             PRIM_TYPE(type)});
      } else {
        // sub_group_block_write(__global T*, T<len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + type_suffix, len, vf,
            {GLOBAL_PTR(type), VECTOR_TYPE(PRIM_TYPE(type), len)});
        // sub_group_block_write(writeonly image2d_t, int2, T<len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + type_suffix, len, vf,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_WO_T), INT2_TYPE,
             VECTOR_TYPE(PRIM_TYPE(type), len)});
        // sub_group_block_write(readwrite image2d_t, int2, T<len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + type_suffix, len, vf,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE,
             VECTOR_TYPE(PRIM_TYPE(type), len)});
      }
    }
  }
}

static void initializeVectInfo() {
  // Load Table-Gen'erated VectInfo.gen
  const static std::tuple<const char *, const char *, const char *> infos[] = {
    #include "VectInfo.gen"
  };
  g_VecInfo.insert(g_VecInfo.end(), std::begin(infos), std::end(infos));

  // Add extra vector info for 'sub_group_ballot'
  g_VecInfo.push_back({"intel_sub_group_ballot", KERNEL_CALL_ONCE,
                       "_ZGVbM4v_intel_sub_group_balloti(intel_sub_group_ballot_vf4)"});
  g_VecInfo.push_back({"intel_sub_group_ballot", KERNEL_CALL_ONCE,
                       "_ZGVbM8v_intel_sub_group_balloti(intel_sub_group_ballot_vf8)"});
  g_VecInfo.push_back({"intel_sub_group_ballot", KERNEL_CALL_ONCE,
                       "_ZGVbM16v_intel_sub_group_balloti(intel_sub_group_ballot_vf16)"});

  // Add extra vector info for 'sub_group_block_read*', 'sub_group_block_write*'
  std::vector<std::tuple<std::string, reflection::TypePrimitiveEnum,
                         std::vector<unsigned int>, std::vector<unsigned int>>>
      entries{
          {"",
           reflection::PRIMITIVE_UINT,
           {1, 2, 4, 8},
           {4, 8, 16, 32, 64}},
          {"_uc",
           reflection::PRIMITIVE_UCHAR,
           {1, 2, 4, 8, 16},
           {4, 8, 16, 32, 64}},
          {"_us",
           reflection::PRIMITIVE_USHORT,
           {1, 2, 4, 8},
           {4, 8, 16, 32, 64}},
          {"_ui",
           reflection::PRIMITIVE_UINT,
           {1, 2, 4, 8},
           {4, 8, 16, 32, 64}},
          {"_ul",
           reflection::PRIMITIVE_ULONG,
           {1, 2, 4, 8},
           {4, 8, 16, 32, 64}},
      };
  for (auto &entry : entries) {
    pushSGBlockBuiltinDivergentVectInfo(std::get<0>(entry), std::get<1>(entry),
                                        std::get<2>(entry), std::get<3>(entry));
  }
}

void OCLVecCloneImpl::handleLanguageSpecifics(Function &F, PHINode *Phi,
                                              Function *Clone,
                                              BasicBlock *EntryBlock,
                                              const VectorVariant &Variant) {
  // The FunctionsAndActions array has only the OpenCL function built-ins that
  // are uniform.
  std::pair<std::string, FnAction> FunctionsAndActions[] = {
      std::make_pair(CompilationUtils::mangledGetGID(),
                     FnAction::MoveAndUpdateUsesForDim),
      std::make_pair(CompilationUtils::mangledGetLID(),
                     FnAction::MoveAndUpdateUsesForDim),
      std::make_pair(CompilationUtils::mangledGetSubGroupLocalId(),
                     FnAction::UpdateOnly),
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

  // TODO: this operation can be expensive.
  auto Kernels = KernelList(F.getParent()).getList();
  std::set<Function *> KernelsSet(Kernels.begin(), Kernels.end());

  bool IsKernel = KernelsSet.count(&F);

  unsigned VecDim;
  bool CanUniteWorkgroups;
  if (DimChooser && IsKernel) {
    VecDim = DimChooser->getVectorizationDim(&F);
    CanUniteWorkgroups = DimChooser->getCanUniteWorkgroups(&F);
  } else {
    VecDim = 0;
    CanUniteWorkgroups = false;
  }

  SmallVector<Instruction *, 4> InstsToRemove;
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
      Function *parentFunc = CI->getFunction();
      if (parentFunc != Clone)
        continue;
      assert((Action >= FnAction::MoveAndUpdateUses &&
              Action <= FnAction::UpdateOnly) &&
             "Unexpected Action.");

      switch (Action) {
      case FnAction::MoveAndUpdateUsesForDim: {
        ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
        assert(C && "The function argument must be constant");
        unsigned dim = C->getValue().getZExtValue();
        if (dim == VecDim) {
          // If the get-id calls return i32 (e.g., on 32-bit target), there's
          // no truncation, so we don't need to do special optimization.
          // LT2GigGlobalWorkSize is set to AUTO by default, then for native
          // OpenCL program, the optimization is turned on to match the
          // behavior of Volcano, and for SYCL, it's turned on only when the
          // assumption that TID fits in i32 stands.
          bool TIDIsInt32 = CI->getType()->isIntegerTy(32);
          if (!TIDIsInt32 &&
              ((FuncName == CompilationUtils::mangledGetLID() &&
                LT2GigWorkGroupSize) ||
               (FuncName == CompilationUtils::mangledGetGID() &&
                ((LT2GigGlobalWorkSize == GWS_TRUE) ||
                 (LT2GigGlobalWorkSize == GWS_AUTO &&
                  (IsOCL || TIDFitsInInt32(CI)))))))
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
      case FnAction::UpdateOnly:
        updateTID(CI, Phi);
        InstsToRemove.push_back(CI);
        break;
      case FnAction::MoveOnly:
        // All the other OpenCL function built-ins, if they have constant
        // arguments or don't have argument, then should just be moved at
        // the entry block.
        if (CI->arg_empty() || isa<Constant>(CI->getArgOperand(0)))
          CI->moveBefore(EntryBlock->getTerminator());
        break;
      case FnAction::AssertIfEncountered:
        assert(
            Func && FuncName != CompilationUtils::mangledGetGlobalLinearId() &&
            FuncName != CompilationUtils::mangledGetLocalLinearId() &&
            "get_global_linear_id() and get_local_linear_id() should have been "
            "resolved in earlier passes");
        break;
      }
    }
  }

  for (auto *I : InstsToRemove)
    I->eraseFromParent();

  unsigned VF = Variant.getVlen();

  if (IsKernel)
    updateKernelMetadata(F, Clone, VecDim, CanUniteWorkgroups);
  else
    Clone->addFnAttr("widened-size", std::to_string(VF));

  // Load all vector info into g_VecInfo, at most once.
  llvm::call_once(initVecInfoOnce, initializeVectInfo);

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
        g_VecInfo,
        [FnName,
         VF](const std::tuple<std::string, std::string, std::string> &Info)
            -> bool {
          return std::get<0>(Info) == FnName &&
                 VectorVariant(std::get<2>(Info)).getVlen() == VF;
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
      if (VectorVariant(std::get<2>(Variant)).isMasked())
        NotHasMask = false;
      else
        HasMask = false;
      if (std::get<1>(Variant) != KERNEL_CALL_ONCE)
        KernelCallOnce = false;
    }

    AttributeList AL = Call->getAttributes();

    AL = AL.addFnAttribute(Call->getContext(), "vector-variants", Variants);
    // TODO: So far the functions that have their vector variants assigned here
    // are essentially "kernel-call-once" functions.
    if (KernelCallOnce) {
      AL = AL.addFnAttribute(Call->getContext(),
                             CompilationUtils::ATTR_KERNEL_CALL_ONCE);
    }
    if (HasMask)
      AL = AL.addFnAttribute(Call->getContext(),
                             CompilationUtils::ATTR_HAS_VPLAN_MASK);
    else if (!NotHasMask) {
      unsigned ParamsNum = Call->arg_size();
      AL = AL.addFnAttribute(Call->getContext(), "call-params-num",
                             std::to_string(ParamsNum));
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
  DPCPPPrepareKernelForVecClone PK(Intel::VectorizerCommon::getCPUIdISA(CPUId));

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
        CI->setAttributes(CI->getAttributes().addFnAttribute(
            CI->getContext(), "kernel-uniform-call"));
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
    unsigned VectorLength = FMD.RecommendedVL.get();
    if ((VectorLength > 1) && (!F->hasOptNone()))
      PK.run(*F);
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

extern "C" Pass *
createOCLVecClonePass(const Intel::OpenCL::Utils::CPUDetect *CPUId,
                      bool IsOCL) {
  return new intel::OCLVecClone(CPUId, IsOCL);
}

extern "C" Pass *createOCLReqdSubGroupSizePass() {
  return new intel::OCLReqdSubGroupSize();
}
