//=-- Intel_MathLibrariesDeclaration.cpp - Add math function declaration -*--=//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass goes through each llvm math intrinsic and adds a declaration for
// the corresponding math function in the IR if we are compiling for LTO. For
// example, if the IR contains this llvm intrinsic:
//
//   declare double @llvm.exp.f64(double %0)
//
// Then this pass will add the follow:
//
//   declare double @exp(double %0)
//
// The reason for adding these declarations is that we want the undefined math
// symbols during the first linker's symbol resolution. That will force the
// linker to pull Intel's math libraries before GNU math libraries to get the
// resolution.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_MathLibrariesDeclaration.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "intel-math-libraries-decl"

STATISTIC(NumOfNewPrototypesGenerated, "Number of new prototypes generated");

// Enable the generation of math function prototypes by default
static cl::opt<bool> EnableMathLibsDecl("enable-math-libs-decls",
                                        cl::init(true), cl::ReallyHidden);

// Helper class to add the declaration for math libraries
class MathLibrariesDeclImpl {
public:
  MathLibrariesDeclImpl(Module &M,
                        function_ref<TargetTransformInfo &(Function &)> GTTI)
      : M(M), GetTTI(GTTI), SinFunc(nullptr), CosFunc(nullptr) {}
  bool run();

private:
  Module &M;

  // Return true if the input function is an intrinsic to a simple math
  // function and we could generate the float, double or long prototype.
  // A simple math intrinsic is a function where all the arguments and
  // the return types are same. For example, assume that the module
  // contains the following intrinsic:
  //
  //   declare double @llvm.exp.f64(double %0)
  //
  // Then this function will generate the following declaration:
  //
  //   declare double @exp(double %0)
  //
  bool isSimpleTypesMathIntrinsic(Function &F);

  // Given a function F and a string, copy the prototype and replace the name
  // with NewFuncName. If the supplied name is a nullptr then the function will
  // return false.
  bool generateFuncPrototype(Function &F, const char *NewFuncName);

  // Given a function name and the function type, generate the declaration for
  // it if the function is not in the IR.
  bool generateFuncPrototype(StringRef &NewFuncName, FunctionType *FuncType);

  // Return true if the pass found sin and cos, and it generated sincos and
  // fmod. If sin and cos are in the compilation unit then the compiler might
  // optimize it by calling sincos (function optimized for computing the sine
  // and cosine for the same angle).
  bool generateSinCos();

  // Generate a declaration for __intel_new_feature_init if libirc is allowed
  // and we are compiling for advanced opt or auto CPU dispatch. This is
  // needed because the X86FeatureInitPass in the code generator may need to
  // generate a call to __intel_new_feature_init on the link step of an -flto
  // compilation, and library functions called by __intel_new_feature_init will
  // need to be recognized during the first LTO resolution pass.
  bool generateIntelNewFeatureProcInit(Function &F);

  function_ref<TargetTransformInfo &(Function &)> GetTTI;
  Function *SinFunc;
  Function *CosFunc;
};

// Generate the new function prototype from the input function
bool MathLibrariesDeclImpl::generateFuncPrototype(Function &F,
                                                  const char *NewFuncName) {

  if (NewFuncName == nullptr)
    return false;

  StringRef NewFuncNameRef(NewFuncName);

  assert(!NewFuncNameRef.empty() && "Trying to create a new math declaration "
                                    "with an empty name");

  // If the function exists then there is nothing to do
  if (M.getFunction(NewFuncNameRef))
    return false;

  auto *MathFunc =
      Function::Create(F.getFunctionType(), F.getLinkage(), NewFuncNameRef, M);
  MathFunc->setAttributes(F.getAttributes());

  NumOfNewPrototypesGenerated++;

  LLVM_DEBUG(dbgs() << "  Intrinsic @" << F.getName() << " generates "
                    << "function @" << NewFuncNameRef << "\n");

  return true;
}

