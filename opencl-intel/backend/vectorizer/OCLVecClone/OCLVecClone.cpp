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
#include "MetadataAPI.h"
#include "OCLPrepareKernelForVecClone.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"

#define DEBUG_TYPE "OCLVecClone"
#define SV_NAME "ocl-vecclone"

using namespace llvm;
using namespace Intel::MetadataAPI;

namespace intel {

using ContainerTy = std::vector<std::pair<std::string, VectorVariant>>;
static ContainerTy OCLBuiltinVecInfo();

char OCLVecClone::ID = 0;
static const char lv_name[] = "OCLVecClone";
OCL_INITIALIZE_PASS_BEGIN(OCLVecClone, SV_NAME, lv_name,
                          false /* modifies CFG */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLVecClone, SV_NAME, lv_name,
                        false /* modififies CFG */, false /* transform pass */)

OCLVecClone::OCLVecClone(const Intel::CPUId *CPUId,
                         bool EnableVPlanVecForOpenCL)
    : VecClone(), CPUId(CPUId),
      EnableVPlanVecForOpenCL(EnableVPlanVecForOpenCL) {
  V_INIT_PRINT;
}

OCLVecClone::OCLVecClone() : VecClone(), EnableVPlanVecForOpenCL(true) {}

// Remove the "ocl_recommended_vector_length" metadata from the original kernel.
// "ocl_recommened_vector_length" metadata is used only by OCLVecClone. The rest
// of the Volcano passes recognize the "vector_width" metadata. Thus, we add
// "vector_width" metadata to the original kernel and the cloned kernel.
static void updateMetadata(Function &F, Function *Clone) {
  auto FMD = KernelInternalMetadataAPI(&F);
  auto KMD = KernelMetadataAPI(&F);
  auto CloneMD = KernelInternalMetadataAPI(Clone);
  // Get VL from the metadata from the original kernel.
  unsigned VectorLength;
  if (KMD.hasVecLength()) // check for forced vector length
    VectorLength = KMD.getVecLength();
  else
    VectorLength = FMD.OclRecommendedVectorLength.get();
  // Set the "vector_width" metadata to the cloned kernel.
  CloneMD.VectorizedKernel.set(nullptr);
  CloneMD.VectorizedWidth.set(VectorLength);
  // For now, only x dimension is vectorized. For this reason, the chosen
  // vectorization dimension is 0.
  CloneMD.VectorizationDimension.set(0);
  // Set the metadata that points to the orginal kernel of the clone.
  CloneMD.ScalarizedKernel.set(&F);
  // TODO: for now, it is false.
  CloneMD.CanUniteWorkgroups.set(false);

  // Set "vector_width" for the original kernel.
  FMD.VectorizedWidth.set(1);
  FMD.ScalarizedKernel.set(nullptr);
  FMD.VectorizedKernel.set(Clone);
  // Remove "ocl_recommended_vector_length" metadata
  MDValueGlobalObjectStrategy::unset(&F, "ocl_recommended_vector_length");
}

// Updates all the uses of TID calls with TID + new induction variable.
static void updateTID(IVecVec &TIDCalls, PHINode *Phi, BasicBlock *EntryBlock) {
  IRBuilder<> IRB(Phi);
  IRB.SetInsertPoint(Phi->getNextNode());
  // Currently, only zero dimension is vectorized.
  // Once that is fixed, don't forget to address the FIXME in the
  // collectIDCallInst regarding zero-operand ID calls.
  IVec zeroDimTIDCalls = TIDCalls[0];
  for (IVec::iterator tidIt = zeroDimTIDCalls.begin(),
                      tidE = zeroDimTIDCalls.end();
       tidIt != tidE; ++tidIt) {
    Instruction *TIDCallInstr = *tidIt;
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
}

void OCLVecClone::handleLanguageSpecifics(Function &F, PHINode *Phi,
                                          Function *Clone,
                                          BasicBlock *EntryBlock) {
  // FIXME: Add *_linear_id.
  std::string MangledIDFuncs[] = {CompilationUtils::mangledGetGID(),
                                  CompilationUtils::mangledGetLID(),
                                  CompilationUtils::mangledGetSubGroupLID()};
  for (auto &MangledIDFunc : MangledIDFuncs) {
    IVecVec Calls;
    LoopUtils::collectTIDCallInst(MangledIDFunc.c_str(), Calls, Clone);
    updateTID(Calls, Phi, EntryBlock);
  }
  updateMetadata(F, Clone);

  static auto VecInfo = OCLBuiltinVecInfo();
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
      Variants = Call->getFnAttr("vector-variants").getValueAsString();

    for (auto &Variant : MatchingVariants) {
      if (!Variants.empty())
        Variants += ',';

      Variants +=  Variant.second.toString();
    }

    AttributeList AL = Call->getAttributes();

    AL = AL.addAttribute(Call->getContext(), AttributeList::FunctionIndex,
                         "vector-variants", Variants);

    Call->setAttributes(AL);
  }
}

// TODO: Replace with tblgen-generated data. Currently we probably duplicate
// some code for the mangling part, but that should go away once we start using
// tblgen.
//
// The code below populates table with mapping from "scalar" OpenCL builin to
// its vector-variants information that is used to annotate builtins' call-sites
// to let VPlan vectorizer know how to process this particular builtin for a
// given vectorized version of the kernel.
namespace {
struct TypeInfo {
  char TypeLetter;
  unsigned VF; // 1 means scalar.
  TypeInfo(char TypeLetter, unsigned VF) : TypeLetter(TypeLetter), VF(VF) {}
  TypeInfo(char TypeLetter) : TypeInfo(TypeLetter, 1) {}
  TypeInfo() : TypeInfo('\0', 0) {}

