//=---- Intel_LowerSubscriptIntrinsic.cpp - Lower llvm.intel.subscript ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass lowers the 'llvm.intel.subscript' intrinsic to explicit address
// computations.
// For now the pass also removes 'distribute' points.
// TODO: move out the logic to a separate pass or rename current pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_LowerSubscriptIntrinsic.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

static cl::opt<bool> EnableSubscriptLowering(
    "enable-subscript-lowering", cl::Hidden, cl::init(true),
    cl::desc(
        "Enable lowering llvm.intel.subscript lowering to pointer arithmetic"));

static bool lowerIntrinsics(Function &F) {

  if (!EnableSubscriptLowering)
    return false;

  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();

  bool Changed = false;
  for (BasicBlock &BB : F) {
    // Replace llvm.intel.subscript intrinsics.
    for (auto BI = BB.begin(), BE = BB.end(); BI != BE;) {
      Instruction &Inst = *BI++;

      // Remove distribute points
      IntrinsicInst *Intrin = dyn_cast<IntrinsicInst>(&Inst);
      if (Intrin && Intrin->hasOperandBundles()) {
        OperandBundleUse BU = Intrin->getOperandBundleAt(0);

        StringRef TagName = BU.getTagName();
        if (TagName.equals("DIR.PRAGMA.DISTRIBUTE_POINT") ||
            TagName.equals("DIR.PRAGMA.END.DISTRIBUTE_POINT")) {

          Intrin->replaceAllUsesWith(UndefValue::get(Intrin->getType()));
          Intrin->eraseFromParent();
          Changed = true;
          continue;
        }
      }

      SubscriptInst *CI = dyn_cast<SubscriptInst>(&Inst);
      if (!CI)
        continue;

      IRBuilder<> Builder(CI);
      Value *Offset[] = {EmitSubsOffset(&Builder, DL, CI)};
      CI->replaceAllUsesWith(
          Builder.CreateInBoundsGEP(CI->getPointerOperand(), Offset));
      salvageDebugInfo(*CI);
      CI->eraseFromParent();

      Changed = true;
    }
  }
  return Changed;
}