// Generate the new function prototype with the function name and type
bool MathLibrariesDeclImpl::generateFuncPrototype(StringRef &NewFuncName,
                                                  FunctionType *FuncType) {

  if (FuncType == nullptr)
    return false;

  assert(!NewFuncName.empty() && "Trying to create a new math declaration "
                                    "with an empty name");

  // If the function exists then there is nothing to do
  if (M.getFunction(NewFuncName))
    return false;

  Function::Create(FuncType, llvm::GlobalValue::ExternalLinkage,
                   NewFuncName, M);

  NumOfNewPrototypesGenerated++;

  LLVM_DEBUG(dbgs() << "  New function @" << NewFuncName << " generated\n");

  return true;
}

// Special case for handling sincos from libm
bool MathLibrariesDeclImpl::generateSinCos() {

  // Sine and cosine must be found
  if (!SinFunc || !CosFunc)
    return false;

  // Make sure they both are used for computing the same type
  if (SinFunc->getReturnType() != CosFunc->getReturnType())
    return false;

  StringRef FSinCosName;
  StringRef FModName;
  SmallVector<Type *, 3> SinCosArgTys;
  SmallVector<Type *, 2> FModArgTys;
  LLVMContext &Cxt = M.getContext();
  switch (SinFunc->getReturnType()->getTypeID()) {
  case Type::FloatTyID:
    FSinCosName = StringRef("sincosf");
    FModName = StringRef("fmodf");
    SinCosArgTys.push_back(Type::getFloatTy(Cxt));
    SinCosArgTys.push_back(Type::getFloatPtrTy(Cxt));
    SinCosArgTys.push_back(Type::getFloatPtrTy(Cxt));
    FModArgTys.push_back(Type::getFloatTy(Cxt));
    FModArgTys.push_back(Type::getFloatTy(Cxt));
    break;
  case Type::DoubleTyID:
    FSinCosName = StringRef("sincos");
    FModName = StringRef("fmod");
    SinCosArgTys.push_back(Type::getDoubleTy(Cxt));
    SinCosArgTys.push_back(Type::getDoublePtrTy(Cxt));
    SinCosArgTys.push_back(Type::getDoublePtrTy(Cxt));
    FModArgTys.push_back(Type::getDoubleTy(Cxt));
    FModArgTys.push_back(Type::getDoubleTy(Cxt));
    break;
  case Type::X86_FP80TyID:
    FSinCosName = StringRef("sincosl");
    FModName = StringRef("fmodl");
    SinCosArgTys.push_back(Type::getX86_FP80Ty(Cxt));
    SinCosArgTys.push_back(Type::getX86_FP80PtrTy(Cxt));
    SinCosArgTys.push_back(Type::getX86_FP80PtrTy(Cxt));
    FModArgTys.push_back(Type::getX86_FP80Ty(Cxt));
    FModArgTys.push_back(Type::getX86_FP80Ty(Cxt));
    break;
  case Type::FP128TyID:
    FSinCosName = StringRef("sincosl");
    FModName = StringRef("fmodl");
    SinCosArgTys.push_back(Type::getFP128Ty(Cxt));
    SinCosArgTys.push_back(Type::getFP128PtrTy(Cxt));
    SinCosArgTys.push_back(Type::getFP128PtrTy(Cxt));
    FModArgTys.push_back(Type::getFP128Ty(Cxt));
    FModArgTys.push_back(Type::getFP128Ty(Cxt));
    break;
  default:
    break;
  }

  if (FSinCosName.empty() || FModName.empty() ||
      SinCosArgTys.size() != 3 || FModArgTys.size() != 2)
    return false;

  // Generate one of the following:
  //   void sincos(double, double*, double*)
  //   void sincosf(float, float*, float*)
  //   void sincosl(long double, long double*, long double*)
  Type *RetTy = Type::getVoidTy(Cxt);
  FunctionType *FuncTy = FunctionType::get(RetTy, SinCosArgTys, false);
  bool GenSinCos = generateFuncPrototype(FSinCosName, FuncTy);

  // Generate one of the following:
  //  double fmod(double, double)
  //  float fmodf(float, float)
  //  long double fmodl(long double, long double)
  RetTy = FModArgTys[0];
  FuncTy = FunctionType::get(RetTy, FModArgTys, false);
  bool GenFMod = generateFuncPrototype(FModName, FuncTy);

  return GenSinCos || GenFMod;
}

