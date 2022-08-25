//==--- InstToFuncCall.cpp - Replaces inst with func call --- C++ -*--------==//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/InstToFuncCall.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

extern cl::opt<VFISAKind> IsaEncodingOverride;

static cl::opt<bool> ReplaceFDivFastWithSVML(
    "dpcpp-kernel-replace-fdiv-fast-with-svml", cl::init(false), cl::Hidden,
    cl::desc("Replace fdiv fast with svml relaxed divide"));

namespace {

class Inst2FunctionLookup {
public:
  using LookupValue = std::pair<StringRef, CallingConv::ID>;

  Inst2FunctionLookup(VFISAKind ISA) {
    if (IsaEncodingOverride.getNumOccurrences())
      ISA = IsaEncodingOverride.getValue();

    Type2ValueLookup FpToUiLookup;
    Type2ValueLookup FpToSiLookup;
    Type2ValueLookup UiToFpLookup;
    Type2ValueLookup SiToFpLookup;

    /// Replaces:
    /// %conv = fptoui double %tmp2 to i64
    /// With:
    /// %call_conv = call i64 @_Z13convert_ulongd(double %tmp2) nounwind
    FpToUiLookup[std::make_pair(Integer64, Double)] =
        std::make_pair("_Z13convert_ulongd", CallingConv::C);

    /// Replaces:
    /// %conv = fptoui float %tmp2 to i64
    /// With:
    /// %call_conv = call i64 @_Z13convert_ulongf(float %tmp2) nounwind
    FpToUiLookup[std::make_pair(Integer64, Float)] =
        std::make_pair("_Z13convert_ulongf", CallingConv::C);

    /// Replaces:
    /// %conv = fptoui double %tmp2 to i32
    /// With:
    /// %call_conv = call i32 @_Z12convert_uintd(double %tmp2) nounwind
    FpToUiLookup[std::make_pair(Integer32, Double)] =
        std::make_pair("_Z12convert_uintd", CallingConv::C);

    /// Replaces:
    /// %conv = sitofp i64 %tmp2 to double
    /// With:
    /// %call_conv = call double @_Z14convert_doublel(i64 %tmp2) nounwind
    SiToFpLookup[std::make_pair(Double, Integer64)] =
        std::make_pair("_Z14convert_doublel", CallingConv::C);

    bool IsV16Supported = (ISA == VFISAKind::AVX512);
    if (IsV16Supported) {
      /// Replaces:
      /// %conv = fptoui <16 x float> %tmp2 to <16 x i64>
      /// With:
      /// %call_conv = call <16 x i64> @_Z15convert_ulong16Dv16_f(<16 x
      /// float> %tmp2) nounwind
      FpToUiLookup[std::make_pair(V16xInteger64, V16xFloat)] =
          std::make_pair("_Z15convert_ulong16Dv16_f", CallingConv::C);

      /// Replaces:
      /// %conv = fptoui <16 x double> %tmp2 to <16 x i64>
      /// With:
      /// %call_conv = call <16 x i64> @_Z15convert_ulong16Dv16_d(<16 x
      /// double> %tmp2) nounwind
      FpToUiLookup[std::make_pair(V16xInteger64, V16xDouble)] =
          std::make_pair("_Z15convert_ulong16Dv16_d", CallingConv::C);

      /// Replaces:
      /// %conv = fptosi float %tmp2 to i64
      /// With:
      /// %call_conv = call i64 @_Z12convert_longf(float %tmp2) nounwind
      FpToSiLookup[std::make_pair(Integer64, Float)] =
          std::make_pair("_Z12convert_longf", CallingConv::C);

      /// Replaces:
      /// %conv = fptosi <16 x float> %tmp2 to <16 x i64>
      /// With:
      /// %call_conv = call <16 x i64> @_Z14convert_long16Dv16_f(<16 x
      /// float> %tmp2) nounwind
      FpToSiLookup[std::make_pair(V16xInteger64, V16xFloat)] =
          std::make_pair("_Z14convert_long16Dv16_f", CallingConv::C);

      /// Replaces:
      /// %conv = fptosi double %tmp2 to i64
      /// With:
      /// %call_conv = call i64 @_Z12convert_longd(double %tmp2) nounwind
      FpToSiLookup[std::make_pair(Integer64, Double)] =
          std::make_pair("_Z12convert_longd", CallingConv::C);

      /// Replaces:
      /// %conv = fptosi <16 x double> %tmp2 to <16 x i64>
      /// With:
      /// %call_conv = call <16 x i64> @_Z14convert_long16Dv16_d(<16 x
      /// double> %tmp2) nounwind
      FpToSiLookup[std::make_pair(V16xInteger64, V16xDouble)] =
          std::make_pair("_Z14convert_long16Dv16_d", CallingConv::C);

      /// Replaces:
      /// %conv = sitofp i64 %tmp2 to float
      /// With:
      /// %call_conv = call float @_Z13convert_floatl(i64 %tmp2) nounwind
      SiToFpLookup[std::make_pair(Float, Integer64)] =
          std::make_pair("_Z13convert_floatl", CallingConv::C);

      /// Replaces:
      /// %conv = sitofp <16 x i64> %tmp2 to <16 x float>
      /// With:
      /// %call_conv = call <16 x float> @_Z15convert_float16Dv16_l(<16 x
      /// i64> %tmp2) nounwind
      SiToFpLookup[std::make_pair(V16xFloat, V16xInteger64)] =
          std::make_pair("_Z15convert_float16Dv16_l", CallingConv::C);

      /// Replaces:
      /// %conv = uitofp i64 %tmp2 to double
      /// With:
      /// %call_conv = call double @_Z13convert_doublem(i64 %tmp2)
      /// nounwind
      UiToFpLookup[std::make_pair(Double, Integer64)] =
          std::make_pair("_Z14convert_doublem", CallingConv::C);

      /// Replaces:
      /// %conv = uitofp <16 x i64> %tmp2 to <16 x float>
      /// With:
      /// %call_conv = call <16 x float> @_Z15convert_float16Dv16_m(<16 x
      /// i64> %tmp2) nounwind
      UiToFpLookup[std::make_pair(V16xFloat, V16xInteger64)] =
          std::make_pair("_Z15convert_float16Dv16_m", CallingConv::C);

      /// Replaces:
      /// %conv = uitofp <16 x i64> %tmp2 to <16 x double>
      /// With:
      /// %call_conv = call <16 x double> @_Z16convert_double16Dv16_m(<16
      /// x i64> %tmp2) nounwind
      UiToFpLookup[std::make_pair(V16xDouble, V16xInteger64)] =
          std::make_pair("_Z16convert_double16Dv16_m", CallingConv::C);

      /// Replaces:
      /// %conv = uitofp i64 %tmp2 to float
      /// With:
      /// %call_conv = call float @_Z13convert_floatm(i64 %tmp2) nounwind
      UiToFpLookup[std::make_pair(Float, Integer64)] =
          std::make_pair("_Z13convert_floatm", CallingConv::C);
    }

    Lookup[Instruction::UIToFP] = std::move(UiToFpLookup);
    Lookup[Instruction::SIToFP] = std::move(SiToFpLookup);
    Lookup[Instruction::FPToUI] = std::move(FpToUiLookup);
    Lookup[Instruction::FPToSI] = std::move(FpToSiLookup);

    // Maximum ULP of 'fdiv fast' on AVX/SSE42 is 3, while limit is 2.5
    // in OpenCL spec. So we replace 'fdiv fast' with svml function on
    // AVX/SSE42.
    if (ReplaceFDivFastWithSVML ||
        (ISA == VFISAKind::SSE || ISA == VFISAKind::AVX)) {
      Type2ValueLookup FDivLookup;
      FDivLookup[{Float, Float}] = {"_Z9divide_rmff", CallingConv::C};
      FDivLookup[{V2xFloat, V2xFloat}] = {"_Z9divide_rmDv2_fS_",
                                          CallingConv::C};
      FDivLookup[{V3xFloat, V3xFloat}] = {"_Z9divide_rmDv3_fS_",
                                          CallingConv::C};
      FDivLookup[{V4xFloat, V4xFloat}] = {"_Z9divide_rmDv4_fS_",
                                          CallingConv::C};
      FDivLookup[{V8xFloat, V8xFloat}] = {"_Z9divide_rmDv8_fS_",
                                          CallingConv::C};
      FDivLookup[{V16xFloat, V16xFloat}] = {"_Z9divide_rmDv16_fS_",
                                            CallingConv::C};
      LookupFastMath[Instruction::FDiv] = std::move(FDivLookup);
    }
  }

