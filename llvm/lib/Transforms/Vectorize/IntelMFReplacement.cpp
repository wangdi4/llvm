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

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/BasicAliasAnalysis.h" // INTEL_CUSTOMIZATION
#include "llvm/Analysis/GlobalsModRef.h" // INTEL_CUSTOMIZATION
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h" // INTEL_CUSTOMIZATION
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/User.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/ModRef.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/Vectorize/IntelMFReplacement.h"
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

static bool isOptimizableOperation(Instruction *Inst) {
  Value *Divisor = Inst->getOperand(1);

  // This change is valid only for non-vectors and 32-bit integers.
  if (!Divisor->getType()->isIntegerTy(32))
    return false;

  if (isa<UndefValue>(Divisor))
    return false;

  // Do not replace functions with constant divisors as they may be replaced
  // with shifts in the future
  if (auto *C = dyn_cast<ConstantInt>(Divisor)) {
    const APInt &ConstIntVal = C->getValue();
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
  AttrBuilder AB(C);
  AB.addAttribute(Attribute::NoUnwind);
  return AttributeList::get(C, AttributeList::FunctionIndex, AB);
}

// Returns attributes { nounwind, readnone, willreturn }
static AttributeList getPureAttr(LLVMContext &C) {
  AttrBuilder AB(C);
  AB.addAttribute(Attribute::NoUnwind);
  AB.addMemoryAttr(llvm::MemoryEffects::none());
  AB.addAttribute(Attribute::WillReturn);
  AttributeList PureAttr =
      AttributeList::get(C, AttributeList::FunctionIndex, AB);
  return PureAttr;
}

// TODO: Change this to plain sincos.
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
static bool LLVM_ATTRIBUTE_UNUSED combineSinCos(CallInst *Call,
                                                StringRef OCLSinCosName,
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
      AngleType, 0, nullptr, DL.getStackAlignment(),
      "cos.ptr", Call->getFunction()->getEntryBlock().getFirstNonPHI());

  // This stack temp needs to be declared private, if the sin/cos are inside
  // an OMP region.
  VPOUtils::addPrivateToEnclosingRegion(CosTmp, Call->getParent(), DT, false);

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
  auto *CosVal = B.CreateAlignedLoad(AngleType,
                                     CosTmp, DL.getStackAlignment(), "cos.val");
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

namespace llvm {

class MathLibraryFunctionsReplacement {
  DominatorTree &DT;

private:
  SmallVector<Instruction *, 4> DivRemInsts;
  SmallVector<WeakVH, 4> GenSinCosInsts; // sin,cos
  StringRef SinCosName = "sincosf";

  void collectMathInstructions(Function &F);
  bool transformDivRem(Module *M);
  bool transformDivRem(Instruction *I, Module *M);

  using InstOpcode = unsigned;
  using DivRemFnMap = std::unordered_map<InstOpcode, const char *>;

public:
  MathLibraryFunctionsReplacement(DominatorTree &DT) : DT(DT) {}
  static DivRemFnMap DivRemInstFnMap;
  bool run(Function &F, bool isOCL) {
    bool Changed = false;

    // -mf-x86-target option
    if (IsX86Target.getNumOccurrences() > 0)
      isOCL = false;

    // collect all instructions that we may be interested into, into several
    // vectors.
    collectMathInstructions(F);

    if (isOCL)
      Changed |= transformDivRem(F.getParent());
    // TODO: sincos transform goes in "else" branch
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
        if (isSin(Call) || isCos(Call))
          GenSinCosInsts.emplace_back(Call);
      }
    }
  }
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
    // Create function with { nounwind readnone willreturn } attributes.
    Function *FnDecl = cast<Function>(
        M->getOrInsertFunction(DivRemInstFnMap[Inst->getOpcode()], FnSignature,
                               getPureAttr(M->getContext()))
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

    return G.run(F, isOCL);
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
  // Return without any change if instruction to function replacement is
  // disabled.
  if (DisableMFReplacement)
    return PreservedAnalyses::all();

  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  MathLibraryFunctionsReplacement G(DT);
  // Update the last parameter if the new PM is enabled with bisection.
  if (!G.run(F, isOCL))
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