// Generate "__intel_new_feature_proc_init" if needed.
bool MathLibrariesDeclImpl::generateIntelNewFeatureProcInit(Function &F) {
  static bool AlreadyGenerated = false;
  if (AlreadyGenerated)
    return false;
  TargetTransformInfo *FTTI = &GetTTI(F);
  if (!FTTI->isLibIRCAllowed())
    return false;
  if (!FTTI->isIntelAdvancedOptimEnabled() &&
      !F.getMetadata("llvm.auto.cpu.dispatch"))
    return false;
  LLVMContext &Cxt = M.getContext();
  FunctionCallee FeatureInit = M.getOrInsertFunction(
      "__intel_new_feature_proc_init", Type::getVoidTy(Cxt),
      Type::getInt32Ty(Cxt), Type::getInt64Ty(Cxt));
  Function *NF = cast<Function>(FeatureInit.getCallee());
  appendToCompilerUsed(M, {NF});
  AlreadyGenerated = true;
  return true;
}

// Return true if the input function is an intrisic to a simple math
// function and the prototype for the actual math function was generated.
bool MathLibrariesDeclImpl::isSimpleTypesMathIntrinsic(Function &F) {

  // Identify which prototype needs to be created for a math intrinsic and
  // generate it. The input strings Fname, DName and LName represent the names
  // used for float, double or long version of the function declaration.
  auto GenerateSimpleTypesPrototype = [&F, this](const char *FName,
                                            const char *DName,
                                            const char *LName) -> bool {

    if (F.arg_size() == 0)
      return false;

    Type *TypeToCheck = F.getArg(0)->getType();
    for (unsigned I = 1; I < F.arg_size(); I++)
      if (F.getArg(I)->getType() != TypeToCheck)
        return false;

    if (F.getReturnType() != TypeToCheck)
      return false;

    bool Result = false;
    switch (TypeToCheck->getTypeID()) {
    case Type::FloatTyID:
      Result = generateFuncPrototype(F, FName);
      break;
    case Type::DoubleTyID:
      Result = generateFuncPrototype(F, DName);
      break;
    case Type::X86_FP80TyID:
    case Type::FP128TyID:
      Result = generateFuncPrototype(F, LName);
      break;
    default:
      break;
    }

    return Result;
  };

  if (!F.isIntrinsic())
    return false;

  bool Changed = false;

  // How to insert a new entry:
  //
  // 1) Make sure that the intrinsic will be lowered into the function you are
  //    expecting. You can check the LLVM reference manual or CodeGen sources
  //    to have an idea how it will be lowered.
  //
  // 2) Call the function GenerateSimpleTypesPrototype(FName, DName, LName), where
  //    there parameters are:
  //
  //    * FName: name of the function to be called to handle single precision
  //    * DName: name of the function that handles double precision
  //    * LName: name of the function that handles quadruple precision
  //
  //    If one of the names is nullptr then it won't generate a prototype for
  //    that precision. Make sure that the name of the function actually
  //    exists.
  //
  // 3) If you are trying to catch libimf version of the function, make sure
  //    that there is a definition for it in libimf.
  //
  // 4) If GenerateSimpleTypesPrototype won't do the lowering you need, then you may
  //    need to define your own function for specialized lowering.
  switch (F.getIntrinsicID()) {
  case Intrinsic::ceil:
    Changed = GenerateSimpleTypesPrototype("ceilf", "ceil", "ceill");
    break;
  case Intrinsic::copysign:
    Changed =
        GenerateSimpleTypesPrototype("copysignf", "copysign", "copysignl");
    break;
  case Intrinsic::cos:
    Changed = GenerateSimpleTypesPrototype("cosf", "cos", "cosl");
    CosFunc = &F;
    break;
  case Intrinsic::exp:
    Changed = GenerateSimpleTypesPrototype("expf", "exp", "expl");
    break;
  case Intrinsic::exp2:
    Changed = GenerateSimpleTypesPrototype("exp2f", "exp2", "exp2l");
    break;
  case Intrinsic::floor:
    Changed = GenerateSimpleTypesPrototype("floorf", "floor", "floorl");
    break;
  case Intrinsic::log:
    Changed = GenerateSimpleTypesPrototype("logf", "log", "logl");
    break;
  case Intrinsic::log2:
    Changed = GenerateSimpleTypesPrototype("log2f", "log2", "log2l");
    break;
  case Intrinsic::pow:
    Changed = GenerateSimpleTypesPrototype("powf", "pow", "powl");
    break;
  case Intrinsic::sin:
    Changed = GenerateSimpleTypesPrototype("sinf", "sin", "sinl");
    SinFunc = &F;
    break;
  case Intrinsic::sqrt:
    Changed = GenerateSimpleTypesPrototype("sqrtf", "sqrt", "sqrtl");
    break;
  case Intrinsic::round:
    Changed = GenerateSimpleTypesPrototype("roundf", "round", "roundl");
    break;
  case Intrinsic::trunc:
    Changed = GenerateSimpleTypesPrototype("truncf", "trunc", "truncl");
    break;
  default:
    break;
  }

  return Changed;
}