  const LookupValue *operator[](const Instruction &I) const {
    Opcode2T2VLookup::const_iterator It;
    if (isa<FPMathOperator>(I) && I.isFast()) {
      It = LookupFastMath.find(I.getOpcode());
      if (It == LookupFastMath.end())
        return nullptr;
    } else {
      It = Lookup.find(I.getOpcode());
      if (It == Lookup.end())
        return nullptr;
    }

    const Type2ValueLookup &Lookup2 = It->second;

    Value *In = I.getOperand(0);
    Type *InType = In->getType();
    TypeInfo TI = getTypeInfo(InType);
    if (Unknown == TI)
      return nullptr;

    Type *OutType = I.getType();
    TypeInfo TO = getTypeInfo(OutType);
    if (Unknown == TO)
      return nullptr;

    Type2ValueLookup::const_iterator It2 = Lookup2.find(std::make_pair(TO, TI));
    // If we are here, the lookup must have the mapping.
    return It2 == Lookup2.end() ? nullptr : &It2->second;
  }

private:
  // maps [DstType,SrcType] --> LookupValue
  typedef DenseMap<std::pair<int, int>, LookupValue> Type2ValueLookup;

  // maps instruction opcode --> Type2ValueLookup
  typedef DenseMap<unsigned, Type2ValueLookup> Opcode2T2VLookup;

