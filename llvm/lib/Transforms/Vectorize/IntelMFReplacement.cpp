//===-------------------IntelMFReplacement.cpp ----------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// OpenCL: Replace arithmetic instructions like udiv, idiv, urem and srem
// with OpenCL vector functions.
// Non-OCL:
//  - Fuse float sinf and cosf pairs to:
//     %sin.val = _Z6sincosfPf(float %f, float* %cos.ptr)
//     (the OCL format recognized by the vectorizer)
//  - Split non-vectorized _Z6sincosfPf back to sinf+cosf.
//  - Change libc sincos to _Z6sincosfPf as above.
//  - Change vectorized OCL sincos:
//      void @_Z14sincos_ret2ptrDv8_fPS_S1_(<8 x float> %angle, <8 x float>*
//        cosPtr, <8 x float>* %sinPtr)
//     to x86 SVML sincos:
//       {<8 x float>, <8 x float>} = __svml_sincosf8(<8 x float> %angle)
//   Only 32-bit float unmasked types are supported.
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/User.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/Vectorize.h"
#include <unordered_map>

using namespace llvm;
using namespace vpo;

#define DEBUG_TYPE "mf-replace"

static cl::opt<bool> DisableMFReplacement(
    "disable-mf-replacement", cl::init(false), cl::Hidden,
    cl::desc("Disable replacement of math-instruction like u/i-div and i/s-rem "
             "with scalar SVML function."));

static cl::opt<bool>
    IsX86Target("mf-x86-target", cl::Hidden,
                cl::desc("Perform only conversions for x86 targets."));

STATISTIC(NumInstConverted, "Number of instructions converted");