bool MathLibrariesDeclImpl::run() {

  if (!EnableMathLibsDecl)
    return false;

  bool Changed = false;

  LLVM_DEBUG(dbgs() << "Math function declarations added:\n");

  for (Function &F : M) {
    // NOTE: This function handles the cases where the arguments and return
    // types are the same. We may need to define other cases.
    Changed |= isSimpleTypesMathIntrinsic(F);
    // Check if the compiler might need to add __intel_new_feature_proc_init.
    Changed |= generateIntelNewFeatureProcInit(F);
  }

  // Check if there is a chance that the compiler might add sincos
  Changed |= generateSinCos();

  LLVM_DEBUG(dbgs() << "  Total functions added: "
                    << NumOfNewPrototypesGenerated << "\n");

  return Changed;
}

// New pass manager
PreservedAnalyses
IntelMathLibrariesDeclarationPass::run(Module &M, ModuleAnalysisManager &AM) {

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  auto GetTTI = [&FAM](Function &F) -> TargetTransformInfo & {
    return FAM.getResult<TargetIRAnalysis>(F);
  };

  MathLibrariesDeclImpl MathLibsDecl(M, GetTTI);
  MathLibsDecl.run();

  return PreservedAnalyses::all();
}

// Legacy pass manager
namespace {

class IntelMathLibrariesDeclarationWrapper : public ModulePass {
public:
  static char ID;

  IntelMathLibrariesDeclarationWrapper() : ModulePass(ID) {
    initializeIntelMathLibrariesDeclarationWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    TargetTransformInfoWrapperPass *TTIWP =
        &getAnalysis<TargetTransformInfoWrapperPass>();
    auto GetTTI = [&TTIWP](Function &F) -> TargetTransformInfo & {
      return TTIWP->getTTI(F);
    };
    MathLibrariesDeclImpl MathLibsDecl(M, GetTTI);
    return MathLibsDecl.run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesAll();
  }
};

} // end of anonymous namespace

char IntelMathLibrariesDeclarationWrapper::ID = 0;

INITIALIZE_PASS_BEGIN(IntelMathLibrariesDeclarationWrapper, DEBUG_TYPE,
                      "Intel Math Libraries Declaration", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(IntelMathLibrariesDeclarationWrapper, DEBUG_TYPE,
                    "Intel Math Libraries Declaration", false, false)

ModulePass *llvm::createIntelMathLibrariesDeclarationWrapperPass() {
  return new IntelMathLibrariesDeclarationWrapper();
}