  enum TypeInfo {
    Unknown,
    Integer32,
    Integer64,
    Float,
    Double,
    Half,
    V2xFloat,
    V3xFloat,
    V4xFloat,
    V8xFloat,
    V16xInteger64,
    V16xFloat,
    V16xDouble
  };

  Opcode2T2VLookup Lookup;
  Opcode2T2VLookup LookupFastMath;

  static TypeInfo getTypeInfo(Type *Ty) {
    if (auto *ITy = dyn_cast<IntegerType>(Ty)) {
      switch (ITy->getBitWidth()) {
      default:
        return Unknown;
      case 32:
        return Integer32;
      case 64:
        return Integer64;
      }
    }

    if (Ty->isFloatTy())
      return Float;
    if (Ty->isDoubleTy())
      return Double;
    if (Ty->isHalfTy())
      return Half;
    if (Ty->isVectorTy()) {
      FixedVectorType *VTy = cast<FixedVectorType>(Ty);
      Type *EltTy = VTy->getElementType();
      int NumElts = VTy->getNumElements();
      if (EltTy->isFloatTy()) {
        if (NumElts == 2)
          return V2xFloat;
        if (NumElts == 3)
          return V3xFloat;
        if (NumElts == 4)
          return V4xFloat;
        if (NumElts == 8)
          return V8xFloat;
        if (NumElts == 16)
          return V16xFloat;
      }
      if (NumElts != 16)
        return Unknown;
      if (EltTy->isDoubleTy())
        return V16xDouble;
      if (auto *ITy = dyn_cast<IntegerType>(EltTy))
        if (ITy->getBitWidth() == 64)
          return V16xInteger64;
    }

    return Unknown;
  }
};

class InstToFuncCallImpl {
public:
  bool run(Module &M, VFISAKind ISA);

private:
  void replaceInstWithCall(Function &F, Instruction &I, StringRef FName,
                           CallingConv::ID CC);
};

} // namespace

bool InstToFuncCallImpl::run(Module &M, VFISAKind ISA) {
  bool Changed = false;

  Inst2FunctionLookup I2F(ISA);
  SmallVector<Instruction *, 16> ToRemove;
  for (Function &F : M) {
    for (auto &I : instructions(F)) {
      // Check if a mapping exists for replacing this instruction class.
      const Inst2FunctionLookup::LookupValue *LV = I2F[I];
      if (!LV)
        continue;
      replaceInstWithCall(F, I, LV->first, LV->second);
      ToRemove.push_back(&I);
      Changed = true;
    }
  }

  for (Instruction *I : ToRemove)
    I->eraseFromParent();

  return Changed;
}

void InstToFuncCallImpl::replaceInstWithCall(Function &F, Instruction &I,
                                             StringRef FName,
                                             CallingConv::ID CC) {
  SmallVector<Value *, 8> Args(I.operands());
  SmallVector<Type *, 8> ArgTypes;
  transform(Args, std::back_inserter(ArgTypes),
            [](Value *V) { return V->getType(); });

  FunctionType *FuncTy =
      FunctionType::get(I.getType(), ArgTypes, /*isVarArg*/ false);
  FunctionCallee NewF = F.getParent()->getOrInsertFunction(FName, FuncTy);
  auto *CI = CallInst::Create(NewF, Args, "call_conv", &I);
  CI->setCallingConv(CC);
  CI->setDebugLoc(I.getDebugLoc());

  // So far the newly created function calls in this pass can have following
  // attributes in order to facilitate optimizations like LICM hoisting.
  CI->addFnAttr(Attribute::NoUnwind);
  CI->addFnAttr(Attribute::ReadNone);
  CI->addFnAttr(Attribute::WillReturn);

  I.replaceAllUsesWith(CI);
}

namespace {

/// For legacy pass manager.
class InstToFuncCallLegacy : public ModulePass {
public:
  static char ID;

  InstToFuncCallLegacy(VFISAKind ISA = VFISAKind::SSE)
      : ModulePass(ID), ISA(ISA) {
    initializeInstToFuncCallLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "InstToFuncCallLegacy"; }

  bool runOnModule(Module &M) override {
    InstToFuncCallImpl Impl;
    return Impl.run(M, ISA);
  }

private:
  VFISAKind ISA;
};

} // namespace

INITIALIZE_PASS(InstToFuncCallLegacy, "dpcpp-kernel-inst-to-func-call",
                "Replaces LLVM IR instructions with calls to functions", false,
                false)

char InstToFuncCallLegacy::ID = 0;

ModulePass *llvm::createInstToFuncCallLegacyPass(VFISAKind ISA) {
  return new InstToFuncCallLegacy(ISA);
}

PreservedAnalyses InstToFuncCallPass::run(Module &M, ModuleAnalysisManager &) {
  InstToFuncCallImpl Impl;
  if (!Impl.run(M, ISA))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