PreservedAnalyses
LowerSubscriptIntrinsicPass::run(Function &M, FunctionAnalysisManager &FM) {
  if (!lowerIntrinsics(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  // PA.preserve<AAManager>();    depends on other analyses.
  // PA.preserve<MemorySSAAnalysis>(); depends on GlobalsAA.
  // PA.preserve<BasicAA>();      done in later patch.
  // PA.preserve<InlineAggAnalysis>(); postponed, very specific.
  // PA.preserve<ScalarEvolutionAnalysis>(); postponed.
  // PA.preserve<DependenceAnalysis>(); postponed.
  // PA.preserve<MemoryDependenceAnalysis>(); postponed.
  // PA.preserve<SCEVAA>(); postponed.
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<AndersensAA>();
  return PA;
}

namespace {

// Contiguous set of GEP's indexes which either array accesses,
// or non-array accesses.
class gep_slice_iterator {
public:
  typedef std::forward_iterator_tag iterator_category;

  typedef iterator_range<gep_type_iterator> value_type;
  typedef const value_type &reference;
  typedef const value_type *pointer;
  typedef gep_slice_iterator iterator;

  iterator &operator++() {
    Slice = make_range(Slice.end(), Slice.end());
    computeSlice();
    return *this;
  }
  iterator operator++(int) {
    iterator retval = *this;
    ++(*this);
    return retval;
  }
  bool operator==(const gep_slice_iterator &It) const {
    return Slice.begin() == It.Slice.begin();
  }
  bool operator!=(const gep_slice_iterator &It) const {
    return !(operator==(It));
  }
  reference operator*() const { return Slice; }
  pointer operator->() const { return &operator*(); }

  bool isSequential() const { return Slice.begin().isSequential(); }
  unsigned getSize() const { return NumberOfIndexes; }

  // Slice starts with special index for pointer.
  bool isFirstSlice() const {
    return isSequential() && !Slice.begin().isBoundedSequential();
  }
  bool isLastSlice() const { return Slice.end() == GlobalEnd; }

  bool areAllArrayIndsZero() const { return AreAllArrayIndsZero; }

  static gep_slice_iterator gep_slice_begin(User *GEP) {
    return gep_slice_iterator(gep_type_begin(GEP), gep_type_end(GEP));
  }
  static gep_slice_iterator gep_slice_end(User *GEP) {
    return gep_slice_iterator(gep_type_end(GEP), gep_type_end(GEP));
  }

protected:
  value_type Slice;
  gep_type_iterator GlobalEnd;

  // Cached values from computeSlice/cacheSeqStats
  int NumberOfIndexes;
  bool AreAllArrayIndsZero;

  gep_slice_iterator(gep_type_iterator Begin, gep_type_iterator GlobalEnd)
      : Slice(Begin, Begin), GlobalEnd(GlobalEnd), AreAllArrayIndsZero(false) {

    computeSlice();
  }

  void computeSlice() {
    auto SliceIt = Slice.end();
    NumberOfIndexes = 0;
    for (; SliceIt != GlobalEnd && SliceIt.isSequential() == isSequential();
         ++SliceIt) {
      NumberOfIndexes++;
    }
    Slice = make_range(Slice.begin(), SliceIt);

    cacheSeqStats();
  }

  void cacheSeqStats() {
    if (!isSequential()) {
      AreAllArrayIndsZero = false;
      return;
    }

    AreAllArrayIndsZero = true;
    for (auto SliceIt = Slice.begin(), End = Slice.end(); SliceIt != End;
         ++SliceIt) {

      Value *Arg = SliceIt.getOperand();
      Constant *Const = dyn_cast<Constant>(Arg);
      if (!Const || !Const->isNullValue()) {
        AreAllArrayIndsZero = false;
        break;
      }
    }
  }
};

template <typename IRBuilderParam>
static Value *convertGEPToSubscript(const DataLayout &DL,
                                    IRBuilder<IRBuilderParam> &Builder,
                                    GEPOperator *GEP) {

  if (!GEP->hasIndices())
    return nullptr;

  Type *PtrIntTy =
      DL.getIntPtrType(GEP->getContext(), GEP->getPointerAddressSpace());

  // Partial result
  Value *Res = GEP->getPointerOperand();

  for (gep_slice_iterator It = gep_slice_iterator::gep_slice_begin(GEP),
                          End = gep_slice_iterator::gep_slice_end(GEP),
                          BaseIt = It;
       It != End; ++It) {

    if (It.isLastSlice()) {
      // No conversion done so far.
      if (Res == GEP->getPointerOperand())
        // Conditions to glue with next slice.
        if (!It.isSequential() || It.areAllArrayIndsZero())
          return nullptr;
    } else {
      // Conditions to glue with next slice.
      if (!It.isSequential() || It.areAllArrayIndsZero())
        continue;
    }

    {
      SmallVector<Value *, 8> StructOffs;
      if (!BaseIt.isFirstSlice())
        StructOffs.push_back(ConstantInt::get(PtrIntTy, 0));

      for (; BaseIt != It; ++BaseIt)
        for (auto OpIt = BaseIt->begin(), End = BaseIt->end(); OpIt != End;
             ++OpIt)
          StructOffs.push_back(OpIt.getOperand());

      if (It.isSequential())
        for (auto OpIt = It->begin(), End = It->end(); OpIt != End; ++OpIt)
          StructOffs.push_back(ConstantInt::get(PtrIntTy, 0));
      else
        for (auto OpIt = It->begin(), End = It->end(); OpIt != End; ++OpIt)
          StructOffs.push_back(OpIt.getOperand());

      // Minor optimization: "gep base, 0" == base
      if (StructOffs.size() != 1 || !isa<ConstantInt>(StructOffs[0]) ||
          !cast<ConstantInt>(StructOffs[0])->isZero())
        Res = Builder.CreateInBoundsGEP(Res, StructOffs);
      // Reset BaseIt after new GEP generated.
      BaseIt = It;
      ++BaseIt;
    }

    if (It.isSequential()) {
      unsigned Rank = It.getSize();
      // Processing one array index at a time.
      for (auto TypeIt = It->begin(), End = It->end(); TypeIt != End;
           ++TypeIt, --Rank)
        Res = Builder.CreateSubscript(
            Rank - 1, ConstantInt::get(PtrIntTy, 0),
            ConstantInt::get(PtrIntTy,
                             DL.getTypeStoreSize(TypeIt.getIndexedType())),
            Res, TypeIt.getOperand());
    }
  }

  return Res;
}

} // namespace

// See recursion scheme in LangRef.rst.
// NB: First 0th index is not optimized off conversion of array accesses.
bool ConvertGEPToSubscriptIntrinsicPass::convertGEPToSubscriptIntrinsic(
    const DataLayout &DL, GetElementPtrInst *GEP, bool Unlink) {

  IRBuilder<> Builder(GEP);
  if (Value *Replacement =
          convertGEPToSubscript(DL, Builder, cast<GEPOperator>(GEP))) {
    Replacement->takeName(GEP);
    GEP->replaceAllUsesWith(Replacement);
  } else
    return false;

  if (Unlink)
    GEP->eraseFromParent();
  return true;
}

bool ConvertGEPToSubscriptIntrinsicPass::convertGEPToSubscriptIntrinsic(
    const DataLayout &DL, Instruction *Inst, Use *GEPUse) {

  IRBuilder<> Builder(isa<PHINode>(Inst) ? &*cast<PHINode>(Inst)
                                                 ->getIncomingBlock(*GEPUse)
                                                 ->getFirstInsertionPt()
                                         : Inst);
  if (Value *Replacement = convertGEPToSubscript(
          DL, Builder, cast<GEPOperator>(GEPUse->get()))) {
    GEPUse->set(Replacement);
    return true;
  }
  return false;
}

static bool convertToIntrinsics(Function &F) {

  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();

  bool Changed = false;
  for (BasicBlock &BB : F) {
    // Replace llvm.intel.subscript intrinsics.
    for (auto BI = BB.begin(), BE = BB.end(); BI != BE;) {
      Instruction &Inst = *BI++;
      for (Use &Use : Inst.operands()) {
        if (isa<GEPOperator>(Use.get()) && isa<ConstantExpr>(Use.get())) {
          if (ConvertGEPToSubscriptIntrinsicPass::
                  convertGEPToSubscriptIntrinsic(DL, &Inst, &Use)) {
            Changed = true;
          }
        }
      }
      GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&Inst);
      if (!GEP) {
        continue;
      }
      if (ConvertGEPToSubscriptIntrinsicPass::convertGEPToSubscriptIntrinsic(
              DL, GEP)) {
        Changed = true;
      }
    }
  }
  return Changed;
}

PreservedAnalyses
ConvertGEPToSubscriptIntrinsicPass::run(Function &M,
                                        FunctionAnalysisManager &FM) {
  if (!convertToIntrinsics(M))
    return PreservedAnalyses::all();
  else
    return PreservedAnalyses::none();
}

namespace {
class LowerSubscriptIntrinsicLegacyPass : public FunctionPass {
public:
  static char ID;

  LowerSubscriptIntrinsicLegacyPass() : FunctionPass(ID) {
    initializeLowerSubscriptIntrinsicLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<WholeProgramWrapperPass>();

    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
  }

  bool runOnFunction(Function &F) override { return lowerIntrinsics(F); }
};
} // end anonymous namespace

char LowerSubscriptIntrinsicLegacyPass::ID = 0;

INITIALIZE_PASS(LowerSubscriptIntrinsicLegacyPass, "lower-subscript",
                "Subscript Intrinsic Lowering", false, false)

FunctionPass *llvm::createLowerSubscriptIntrinsicLegacyPass() {
  return new LowerSubscriptIntrinsicLegacyPass();
}

namespace {
class ConvertGEPToSubscriptIntrinsicLegacyPass : public FunctionPass {
public:
  static char ID;

  ConvertGEPToSubscriptIntrinsicLegacyPass() : FunctionPass(ID) {
    initializeConvertGEPToSubscriptIntrinsicLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    return convertToIntrinsics(F);
  }
};
} // end anonymous namespace

char ConvertGEPToSubscriptIntrinsicLegacyPass::ID = 0;

INITIALIZE_PASS(ConvertGEPToSubscriptIntrinsicLegacyPass,
                "convert-to-subscript", "GEP to Subscript conversion", false,
                false)

FunctionPass *llvm::createConvertGEPToSubscriptIntrinsicLegacyPass() {
  return new ConvertGEPToSubscriptIntrinsicLegacyPass();
}