class MathLibraryFunctionsReplacementPass
    : PassInfoMixin<MathLibraryFunctionsReplacementPass> {
private:
  bool isOCL;

public:
  MathLibraryFunctionsReplacementPass(bool isOCL) : isOCL(isOCL) {}
  // Run the pass over the function.
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

static bool isOptimizableOperation(Instruction *Inst) {
  Value *Divisor = Inst->getOperand(1);

  // This change is valid only for non-vectors and 32-bit integers.
  if (!Divisor->getType()->isIntegerTy(32))
    return false;

  // Do not replace functions with constant divisors as they may be replaced
  // with shifts in the future
  if (Constant *C = dyn_cast<Constant>(Divisor)) {
    const APInt &ConstIntVal = cast<ConstantInt>(C)->getValue();
    if (ConstIntVal.isPowerOf2() || (-ConstIntVal).isPowerOf2())
      return false;
  }
  return true;
}

// We guard the sin/cos => sincos transform, under the -ffast-math flag.
static bool funcHasUnsafeFPAttr(Function *F) {
  if (F->hasFnAttribute("unsafe-fp-math")) {
    Attribute Attr = F->getFnAttribute("unsafe-fp-math");
    StringRef Val = Attr.getValueAsString();
    return Val == "true";
  }
  return false;
}

static bool isSin(CallInst *Call) {
  if (auto *Intrin = dyn_cast<IntrinsicInst>(Call))
    return Intrin->getIntrinsicID() == Intrinsic::sin;

  StringRef NameCpp = "_Z3sinf"; // TODO: double
  StringRef NameC = "sinf";      // TODO: double
  Function *Func = Call->getCalledFunction();
  return Func && (Func->getName() == NameCpp || Func->getName() == NameC);
}

static bool isCos(CallInst *Call) {
  if (auto *Intrin = dyn_cast<IntrinsicInst>(Call))
    return Intrin->getIntrinsicID() == Intrinsic::cos;

  StringRef NameC = "cosf";      // TODO: double
  StringRef NameCpp = "_Z3cosf"; // TODO: double
  Function *Func = Call->getCalledFunction();
  return Func && (Func->getName() == NameCpp || Func->getName() == NameC);
}

// Returns attribute { nounwind }
static AttributeList getNoUnwindAttr(LLVMContext &C) {
  AttrBuilder AB;
  AB.addAttribute(Attribute::NoUnwind);
  return AttributeList::get(C, AttributeList::FunctionIndex, AB);
}

// Returns attributes { nounwind, readnone }
static AttributeList getPureAttr(LLVMContext &C) {
  AttrBuilder AB;
  AB.addAttribute(Attribute::NoUnwind);
  AB.addAttribute(Attribute::ReadNone);
  AttributeList PureAttr =
      AttributeList::get(C, AttributeList::FunctionIndex, AB);
  return PureAttr;
}

// If this sinf/cosf call dominates or is dominated by opposite cosf/sinf calls
// of the same value, generate a call to sincos at the highest dominator
// and replace the uses of the individual sin/cos calls.
// This call and all the replaced calls are removed.
// (its uses are modified). Example of generated IR:
//   %cos.ptr = alloca float
//   %sin.val = call void @_Z6sincosfPf(float %f, float* %cos.ptr)
//   %cos.val = load float, float* %cos.ptr
//
// The above IR is OpenCL specific "float sincos(float,float*)".
// If we generate libc sincos in the future, we need to convert to OCL sincos
// in paropt.
static bool combineSinCos(CallInst *Call, StringRef OCLSinCosName,
                          DominatorTree &DT) {
  // Check if this call has already been converted/removed, before any other
  // code.
  if (!Call->hasNUsesOrMore(1))
    return false;

  bool IsSin = isSin(Call);
  assert(IsSin || isCos(Call));

  if (!funcHasUnsafeFPAttr(Call->getFunction()))
    return false;

  Value *Angle = Call->getOperand(0);
  assert(Angle);
  Type *AngleType = Angle->getType();
  if (!AngleType->isFloatTy()) // TODO: double
    return false;
  if (Call->getType() != AngleType)
    return false;
  // Don't fuse sin+cos if the call will be folded.
  if (isa<Constant>(Angle))
    return false;

  // Search the uses of "Angle" and collect all sin/cos calls that dominate
  // or are dominated by the given call.
  Instruction *DominantI = Call;
  SmallVector<CallInst *, 2> Cands;
  for (User *U : Angle->users()) {
    if (auto *CallUser = dyn_cast<CallInst>(U)) {
      if ((isSin(CallUser) || isCos(CallUser)) &&
          (DT.dominates(CallUser, Call) || DT.dominates(Call, CallUser)) &&
          CallUser->getOperand(0) == Angle) {
        assert(CallUser != Call && "Expecting strict dominators");
        Cands.push_back(CallUser);
        // Keep track of the most-dominating call.
        if (DT.dominates(CallUser, DominantI))
          DominantI = CallUser;
      }
    }
  }
  // Finally add the original call given to us.
  Cands.push_back(Call);

  bool found = false;

  // At least one candidate must be the other cos/sin cofunction.
  for (auto *Cand : Cands)
    if ((IsSin && isCos(Cand)) || (!IsSin && isSin(Cand))) {
      found = true;
      break;
    }
  if (!found)
    return false;

  Module *M = Call->getParent()->getModule();
  auto &DL = M->getDataLayout();
  // sincos pass-by-reference temporary, allocated at the top of the function.
  // This address is passed to sincos.
  auto *CosTmp = new AllocaInst(
      AngleType, 0, nullptr, MaybeAlign(DL.getStackAlignment().value()),
      "cos.ptr", Call->getFunction()->getEntryBlock().getFirstNonPHI());

  // This stack temp needs to be declared private, if the sin/cos are inside
  // an OMP region.
  VPOParoptUtils::addPrivateToEnclosingRegion(CosTmp, Call, DT);

  // The rest of the sincos code will be inserted above the most dominating
  // candidate.
  IRBuilder<> B(DominantI);

  // gen sincos call
  AttributeList NoUnwind = getNoUnwindAttr(M->getContext());
  FunctionCallee Callee = M->getOrInsertFunction(
      OCLSinCosName, NoUnwind, AngleType, AngleType, CosTmp->getType());
  auto *SinVal = B.CreateCall(Callee, {Angle, CosTmp}, ""); // void rettype
  // If the original call was an LLVM intrinsic sin/cos, use "fast".
  if (isa<IntrinsicInst>(Call))
    SinVal->setFast(true);

  // Load the cosine result from the alloca temporary.
  auto *CosVal =
      B.CreateAlignedLoad(CosTmp->getType()->getPointerElementType(), CosTmp,
                          DL.getStackAlignment().value(), "cos.val");
  LLVM_DEBUG(dbgs() << __FUNCTION__ << "Generated sincos\n"
                    << SinVal << "\n"
                    << CosVal << "\n");
  for (auto *Cand : Cands) {
    LLVM_DEBUG(dbgs() << "Replaced\n" << Cand << "\n");
    if (isSin(Cand))
      Cand->replaceAllUsesWith(SinVal);
    else if (isCos(Cand))
      Cand->replaceAllUsesWith(CosVal);
    else
      llvm_unreachable("Expecting cos or sin");
    Cand->eraseFromParent();
  }
  return true;
}

// Splits the given call to _Z6sincosfPf into separate sin/cos calls.
// %SinVal = _Z6sincosfPf(%Angle, %CosPtr)
// =>
// %SinVal = sinf(%Angle)
// %CosVal = cosf(%Angle)
// store %CosVal, %CosPtr (we store-forward if possible)
static bool splitSinCosCall(CallInst *Call) {
  // sinval = _Z6sincosfPf(float Angle, float *CosPtr)
  // cosval = load CosPtr

  Value *Angle = Call->getOperand(0);
  assert(Angle);
  Type *AngleType = Angle->getType();
  assert(AngleType->isFloatTy() && "Only float sincos is supported."); // TODO

  auto *CosPtr = Call->getOperand(1);
  assert(CosPtr);
  IRBuilder<> B(Call);
  Module *M = Call->getParent()->getModule();
  CallInst *SinCall, *CosCall;
  if (Call->isFast()) {
    // A fast sincos means that we can use fast sin/cos.
    Function *IntrSin =
        Intrinsic::getDeclaration(M, Intrinsic::sin, {AngleType});
    Function *IntrCos =
        Intrinsic::getDeclaration(M, Intrinsic::cos, {AngleType});
    SinCall = B.CreateCall(IntrSin, {Angle});
    CosCall = B.CreateCall(IntrCos, {Angle});
    SinCall->setFast(true);
    CosCall->setFast(true);
  } else {
    AttributeList NoUnwind = getNoUnwindAttr(M->getContext());

    // TODO: double
    FunctionCallee SinCallee =
        M->getOrInsertFunction("sinf", NoUnwind, AngleType, AngleType);
    SinCall = B.CreateCall(SinCallee, {Angle}, "sin.val");
    SinCall->setAttributes(NoUnwind);
    // TODO: double
    FunctionCallee CosCallee =
        M->getOrInsertFunction("cosf", NoUnwind, AngleType, AngleType);
    CosCall = B.CreateCall(CosCallee, {Angle}, "cos.val");
    CosCall->setAttributes(NoUnwind);
  }

  // If there is a load of CosPtr we can skip the store and forward to the
  // load.
  LoadInst *CosLoad = nullptr;
  for (auto U : CosPtr->users())
    if (CosLoad = dyn_cast<LoadInst>(U))
      break;

  if (CosLoad) {
    CosLoad->replaceAllUsesWith(CosCall);
    CosLoad->eraseFromParent();
  } else {
    // Otherwise, store-back the cos result to CosPtr.
    B.CreateStore(CosCall, CosPtr);
  }

  Call->replaceAllUsesWith(SinCall);
  Call->eraseFromParent();

  return true;
}

// call @sincosf(angle, sinPtr, cosPtr)
// =>
// %1 = call _Z6sincosfPf(angle, cosPtr)
// store %1, sinPtr
static bool SinCosCallToOCL(CallInst *Call, StringRef OCLSinCosName) {
  // Don't assert these because the sincos call is from the user and
  // may be nonstandard.
  if (Call->getNumOperands() != 4)
    return false;
  Value *Angle = Call->getOperand(0);
  Value *SinPtr = Call->getOperand(1);
  Value *CosPtr = Call->getOperand(2);
  if (!SinPtr->getType()->isPointerTy() || !CosPtr->getType()->isPointerTy())
    return false;
  if (!Call->getType()->isVoidTy())
    return false;
  auto *AngleType = Angle->getType();
  if (!AngleType->isFloatTy())
    return false; // TODO: double

  // without this check, we fail exact-precision library tests
  if (!funcHasUnsafeFPAttr(Call->getFunction()))
    return false;

  // gen OCL sincos call
  auto *M = Call->getModule();
  IRBuilder<> B(Call);
  AttributeList NoUnwind = getNoUnwindAttr(M->getContext());
  FunctionCallee Callee = M->getOrInsertFunction(
      OCLSinCosName, NoUnwind, AngleType, AngleType, CosPtr->getType());
  auto *SinVal = B.CreateCall(Callee, {Angle, CosPtr}, ""); // void rettype
  SinVal->setFast(true);

  // store the sine result back to the pointer from the original sincos call
  B.CreateStore(SinVal, SinPtr);

  Call->eraseFromParent();
  return true;
}

// call void @_Z14sincos_ret2ptrDv8_fPS_S1_(<8 x float> %angle, <8 x float>*
//   cosPtr, <8 x float>* %sinPtr)
// %wide.cos = load <8 x float>, <8 x float>* %cosPtr
// %wide.sin = load <8 x float>, <8 x float>* %sinPtr
// =>
// %0 = call {<8 x float>, <8 x float>} __svml_sincosf8(<8 x float> %angle)
// %wide.sin = extractvalue %0, 0
// %wide.cos = extractvalue %0, 1
static bool rewriteVSinCosCall(CallInst *Call, StringRef SVMLSinCosPrefix) {
  assert(Call->getNumOperands() == 4 && "Incorrect vectorized sincos");
  Value *Angle = Call->getOperand(0);
  Value *CosPtr = Call->getOperand(1);
  Value *SinPtr = Call->getOperand(2);
  assert(SinPtr->getType()->isPointerTy() && CosPtr->getType()->isPointerTy() &&
         "Vector sincos does not have pointer type args");
  VectorType *AngleType = dyn_cast<VectorType>(Angle->getType());
  assert(AngleType && "Vector sincos is not a vector type.");
  assert(AngleType->getElementType()->isFloatTy() &&
         "Only float vector sincos supported."); // TODO, double

  // Create the call to svml_sincosfNN.
  Type *RetStrTy = StructType::create({AngleType, AngleType}, "sincosret");
  IRBuilder<> B(Call);
  Module *M = Call->getParent()->getModule();
  std::string SVMLSinCosName = SVMLSinCosPrefix.str() + "f" +
                               std::to_string(AngleType->getElementCount().Min);
  // Set nounwind, readnone
  AttributeList PureAttr = getPureAttr(M->getContext());
  FunctionCallee Callee =
      M->getOrInsertFunction(SVMLSinCosName, PureAttr, RetStrTy, AngleType);
  auto *SinCosV = B.CreateCall(Callee, {Angle}, "");
  SinCosV->setAttributes(PureAttr);

  // Generate the extracts of the return struct fields.
  auto *SinVal = B.CreateExtractValue(SinCosV, {0}, "");
  auto *CosVal = B.CreateExtractValue(SinCosV, {1}, "");

  // Replace the loads of SinPtr and CosPtr with the extracted values. We
  // leave the pointers alone.
  auto FindLoad = [](Value *PtrVal) -> LoadInst * {
    for (auto U : PtrVal->users())
      if (auto Load = dyn_cast<LoadInst>(U))
        return Load;
    return nullptr;
  };
  LoadInst *CosLoad = FindLoad(CosPtr);
  LoadInst *SinLoad = FindLoad(SinPtr);
  if (CosLoad) {
    // Remove the load and replace with the direct vector value from
    // the SVML sincos call.
    CosLoad->replaceAllUsesWith(CosVal);
    CosLoad->eraseFromParent();
  } else
    // Can't remove the load, store into the pointer.
    B.CreateStore(CosVal, CosPtr);

  if (SinLoad) {
    SinLoad->replaceAllUsesWith(SinVal);
    SinLoad->eraseFromParent();
  } else
    B.CreateStore(SinVal, SinPtr);

  // Finally, remove the call.
  Call->eraseFromParent();
  return true;
}

namespace llvm {

class MathLibraryFunctionsReplacement {
  DominatorTree &DT;

private:
  SmallVector<Instruction *, 4> DivRemInsts;
  SmallVector<CallInst *, 4> GenSinCosInsts;   // sin,cos,sincosf
  SmallVector<CallInst *, 4> LowerSinCosInsts; // OCL scalar and vector sincos
  StringRef OCLSinCosName = "_Z6sincosfPf";
  StringRef OCLVSinCosPrefix = "_Z14sincos_ret2ptrDv";
  StringRef SVMLSinCosPrefix = "__svml_sincos"; // need vlen suffix
  StringRef SinCosName = "sincosf";

  void collectMathInstructions(Function &F);
  bool transformDivRem(Module *M);
  bool transformDivRem(Instruction *I, Module *M);
  bool genOCLSinCos(void);
  bool lowerOCLSinCos(void);

  using InstOpcode = unsigned;
  using DivRemFnMap = std::unordered_map<InstOpcode, const char *>;

public:
  MathLibraryFunctionsReplacement(DominatorTree &DT) : DT(DT) {}
  static DivRemFnMap DivRemInstFnMap;
  bool run(Function &F, bool isOCL, bool cleanSinCosOnly) {
    bool Changed = false;

    // -mf-x86-target option
    if (IsX86Target.getNumOccurrences() > 0)
      isOCL = false;

    // collect all instructions that we may be interested into, into several
    // vectors.
    collectMathInstructions(F);

    if (cleanSinCosOnly)
      return lowerOCLSinCos();

    if (isOCL)
      Changed |= transformDivRem(F.getParent());
    else {
      // If we saw any OCL sincos and this is not an OCL target, we must have
      // run this pass already. Split instead of combining.
      if (!LowerSinCosInsts.empty())
        Changed |= lowerOCLSinCos();
      else
        Changed |= genOCLSinCos();
    }
    return Changed;
  }
};

MathLibraryFunctionsReplacement::DivRemFnMap
    MathLibraryFunctionsReplacement::DivRemInstFnMap = {
        {Instruction::UDiv, "_Z4udivjj"},
        {Instruction::SDiv, "_Z4idivii"},
        {Instruction::URem, "_Z4uremjj"},
        {Instruction::SRem, "_Z4iremii"}};

void MathLibraryFunctionsReplacement::collectMathInstructions(Function &F) {
  for (BasicBlock &Block : F) {
    for (Instruction &Inst : Block) {
      Instruction *I = &Inst;
      if (DivRemInstFnMap.count(I->getOpcode()) && isOptimizableOperation(I))
        DivRemInsts.push_back(I);
      else if (auto *Call = dyn_cast<CallInst>(I)) {
        Function *Func = Call->getCalledFunction();
        if (isSin(Call) || isCos(Call) ||
            (Func && (Func->getName() == SinCosName)))
          GenSinCosInsts.push_back(Call);
        else if (Func && (Func->getName() == OCLSinCosName ||
                          Func->getName().startswith(OCLVSinCosPrefix)))
          LowerSinCosInsts.push_back(Call);
      }
    }
  }
}

// Try to convert sin/cos/sincos calls listed in GenSinCosInsts,
// to OCL sincos format.
bool MathLibraryFunctionsReplacement::genOCLSinCos() {
  int NumConverted = 0;
  for (CallInst *Call : GenSinCosInsts) {
    Function *Func = Call->getCalledFunction();
    if (Func && Func->getName() == SinCosName) {
      SinCosCallToOCL(Call, OCLSinCosName);
      NumConverted++;
    } else if (combineSinCos(Call, OCLSinCosName, DT))
      NumConverted++;
    // neither may happen, if the sin/cos could not be converted.
  }
  NumInstConverted += NumConverted;
  return NumConverted > 0;
}

// Lower scalar OCL sincos to sin/cos.
// Lower vectorized 2-pointer OCL sincos to SVML sincos.
bool MathLibraryFunctionsReplacement::lowerOCLSinCos() {
  bool DidSomething = false;
  for (CallInst *Call : LowerSinCosInsts) {
    Function *Func = Call->getCalledFunction();
    assert(Func);
    if (Func->getName() == OCLSinCosName)
      DidSomething |= splitSinCosCall(Call);
    else if (Func->getName().startswith(OCLVSinCosPrefix))
      DidSomething |= rewriteVSinCosCall(Call, SVMLSinCosPrefix);
    else
      llvm_unreachable("Not an OCL sincos.");
  }
  return DidSomething;
}

bool MathLibraryFunctionsReplacement::transformDivRem(Module *M) {
  for (Instruction *Inst : DivRemInsts) {
    Type *InstTy = Inst->getType();

    Value *Dividend = Inst->getOperand(0);
    Value *Divisor = Inst->getOperand(1);

    assert(Divisor->getType()->isIntegerTy() &&
           "Unexpected divisor element type");

    SmallVector<Type *, 2> OperandTys;

    // Create the function-type and signature
    OperandTys.push_back(Dividend->getType());
    OperandTys.push_back(Divisor->getType());

    FunctionType *FnSignature = FunctionType::get(InstTy, OperandTys, false);
    Function *FnDecl = cast<Function>(
        M->getOrInsertFunction(DivRemInstFnMap[Inst->getOpcode()], FnSignature)
            .getCallee());

    // Create the call
    CallInst *NewCall = CallInst::Create(
        FnDecl, {Dividend, Divisor}, DivRemInstFnMap[Inst->getOpcode()], Inst);
    NewCall->setDebugLoc(Inst->getDebugLoc());

    // Replace original instruction with call
    Inst->replaceAllUsesWith(NewCall);
    Inst->eraseFromParent();
    NumInstConverted++;
  }
  return NumInstConverted > 0;
}

class MathLibraryFunctionsReplacementLegacyPass : public FunctionPass {
private:
  bool isOCL;

public:
  static char ID;

  MathLibraryFunctionsReplacementLegacyPass(bool isOCL)
      : FunctionPass(ID), isOCL(isOCL) {
    initializeMathLibraryFunctionsReplacementLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  // Remove this with a future OCL commit, passing "true".
  MathLibraryFunctionsReplacementLegacyPass(void)
      : MathLibraryFunctionsReplacementLegacyPass(true) {}

  bool runOnFunction(Function &F) override {
    // Return without any change if instruction to function replacement is
    // disabled.
    if (DisableMFReplacement)
      return false;

    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    MathLibraryFunctionsReplacement G(DT);
    NumInstConverted = 0;

    // Disabled by opt-bisect. We still need to clean up OCL sincos on x86,
    // otherwise a link error will occur.
    bool cleanSinCosOnly = skipFunction(F);
    if (isOCL && cleanSinCosOnly)
      return false;

    return G.run(F, isOCL, cleanSinCosOnly);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();

    AU.setPreservesCFG();

    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addPreserved<BasicAAWrapperPass>();
    AU.addPreserved<AAResultsWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    AU.addPreserved<SCEVAAWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end namespace llvm

char MathLibraryFunctionsReplacementLegacyPass::ID = 0;

PreservedAnalyses
MathLibraryFunctionsReplacementPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  MathLibraryFunctionsReplacement G(DT);
  // Update the last parameter if the new PM is enabled with bisection.
  if (!G.run(F, isOCL, false))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  // No loop behavior changes
  PA.preserveSet<AllAnalysesOn<Loop>>();

  // No AA effects
  PA.preserve<AAManager>();
  PA.preserve<BasicAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<SCEVAA>();
  PA.preserve<AndersensAA>();

  return PA;
}

INITIALIZE_PASS_BEGIN(
    MathLibraryFunctionsReplacementLegacyPass,
    "replace-with-math-library-functions",
    "Replace known math operations with optimized library functions", false,
    false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(
    MathLibraryFunctionsReplacementLegacyPass,
    "replace-with-math-library-functions",
    "Replace known math operations with optimized library functions", false,
    false)

// Remove this function with an OCL commit. The pass should be created
// from OCL with a "true" value.
Pass *llvm::createMathLibraryFunctionsReplacementPass() {
  return new MathLibraryFunctionsReplacementLegacyPass(true);
}

Pass *llvm::createMathLibraryFunctionsReplacementPass(bool isOCL) {
  return new MathLibraryFunctionsReplacementLegacyPass(isOCL);
}
