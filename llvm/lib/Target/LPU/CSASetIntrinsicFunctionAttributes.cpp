//===--- CSADefaultAttributes.cpp - Partially inline libcalls ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass tries to partially inline the fast path of well-known library
// functions, such as using square-root instructions for cases where sqrt()
// does not need to set errno.
//
//===----------------------------------------------------------------------===//

#include "CSASetIntrinsicFunctionAttributes.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "csa-set-intrinsic-func-attribs"

static bool optimizeLpuBuiltin(CallInst *Call, BasicBlock &CurrBB) {
  // There is no need to change the IR, since backend will emit
  // instruction if the call has already been marked read-only.
  if (Call->onlyReadsMemory())
    return false;

  // The call must have the expected result type.
  if (!Call->getType()->isFloatingPointTy())
    return false;

  // If we're compling for the LPU, add attribute "readnone" so that backend
  // can use a native instruction for this call. The "readnone" indicates that
  // the instruction is not modifying any global state. Specifically, it's
  // not modifying errno. If the user has chosen to offload the calculation
  // to the LPU, then they're more concerned with speed than handling odd
  // corner cases.
  if (CurrBB.getModule()->getTargetTriple() == "lpu") {
    Call->addAttribute(AttributeSet::FunctionIndex, Attribute::ReadNone);
    return true;
  }

  return false;
}

static
bool runCSASetIntrinsicFunctionAttributes(Function &F,
                                          TargetLibraryInfo *TLI,
                                          const TargetTransformInfo *TTI) {
  bool Changed = false;

  // Iterate over each instruction in the function
  Function::iterator CurrBB;
  for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE;) {
    CurrBB = BB++;

    for (BasicBlock::iterator II = CurrBB->begin(), IE = CurrBB->end();
         II != IE; ++II) {

      // If this isn't a call, skip the instruction
      CallInst *Call = dyn_cast<CallInst>(&*II);
      Function *CalledFunc;

      if (!Call || !(CalledFunc = Call->getCalledFunction()))
        continue;

      // Skip if function either has local linkage or is not a known library
      // function.
      LibFunc::Func LibFunc;
      if (CalledFunc->hasLocalLinkage() || !CalledFunc->hasName() ||
          !TLI->getLibFunc(CalledFunc->getName(), LibFunc))
        continue;

#if 0 // Attempts to call getIntrinsicInstrCost
      // Clone the call? It's a private function
      CallInst callClone(*Call);

      Intrinsic::ID IID = getIntrinsicForCallSite(callClone, TLI);

      Type *T = Call->getType();
      SmallVector<Type*, 4> Tys;
      FastMathFlags FMF;
      unsigned cost = TTI->getIntrinsicInstrCost(IID,
                                                 T,
                                                 Tys,
                                                 FMF);
      fprintf(stderr, "runCSASetIntrinsicFunctionAttributes - Checking %s - cost: %d\n",
              CalledFunc->getName().str().c_str(), cost);
#endif

      // I'd really love to determine if this is a call which will be turned
      // into an instruction from the information in LPUTargetTransformInfo.cpp.
      // Unfortunately, getIntrinsicInstrCost takes an Intrinsic. I've
      // got a call site, and I haven't found a way to get from the called
      // function to the intrinsic. llvm::getIntrinsicForCallSite() (in
      // Analysis/ValueTracking) *looks* like it would be perfect. Except
      // it's checking whether the call only reads memory, and that's the
      // flag I want to set.
      //
      // Sigh.
      switch (LibFunc) {
      case LibFunc::abs:
      case LibFunc::cos:
      case LibFunc::cosf:
      case LibFunc::exp:
      case LibFunc::expf:
      case LibFunc::exp2:
      case LibFunc::exp2f:
      case LibFunc::log:
      case LibFunc::logf:
      case LibFunc::log2:
      case LibFunc::log2f:
      case LibFunc::sin:
      case LibFunc::sinf:
      case LibFunc::sqrt:
      case LibFunc::sqrtf:
        if (optimizeLpuBuiltin(Call, *CurrBB))
          Changed = true;
        break;

      default:
        break;
      }

    } // for BasicBlock::iterator II

  } // for Function::iterator BB

  return Changed;
}

namespace llvm {
class CSASetIntrinsicFunctionAttributesPass
    : public PassInfoMixin<CSASetIntrinsicFunctionAttributesPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
}

PreservedAnalyses
CSASetIntrinsicFunctionAttributesPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  if (!runCSASetIntrinsicFunctionAttributes(F, &TLI, &TTI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

namespace llvm {
  void initializeCSASetIntrinsicFunctionAttributesLegacyPassPass(PassRegistry &);
} // namespace llvm

namespace {
class CSASetIntrinsicFunctionAttributesLegacyPass : public FunctionPass {
public:
  static char ID;

  CSASetIntrinsicFunctionAttributesLegacyPass() : FunctionPass(ID) {
    initializeCSASetIntrinsicFunctionAttributesLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    TargetLibraryInfo *TLI =
        &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    const TargetTransformInfo *TTI =
        &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    return runCSASetIntrinsicFunctionAttributes(F, TLI, TTI);
  }
};
} // namespace

char CSASetIntrinsicFunctionAttributesLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(CSASetIntrinsicFunctionAttributesLegacyPass,
                      "csa-set-intrinsic-func-attribs",
                      "CSA set intrinsic function attributes",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(CSASetIntrinsicFunctionAttributesLegacyPass,
                    "csa-set-intrinsic-func-attribs",
                    "CSA set intrinsic function attributes",
                    false, false)

namespace llvm {
FunctionPass *createCSASetIntrinsicFunctionAttributesPass() {
  return new CSASetIntrinsicFunctionAttributesLegacyPass();
}
}