  std::string str() const {
    if (VF == 1)
      return std::string(1, TypeLetter);

    return std::string("Dv") + std::to_string(VF) + "_" + TypeLetter;
  }

  TypeInfo multVF(unsigned VFMultiplier) {
    TypeInfo New = *this;
    New.VF *= VFMultiplier;
    return New;
  }

  bool operator==(const TypeInfo &Other) const {
    return TypeLetter == Other.TypeLetter && VF == Other.VF;
  }
};
} // namespace

//  The mangling part is probably duplicating existing code, but this is only a
// temporary solution anyway and this simpler interface is enough currently.
static std::string mangleTypes(std::vector<TypeInfo> Types) {
  SmallVector<TypeInfo, 2> SubstitutableTypes;

  std::string Result;

  for (TypeInfo &Type : Types) {
    if (Type.VF == 1) {
      Result += Type.str();
      continue;
    }

    auto It = llvm::find(SubstitutableTypes, Type);
    if (It == SubstitutableTypes.end()) {
      Result += Type.str();
      SubstitutableTypes.push_back(Type);
      continue;
    }

    int Idx = std::distance(SubstitutableTypes.begin(), It);
    switch (Idx) {
    case 0:
      Result += "S_";
      break;
    case 1:
      Result += "S0_";
      break;
    default:
      llvm_unreachable("Not implemented!");
    }
  }

  return Result;
}

static std::pair<std::string, VectorVariant>
getEntry(std::string ScalarBaseName, std::vector<TypeInfo> ScalarTypes,
         std::vector<VectorKind> Params, unsigned VL, bool Masked) {
  // Calculate the scalar name before we modify the types vector.
  std::string ScalarName = ScalarBaseName + mangleTypes(ScalarTypes);

  // Modify the vector inplace, but under different name to avoid reader's
  // confusion.
  auto &Types = ScalarTypes;

  if (Masked)
    Types.emplace_back('j', 1);

  // Calculate the type for the vector variant of the builtin, based on the
  // information about the parameter VectorKind (i.e., is it widened or remains
  // scalar).
  for (unsigned TypeIdx = 0; TypeIdx < Types.size(); ++TypeIdx) {
    TypeInfo Type = Types[TypeIdx];
    bool IsUniform = TypeIdx < Params.size() && Params[TypeIdx].isUniform();
    if (!IsUniform)
      Type = Type.multVF(VL);

    Types[TypeIdx] = Type;
  }
  std::string VectorName = ScalarBaseName + mangleTypes(Types);

  return {ScalarName,
          {VectorVariant::ISAClass::XMM /*does not matter*/, Masked, VL,
           std::move(Params),
           "", // Empty BaseName - does not matter.
           VectorName}};
}

// Create entries for the whole set of VFs (4, 8, 16)
static void addEntries(ContainerTy &Info, std::string ScalarBaseName,
                       std::vector<TypeInfo> Types,
                       std::vector<VectorKind> Params, bool Masked = false) {
  Info.push_back(getEntry(ScalarBaseName, Types, Params, 4 /* VL */, Masked));
  Info.push_back(getEntry(ScalarBaseName, Types, Params, 8 /* VL */, Masked));
  Info.push_back(getEntry(ScalarBaseName, Types, Params, 16 /* VL */, Masked));
}

// Handy overload for single-argument builtins.
static void addEntries(ContainerTy &Info, std::string ScalarBaseName,
                       TypeInfo Type, std::vector<VectorKind> Params,
                       bool Masked = false) {
  std::vector<TypeInfo> Types = {Type};
  addEntries(Info, ScalarBaseName, Types, std::move(Params), Masked);
}

static void addBlockReadWriteBuiltins(ContainerTy &Info) {
  // Scalar -> [VF4, VF8, VF16]
  using BuiltinInfo = std::pair<std::string, std::array<std::string, 3>>;

  BuiltinInfo ReadBuiltins[] = {
      {"_Z26intel_sub_group_block_readPU3AS1Kj",
       {"_Z29intel_sub_group_block_read1_4PU3AS1Kj",
        "_Z29intel_sub_group_block_read1_8PU3AS1Kj",
        "_Z30intel_sub_group_block_read1_16PU3AS1Kj"}},
      {"_Z27intel_sub_group_block_read2PU3AS1Kj",
       {"_Z29intel_sub_group_block_read2_4PU3AS1Kj",
        "_Z29intel_sub_group_block_read2_8PU3AS1Kj",
        "_Z30intel_sub_group_block_read2_16PU3AS1Kj"}},
      {"_Z27intel_sub_group_block_read4PU3AS1Kj",
       {"_Z29intel_sub_group_block_read4_4PU3AS1Kj",
        "_Z29intel_sub_group_block_read4_8PU3AS1Kj",
        "_Z30intel_sub_group_block_read4_16PU3AS1Kj"}},
      {"_Z27intel_sub_group_block_read8PU3AS1Kj",
       {"_Z29intel_sub_group_block_read8_4PU3AS1Kj",
        "_Z29intel_sub_group_block_read8_8PU3AS1Kj",
        "_Z30intel_sub_group_block_read8_16PU3AS1Kj"}},
  };
  BuiltinInfo WriteBuiltins[] = {
      {"_Z27intel_sub_group_block_writePU3AS1jj",
       {"_Z30intel_sub_group_block_write1_4PU3AS1jDv4_j",
        "_Z30intel_sub_group_block_write1_8PU3AS1jDv8_j",
        "_Z31intel_sub_group_block_write1_16PU3AS1jDv16_j"}},
      {"_Z28intel_sub_group_block_write2PU3AS1jDv2_j",
       {"_Z30intel_sub_group_block_write2_4PU3AS1jDv8_j",
        "_Z30intel_sub_group_block_write2_8PU3AS1jDv16_j",
        "_Z31intel_sub_group_block_write2_16PU3AS1jDv32_j"}},
      {"_Z28intel_sub_group_block_write4PU3AS1jDv4_j",
       {"_Z30intel_sub_group_block_write4_4PU3AS1jDv16_j",
        "_Z30intel_sub_group_block_write4_8PU3AS1jDv32_j",
        "_Z31intel_sub_group_block_write4_16PU3AS1jDv64_j"}},
      {"_Z28intel_sub_group_block_write8PU3AS1jDv8_j",
       {"_Z30intel_sub_group_block_write8_4PU3AS1jDv32_j",
        "_Z30intel_sub_group_block_write8_8PU3AS1jDv64_j",
        "_Z31intel_sub_group_block_write8_16PU3AS1jDv128_j"}},
  };


  auto AddBuiltin = [&Info](BuiltinInfo &Builtin, std::vector<VectorKind> Params, unsigned VF,
                                unsigned Idx) -> void {
    Info.emplace_back(Builtin.first,
                      VectorVariant{VectorVariant::ISAClass::XMM,
                                    false /*Masked*/,
                                    VF,
                                    Params,
                                    "", // Empty BaseName - does not matter
                                    Builtin.second[Idx]});
  };

  auto AddReadBuiltin = [&AddBuiltin](BuiltinInfo &Builtin, unsigned VF,
                                unsigned Idx) -> void {
    AddBuiltin(Builtin, {VectorKind::uniform()}, VF, Idx);
  };

  for (auto &ReadBuiltin : ReadBuiltins) {
    AddReadBuiltin(ReadBuiltin, 4, 0);
    AddReadBuiltin(ReadBuiltin, 8, 1);
    AddReadBuiltin(ReadBuiltin, 16, 2);
  }

  auto AddWriteBuiltin = [&AddBuiltin](BuiltinInfo &Builtin, unsigned VF,
                                unsigned Idx) -> void {
    AddBuiltin(Builtin, {VectorKind::uniform(), VectorKind::vector()}, VF, Idx);
  };

  for (auto &WriteBuiltin : WriteBuiltins) {
    AddWriteBuiltin(WriteBuiltin, 4, 0);
    AddWriteBuiltin(WriteBuiltin, 8, 1);
    AddWriteBuiltin(WriteBuiltin, 16, 2);
  }
}

static ContainerTy OCLBuiltinVecInfo() {
  ContainerTy Info;

  addEntries(Info, "_Z13sub_group_all", TypeInfo{'i'}, {VectorKind::vector()});
  addEntries(Info, "_Z13sub_group_any", TypeInfo{'i'}, {VectorKind::vector()});

  addBlockReadWriteBuiltins(Info);

  TypeInfo Types[] = {{'i'}, {'j'}, {'l'}, {'m'}, {'f'}, {'d'}};
  for (TypeInfo &Type : Types) {
    addEntries(Info, std::string("_Z19sub_group_broadcast"), {Type, {'j'}},
               {VectorKind::vector(), VectorKind::uniform()});
    addEntries(Info, std::string("_Z20sub_group_reduce_add"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z20sub_group_reduce_max"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z20sub_group_reduce_min"), Type,
               {VectorKind::vector()});

    addEntries(Info, std::string("_Z28sub_group_scan_exclusive_add"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z28sub_group_scan_exclusive_max"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z28sub_group_scan_exclusive_min"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z28sub_group_scan_inclusive_add"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z28sub_group_scan_inclusive_max"), Type,
               {VectorKind::vector()});
    addEntries(Info, std::string("_Z28sub_group_scan_inclusive_min"), Type,
               {VectorKind::vector()});
  }
  TypeInfo ShuffleTypes[] = {{'i'}, {'i', 2}, {'i', 4}, {'i', 8}, {'i', 16},
                             {'j'}, {'j', 2}, {'j', 4}, {'j', 8}, {'j', 16},
                             {'f'}, {'f', 2}, {'f', 4}, {'f', 8}, {'f', 16},
                             {'l'}, {'m'},    {'d'}};
  for (TypeInfo &Type : ShuffleTypes) {
    addEntries(Info, std::string("_Z23intel_sub_group_shuffle"), {Type, {'j'}},
               {VectorKind::vector(), VectorKind::vector()}, true);
    addEntries(
        Info, std::string("_Z28intel_sub_group_shuffle_down"),
        {Type, Type, {'j'}},
        {VectorKind::vector(), VectorKind::vector(), VectorKind::vector()},
        true);
    addEntries(
        Info, std::string("_Z26intel_sub_group_shuffle_up"),
        {Type, Type, {'j'}},
        {VectorKind::vector(), VectorKind::vector(), VectorKind::vector()},
        true);
    addEntries(Info, std::string("_Z27intel_sub_group_shuffle_xor"),
               {Type, {'j'}}, {VectorKind::vector(), VectorKind::vector()},
               true);
  }
  return Info;
}

static std::pair<const char *, VectorKind> OCLBuiltinReturnInfo[] = {
    // sub_group_get_local_id isn't here due to special processing in VecClone
    // pass. Void-argument functions returning uniform values are implicitly
    // known as uniform too.
    {"_Z13sub_group_alli", VectorKind::uniform()},
    {"_Z13sub_group_anyi", VectorKind::uniform()},
    {"_Z19sub_group_broadcastij", VectorKind::uniform()},
    {"_Z19sub_group_broadcastjj", VectorKind::uniform()},
    {"_Z19sub_group_broadcastlj", VectorKind::uniform()},
    {"_Z19sub_group_broadcastmj", VectorKind::uniform()},
    {"_Z19sub_group_broadcastfj", VectorKind::uniform()},
    {"_Z19sub_group_broadcastdj", VectorKind::uniform()},
    {"_Z20sub_group_reduce_addi", VectorKind::uniform()},
    {"_Z20sub_group_reduce_addj", VectorKind::uniform()},
    {"_Z20sub_group_reduce_addl", VectorKind::uniform()},
    {"_Z20sub_group_reduce_addm", VectorKind::uniform()},
    {"_Z20sub_group_reduce_addf", VectorKind::uniform()},
    {"_Z20sub_group_reduce_addd", VectorKind::uniform()},
    {"_Z20sub_group_reduce_maxi", VectorKind::uniform()},
    {"_Z20sub_group_reduce_maxj", VectorKind::uniform()},
    {"_Z20sub_group_reduce_maxl", VectorKind::uniform()},
    {"_Z20sub_group_reduce_maxm", VectorKind::uniform()},
    {"_Z20sub_group_reduce_maxf", VectorKind::uniform()},
    {"_Z20sub_group_reduce_maxd", VectorKind::uniform()},
    {"_Z20sub_group_reduce_mini", VectorKind::uniform()},
    {"_Z20sub_group_reduce_minj", VectorKind::uniform()},
    {"_Z20sub_group_reduce_minl", VectorKind::uniform()},
    {"_Z20sub_group_reduce_minm", VectorKind::uniform()},
    {"_Z20sub_group_reduce_minf", VectorKind::uniform()},
    {"_Z20sub_group_reduce_mind", VectorKind::uniform()},
};

void OCLVecClone::languageSpecificInitializations(Module &M) {
  OCLPrepareKernelForVecClone PK(CPUId);

  // FIXME: Longer term plan is to make the return value propery part of
  // VectorVariant encoding.
  //
  // Also, note that we're annotating declarations here. It's legal because:
  //   - The uniformity is true for all the VFs possible
  //   - The attribute doesn't have any other existing meaning so we are free to
  //     choose what is suitable. Not putting too much effort into designing the
  //     attribute due to it being a temporary solution (see FIXME above).
  for (auto &Entry : OCLBuiltinReturnInfo) {
    StringRef ScalarFnName = Entry.first;
    Function *Fn = M.getFunction(ScalarFnName);
    if (!Fn)
      continue;

    assert(Entry.second.isUniform() && "Only uniforms are supported by now!");
    Fn->addFnAttr("opencl-vec-uniform-return");
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
    auto MD = KernelMetadataAPI(F);
    auto VecTypeHint = MD.VecTypeHint;
    bool EnableVect = true;

    if (isSimpleFunction(F))
      EnableVect = false;

    // Looks for vector type in metadata.
    if (!MD.hasVecLength() && VecTypeHint.hasValue()) {
      Type *VTHTy = VecTypeHint.getType();
      if (!VTHTy->isFloatTy() && !VTHTy->isDoubleTy() &&
          !VTHTy->isIntegerTy(8) && !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) && !VTHTy->isIntegerTy(64)) {
        EnableVect = false;
      }
    }

    if (EnableVect)
      PK.run(F);
  }
}
} // namespace intel

extern "C" Pass *createOCLVecClonePass(const Intel::CPUId *CPUId,
                                       bool EnableVPlanVecForOpenCL) {
  return new intel::OCLVecClone(CPUId, EnableVPlanVecForOpenCL);
}
