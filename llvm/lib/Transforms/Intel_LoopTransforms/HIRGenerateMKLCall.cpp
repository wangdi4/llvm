//===----- HIRGenerateMKLCall.cpp - Implements MKL Call transformation ----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIR MKL Call transformation, which generates mkl call
// for matrix multiplications.
//
// MKL call supports 2X2 and 2X1 matrix multiplications.
// Data types supported: fp, double, complex*16, complex*32.
// Matrix multiplications embedded in loops will be recognized.
// This pass runs after loop interchange and additional pattern recognition is
// needed when interchange does not get kicked in.
//
// After the pattern matching of matrix multiplication codes, dope vectors are
// populated with values from the memory references, tripcounts, strides etc.
// and a call to mkl library is generated with dope vectors as parameter. Last
// two parameters are data type code and reset field. A general description of
// dope vectors can be found in wiki: https://en.wikipedia.org/wiki/Dope_vector
// In MKL calls Each dope vector contains the following fields:
//   0. Base address: a pointer to the location in memory where the array
//   elements begin
//   1. Address length: length of the array address
//   2. Offset: offset from the base pointer
//   3. Flags:
//   4. Rank: the rank of an array
//   5. Reserved:
// For each of the dimension these three fields are repeated-
//   6. Extent: the extent of an array (its range of indices)
//   7. Stride: the stride of an array, or the amount of memory occupied by each
//   element of the array
//   8. Lower bound: lower bound of the array indices
//
// For example:
//
//   Original Loop
//   Do I = 1..500
//      Do J = 1..500
//         Do K = 1..100
//            C(I,J) = C(I,J) + A(I,K) * B(K,J)
//
//   Transformed Loop after generating mkl calls
//   (%.DopeVector1)[0].0 = &((i8*)(@c)[0][0][0]);
//   (%.DopeVector1)[0].1 = 4;
//   (%.DopeVector1)[0].2 = 0;
//   (%.DopeVector1)[0].3 = 0;
//   (%.DopeVector1)[0].4 = 2;
//   (%.DopeVector1)[0].5 = 0;
//   (%.DopeVector1)[0].6 = 500;
//   (%.DopeVector1)[0].7 = 4;
//   (%.DopeVector1)[0].8 = 1;
//   (%.DopeVector1)[0].9 = 500;
//   (%.DopeVector1)[0].10 = 2000;
//   (%.DopeVector1)[0].11 = 1;
//   (%.DopeVector2)[0].0 = &((i8*)(@b)[0][0][0]);
//   (%.DopeVector2)[0].1 = 4;
//   (%.DopeVector2)[0].2 = 0;
//   (%.DopeVector2)[0].3 = 0;
//   (%.DopeVector2)[0].4 = 2;
//   (%.DopeVector2)[0].5 = 0;
//   (%.DopeVector2)[0].6 = 500;
//   (%.DopeVector2)[0].7 = 4;
//   (%.DopeVector2)[0].8 = 1;
//   (%.DopeVector2)[0].9 = 100;
//   (%.DopeVector2)[0].10 = 2000;
//   (%.DopeVector2)[0].11 = 1;
//   (%.DopeVector3)[0].0 = &((i8*)(@a)[0][0][0]);
//   (%.DopeVector3)[0].1 = 4;
//   (%.DopeVector3)[0].2 = 0;
//   (%.DopeVector3)[0].3 = 0;
//   (%.DopeVector3)[0].4 = 2;
//   (%.DopeVector3)[0].5 = 0;
//   (%.DopeVector3)[0].6 = 100;
//   (%.DopeVector3)[0].7 = 4;
//   (%.DopeVector3)[0].8 = 1;
//   (%.DopeVector3)[0].9 = 500;
//   (%.DopeVector3)[0].10 = 400;
//   (%.DopeVector3)[0].11 = 1;
//   @matmul_mkl_f32_(&((%.DopeVector1)[0]),  &((%.DopeVector2)[0]),
//                    &((%.DopeVector3)[0]),  9,  0);

#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRGenerateMKLCall.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-generate-mkl-call"
#define OPT_DESC "HIR Generate MKL Call"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

const unsigned MinTripCountThreshold = 3;

static cl::opt<unsigned> TripCountThreshold(
    OPT_SWITCH "-hir-mkl-matmul-tripcount-threshold",
    cl::init(MinTripCountThreshold), cl::Hidden,
    cl::desc("Innermost loop minimum TripCount to enable " OPT_DESC));

namespace {
typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

class HIRGenerateMKLCall {
public:
  HIRGenerateMKLCall(HIRFramework &HIRF, HIRLoopStatistics &HLS)
      : HIRF(HIRF), HLS(HLS) {}

  bool run();
  StructType *DopeVectorType;

private:
  HIRFramework &HIRF;
  HIRLoopStatistics &HLS;
  struct CollectCandidateLoops;

  bool generateMKLCall(LLVMContext &Context);
  bool isMatmul(HLLoop *, SmallVector<const RegDDRef *, 3> &MatrixRefs,
                SmallVector<const RegDDRef *, 3> &TripCountDDRefs,
                bool *IsZeroSet) const;
  bool isComplexMatmul(HLLoop *, SmallVector<const RegDDRef *, 3> &MatrixRefs,
                       SmallVector<const RegDDRef *, 3> &TripCountDDRefs,
                       bool *IsZeroSet) const;
  bool isMatVecMul(HLLoop *, SmallVector<const RegDDRef *, 3> &MatrixRefs,
                   SmallVector<const RegDDRef *, 3> &TripCountDDRefs,
                   bool *IsZeroSet) const;
  void createDopeVectorType(LLVMContext &Context, Type *IntType);
  void createDopeVectorAssignmentsForDim(HLLoop *Loop, unsigned IVLevel,
                                         const RegDDRef *RRef,
                                         const RegDDRef *TripCountDDRef,
                                         unsigned AllocaBlobIdx,
                                         unsigned &DopeVectorFieldIdx);

  RegDDRef *
  createDopeVectorAssignments(HLLoop *Loop, const RegDDRef *RRef,
                              SmallVector<const RegDDRef *, 3> &TripCountDDRefs,
                              bool IsComplexType,
                              SmallVector<unsigned, 4> &OrderedLevelsToProc);
  void computeDopeVectorFieldsAndTransform(
      LLVMContext &Context, HLLoop *,
      SmallVector<const RegDDRef *, 3> &MatrixRefs,
      SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool IsZeroSet,
      bool IsComplexType);
};
} // namespace

namespace {

class HIRGenerateMKLCallLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRGenerateMKLCallLegacyPass() : HIRTransformPass(ID) {
    initializeHIRGenerateMKLCallLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &Func) override;
  void releaseMemory() override{};

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

} // namespace

char HIRGenerateMKLCallLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRGenerateMKLCallLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRGenerateMKLCallLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRGenerateMKLCallPass() {
  return new HIRGenerateMKLCallLegacyPass();
}

PreservedAnalyses HIRGenerateMKLCallPass::run(Function &Func,
                                              FunctionAnalysisManager &AM) {
  HIRGenerateMKLCall(AM.getResult<HIRFrameworkAnalysis>(Func),
                     AM.getResult<HIRLoopStatisticsAnalysis>(Func))
      .run();
  return PreservedAnalyses::all();
}

bool HIRGenerateMKLCall::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : "
                    << HIRF.getFunction().getName() << "\n");

  return generateMKLCall(HIRF.getFunction().getContext());
}

bool HIRGenerateMKLCallLegacyPass::runOnFunction(Function &Func) {
  if (skipFunction(Func)) {
    return false;
  }

  return HIRGenerateMKLCall(
             getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
             getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
      .run();
}

// Gather all perfect/near-perfect Loop Nests with level 2/3
struct HIRGenerateMKLCall::CollectCandidateLoops final
    : public HLNodeVisitorBase {

  HIRGenerateMKLCall &GMKLCall;
  SmallVectorImpl<HLLoop *> &CandidateLoops;
  HLNode *SkipNode;

  CollectCandidateLoops(HIRGenerateMKLCall &GenerateMKLCall,
                        SmallVectorImpl<HLLoop *> &CandidateLoops)
      : GMKLCall(GenerateMKLCall), CandidateLoops(CandidateLoops),
        SkipNode(nullptr) {}

  void visit(HLLoop *Loop) {
    const HLLoop *InnermostLoop = nullptr;

    if (Loop->isInnermost()) {
      SkipNode = Loop;
      return;
    }
    LLVM_DEBUG(dbgs() << "In collect Perfect/Near-Perfect loopnest\n");

    // Allow Near Perfect loop (and return the result).
    bool IsNearPerfectLoop = false;
    bool IsPerfectNest = HLNodeUtils::isPerfectLoopNest(
        Loop, &InnermostLoop, false, &IsNearPerfectLoop);
    assert((!IsPerfectNest || !IsNearPerfectLoop) &&
           "isPerfectLoopNest is malfunctioning");

    if (!IsPerfectNest && !IsNearPerfectLoop) {
      // Do not skip recursion.
      // We might find a perfect loop nest starting from an inner loop.
      return;
    }

    if (GMKLCall.HLS.getSelfLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects()) {
      LLVM_DEBUG(
          dbgs() << "\nSkipping loop with calls that have side effects\n");
      SkipNode = Loop;
      return;
    }

    uint64_t TripCount = -1;
    if (InnermostLoop->isConstTripLoop(&TripCount) &&
        TripCount < TripCountThreshold) {
      LLVM_DEBUG(
          dbgs() << "\nSkipping loop with calls with small trip Count\n");
      SkipNode = Loop;
      return;
    }

    if (InnermostLoop->getNestingLevel() - Loop->getNestingLevel() > 2) {
      return;
    }

    for (const HLLoop *TmpLoop = InnermostLoop,
                      *EndLoop = Loop->getParentLoop();
         TmpLoop != EndLoop; TmpLoop = TmpLoop->getParentLoop()) {
      if (TmpLoop->hasUnrollEnablingPragma() ||
          TmpLoop->hasUnrollAndJamEnablingPragma() ||
          TmpLoop->hasVectorizeEnablingPragma()) {
        LLVM_DEBUG(dbgs() << "\nSkipping loop with unroll/vector pragma\n");
        SkipNode = Loop;
        return;
      }
    }

    CandidateLoops.push_back(Loop);
    SkipNode = Loop;
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
};

bool HIRGenerateMKLCall::generateMKLCall(LLVMContext &Context) {
  bool Modified = false;
  SmallVector<HLLoop *, 12> CandidateLoops;

  // Walk all loops, look for outer loops that are perfectly or near perfectly
  // nested
  CollectCandidateLoops CCL(*this, CandidateLoops);
  HIRF.getHLNodeUtils().visitAll(CCL);
  LoopOptReportBuilder &LORBuilder = HIRF.getLORBuilder();

  for (auto *Loop : CandidateLoops) {

    SmallVector<const RegDDRef *, 3> MatrixRefs;
    SmallVector<const RegDDRef *, 3> TripCountDDRefs;
    bool MKLCallGenerated = false;
    bool IsZeroSet = false;

    if (isMatmul(Loop, MatrixRefs, TripCountDDRefs, &IsZeroSet)) {
      LLVM_DEBUG(dbgs() << "\nLoop is a matmul\n"; Loop->dump());
      computeDopeVectorFieldsAndTransform(Context, Loop, MatrixRefs,
                                          TripCountDDRefs, IsZeroSet, false);
      MKLCallGenerated = true;
    } else if (isMatVecMul(Loop, MatrixRefs, TripCountDDRefs, &IsZeroSet)) {
      LLVM_DEBUG(dbgs() << "\nLoop is a vecmul\n"; Loop->dump());
      computeDopeVectorFieldsAndTransform(Context, Loop, MatrixRefs,
                                          TripCountDDRefs, IsZeroSet, false);
      MKLCallGenerated = true;
    } else if (isComplexMatmul(Loop, MatrixRefs, TripCountDDRefs, &IsZeroSet)) {
      LLVM_DEBUG(dbgs() << "\nLoop is a complex matmul\n"; Loop->dump());
      computeDopeVectorFieldsAndTransform(Context, Loop, MatrixRefs,
                                          TripCountDDRefs, IsZeroSet, true);
      MKLCallGenerated = true;
    } else {
      // matmul can be embedded inside other loops
      // Example.
      //  for (int l = 0; l < 100; l++) {
      //    for (int i = 0; i < N; i++) {
      //      for (int j = 0; j < N; j++) {
      //        c[i] += a[i][j] * b[j];
      //      }
      //    }
      //  }
      // For this case we go to the next loop and try pattern match it
      auto Child = Loop->child_begin();
      HLLoop *ChildLoop = nullptr;
      while (!ChildLoop) {
        ChildLoop = dyn_cast<HLLoop>(&*Child);
        Child++;
      }

      if (isMatVecMul(ChildLoop, MatrixRefs, TripCountDDRefs, &IsZeroSet)) {
        LLVM_DEBUG(dbgs() << "\nLoop is a vecmul\n"; Loop->dump());
        computeDopeVectorFieldsAndTransform(Context, ChildLoop, MatrixRefs,
                                            TripCountDDRefs, IsZeroSet, false);
        MKLCallGenerated = true;
      }
    }
    if (MKLCallGenerated) {
      LORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                                  "MKL call generated.");
    }

    Modified |= MKLCallGenerated;
  }

  return Modified;
}

static StringRef getFuncName(Type *Ty, bool IsComplexType) {
  if (IsComplexType) {
    if (Ty->getTypeID() == Type::FloatTyID) {
      return "matmul_mkl_c64_";
    } else {
      assert((Ty->getTypeID() == Type::DoubleTyID) &&
             "Unhandled data type in gen_matmul");
      return "matmul_mkl_c128_";
    }
  } else if (Ty->getTypeID() == Type::FloatTyID) {
    return "matmul_mkl_f32_";
  }
  assert((Ty->getTypeID() == Type::DoubleTyID) &&
         "Unhandled data type in gen_matmul");
  return "matmul_mkl_f64_";
}

static int getEncodingForType(Type *Ty, bool IsComplexType) {
  if (IsComplexType) {
    if (Ty->getTypeID() == Type::FloatTyID) {
      return 28;
    } else {
      assert((Ty->getTypeID() == Type::DoubleTyID) &&
             "Unhandled data type in gen_matmul");
      return 29;
    }
  } else if (Ty->getTypeID() == Type::FloatTyID) {
    return 9;
  }
  assert((Ty->getTypeID() == Type::DoubleTyID) &&
         "Unhandled data type in gen_matmul");
  return 10;
}

// Return the iv levels in some particular dimensions
SmallVector<unsigned, 4> getIVLevelsAtDim(const RegDDRef *RRef, unsigned Dim) {
  SmallVector<unsigned, 4> Levels;
  auto *CE = RRef->getDimensionIndex(Dim);
  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {
    if (CE->hasIV(CE->getLevel(CurIVPair))) {
      Levels.push_back(CE->getLevel(CurIVPair));
    }
  }
  return Levels;
}

// Return the iv levels present in all dimensions
SmallVector<unsigned, 4> getIVLevels(const RegDDRef *RRef) {
  SmallVector<unsigned, 4> Levels;
  unsigned NumDims = RRef->getNumDimensions();
  for (unsigned Dim = 1; Dim <= NumDims; Dim++) {
    SmallVector<unsigned, 4> LevelsAtDim = getIVLevelsAtDim(RRef, Dim);
    Levels.insert(Levels.end(), LevelsAtDim.begin(), LevelsAtDim.end());
  }
  return Levels;
}

// Check whether all dimensions except the highest contain some iv or not
bool checkIV(const RegDDRef *RRef) {
  unsigned NumDims = RRef->getNumDimensions();
  LLVM_DEBUG(dbgs() << "\nNumDims: " << NumDims << "\n");

  for (unsigned Dim = 1; Dim < NumDims; Dim++) {
    auto *CE = RRef->getDimensionIndex(Dim);
    if (!CE->hasIV()) {
      return false;
    }
    LLVM_DEBUG(dbgs() << "has IV at dim: " << Dim << "\n");
  }

  return true;
}

// Create the dope vector structure type with required fields
void HIRGenerateMKLCall::createDopeVectorType(LLVMContext &Context,
                                              Type *IntType) {
  Type *AddrType = Type::getInt8PtrTy(Context);
  constexpr unsigned int max_dope_vector_fields = 12;
  SmallVector<Type *, max_dope_vector_fields> DopeVectorFields = {
      AddrType, // ".addr_a0"
      IntType,  // ".addr_length"
      IntType,  // ".offset"
      IntType,  // ".flags"
      IntType,  // ".rank"
      IntType,  // ".reserved"
      IntType,  // ".extent0"
      IntType,  // ".stride0"
      IntType,  // ".lower0"
      IntType,  // ".extent1"
      IntType,  // ".stride1"
      IntType,  // ".lower1"
  };

  DopeVectorType = StructType::get(Context, DopeVectorFields);
}

// Add live-in temp in the loop
static void updateLiveInAllocaTemp(HLLoop *Loop, unsigned SB) {
  HLLoop *Lp = Loop;
  while (Lp) {
    Lp->addLiveInTemp(SB);
    Lp = Lp->getParentLoop();
  }
}

void HIRGenerateMKLCall::createDopeVectorAssignmentsForDim(
    HLLoop *Loop, unsigned IVLevel, const RegDDRef *MatrixRef,
    const RegDDRef *TripCountDDRef, unsigned AllocaBlobIdx,
    unsigned &DopeVectorFieldIdx) {
  auto &HNU = Loop->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();
  auto &CEU = DDRU.getCanonExprUtils();
  LLVMContext &Context = HNU.getContext();
  Type *IntType =
      Type::getIntNTy(Context, CEU.getTypeSizeInBits(MatrixRef->getBaseType()));
  unsigned Level = Loop->getNestingLevel() - 1;

  // Compute 'extent' field for current dimension and create the
  // assignment instruction like-
  // (%.DopeVector1)[0].6 = 500;
  RegDDRef *ExtentRef = TripCountDDRef->clone();
  ExtentRef->makeConsistent({}, Level);
  RegDDRef *DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  auto FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);
  auto StoreInst = HNU.createStore(ExtentRef, ".extent", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'stride' field for current dimension and create the
  // assignment instruction
  // (%.DopeVector1)[0].7 = 4;
  auto *StrideCanon = MatrixRef->getStrideAtLevel(IVLevel);
  RegDDRef *StrideCanonRef =
      DDRU.createScalarRegDDRef(GenericRvalSymbase, StrideCanon);
  StrideCanonRef->makeConsistent(MatrixRef, Level);
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);
  StoreInst = HNU.createStore(StrideCanonRef, ".stride", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'lower' field for current dimension and create the assignment
  // instruction
  // (%.DopeVector1)[0].8 = 1;
  RegDDRef *LowerRef = DDRU.createConstDDRef(IntType, 1);
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);
  StoreInst = HNU.createStore(LowerRef, ".lower", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");
}

RegDDRef *HIRGenerateMKLCall::createDopeVectorAssignments(
    HLLoop *Loop, const RegDDRef *MatrixRef,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool IsComplexType,
    SmallVector<unsigned, 4> &OrderedLevelsToProc) {
  auto RegionNode = Loop->getParentRegion();
  auto &HNU = Loop->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();
  auto &CEU = DDRU.getCanonExprUtils();
  LLVMContext &Context = HNU.getContext();
  Type *IntType =
      Type::getIntNTy(Context, CEU.getTypeSizeInBits(MatrixRef->getBaseType()));

  Type *AddrType = Type::getInt8PtrTy(Context);
  unsigned Level = Loop->getNestingLevel() - 1;

  unsigned AllocaBlobIdx =
      HNU.createAlloca(DopeVectorType, RegionNode, ".DopeVector");
  unsigned DopeVectorFieldIdx = 0;

  // Compute 'addr_a0' field and create the assignment instruction
  // (%.DopeVector1)[0].0 = &((i8*)(@c)[0][0][0]);
  RegDDRef *DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  CanonExpr *FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto *BasePtrRef = MatrixRef->clone();
  BasePtrRef->setAddressOf(true);
  BasePtrRef->setBitCastDestType(AddrType);
  BasePtrRef->getDimensionIndex(1)->clear();
  BasePtrRef->getDimensionIndex(1)->setConstant(
      MatrixRef->getDimensionIndex(1)->getConstant());

  if (BasePtrRef->getNumDimensions() > 1) {
    BasePtrRef->getDimensionIndex(2)->clear();
    BasePtrRef->getDimensionIndex(2)->setConstant(
        MatrixRef->getDimensionIndex(2)->getConstant());
  }
  BasePtrRef->makeConsistent(MatrixRef, Level);
  HLInst *StoreInst = HNU.createStore(BasePtrRef, ".addr_a0", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump());

  // Compute 'addr_length' field and create the assignment instruction
  // (%.DopeVector1)[0].1 = 4;
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  unsigned AddrLength =
      MatrixRef->getDestTypeSizeInBytes() * (IsComplexType ? 2 : 1);
  auto AddrLengthRef = DDRU.createConstDDRef(IntType, AddrLength);
  StoreInst = HNU.createStore(AddrLengthRef, ".addr_length", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump());

  // Compute 'offset' field and create the assignment instruction
  // (%.DopeVector1)[0].2 = 0;
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto OffsetRef = DDRU.createConstDDRef(IntType, 0);
  StoreInst = HNU.createStore(OffsetRef, ".offset", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'flags' field and create the assignment instruction
  // (%.DopeVector1)[0].3 = 0;
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto FlagsRef = DDRU.createConstDDRef(IntType, 0);
  StoreInst = HNU.createStore(FlagsRef, ".flags", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'rank' field and create the assignment instruction
  // (%.DopeVector1)[0].4 = 2;
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  SmallVector<unsigned, 4> LevelsAtDim = getIVLevelsAtDim(MatrixRef, 1);
  auto DimNumRef =
      DDRU.createConstDDRef(IntType, std::max(MatrixRef->getNumDimensions() - 1,
                                              (unsigned)LevelsAtDim.size()));
  StoreInst = HNU.createStore(DimNumRef, ".rank", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'reserved' field and create the assignment instruction
  // (%.DopeVector1)[0].5 = 0;
  DopeVectorRef = DDRU.createMemRef(AllocaBlobIdx);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto ReservedRef = DDRU.createConstDDRef(IntType, 0);
  StoreInst = HNU.createStore(ReservedRef, ".reserved", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  unsigned DimNum = 1;
  for (auto CEI = MatrixRef->canon_begin(), CEE = MatrixRef->canon_end();
       CEI != CEE; ++CEI, ++DimNum) {

    auto CE = *CEI;
    if (!CE->hasIV()) {
      continue;
    }

    unsigned MinLevel = OrderedLevelsToProc[0];
    for (unsigned I = 0; I < OrderedLevelsToProc.size(); I++) {
      if (MinLevel > OrderedLevelsToProc[I]) {
        MinLevel = OrderedLevelsToProc[I];
      }
    }
    LevelsAtDim = getIVLevelsAtDim(MatrixRef, DimNum);
    // We have to process the iv levels in the particular order given by
    // OrderedLevelsToProc
    for (unsigned I = 0; I < OrderedLevelsToProc.size(); I++) {
      unsigned IVLevel = OrderedLevelsToProc[I];
      // Skip the iv if it is not present in the current dimension
      // For example, if the order is [1, 2, 3] and current dimension has iv 1
      // and 3, we should skip 2
      if (std::find(LevelsAtDim.begin(), LevelsAtDim.end(), IVLevel) ==
          LevelsAtDim.end()) {
        continue;
      }
      createDopeVectorAssignmentsForDim(Loop, IVLevel, MatrixRef,
                                        TripCountDDRefs[IVLevel - MinLevel],
                                        AllocaBlobIdx, DopeVectorFieldIdx);
    }
  }
  // Create the dope vector field reference and make it live-in to loop
  RegDDRef *DopeRef = DDRU.createMemRef(AllocaBlobIdx);
  auto ZeroCE = CEU.createCanonExpr(IntType);
  DopeRef->setAddressOf(true);
  DopeRef->addDimension(ZeroCE);

  if (auto Lp = Loop->getParentLoop()) {
    updateLiveInAllocaTemp(Lp, DopeRef->getBasePtrSymbase());
  }
  return DopeRef;
}

// Compute the dope vector fields from the mem refs and do the transformation
void HIRGenerateMKLCall::computeDopeVectorFieldsAndTransform(
    LLVMContext &Context, HLLoop *Loop,
    SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool IsZeroSet,
    bool IsComplexType) {
  LLVM_DEBUG(dbgs() << "Computing dope vector fields...\n");
  auto RegionNode = Loop->getParentRegion();
  auto &HNU = Loop->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();
  auto &Module = DDRU.getModule();
  auto &CEU = DDRU.getCanonExprUtils();

  Type *IntType = Type::getIntNTy(
      Context, CEU.getTypeSizeInBits(MatrixRefs[0]->getBaseType()));
  createDopeVectorType(Context, IntType);

  SmallVector<RegDDRef *, 8> CallArgs;
  SmallVector<unsigned, 4> LevelsAtStoreRef = getIVLevels(MatrixRefs[0]);
  SmallVector<unsigned, 4> LevelsAtLoadRef1 = getIVLevels(MatrixRefs[1]);
  SmallVector<unsigned, 4> LevelsAtLoadRef2 = getIVLevels(MatrixRefs[2]);
  SmallVector<unsigned, 4> OrderedLevelsToProc;

  OrderedLevelsToProc.push_back(LevelsAtStoreRef[0]);
  if (LevelsAtStoreRef.size() == 2) {
    // The iv at the second dimension of the store ref should be processed first
    // and the iv at first dimension should go last
    OrderedLevelsToProc.insert(OrderedLevelsToProc.begin(),
                               0); // Dummy place holder
    OrderedLevelsToProc.insert(OrderedLevelsToProc.begin(),
                               LevelsAtStoreRef[1]);
    for (unsigned Level : LevelsAtLoadRef1) {
      if (Level != OrderedLevelsToProc[0] && Level != OrderedLevelsToProc[2]) {
        OrderedLevelsToProc[1] = Level;
        break;
      }
    }
    for (unsigned Level : LevelsAtLoadRef2) {
      if (Level != OrderedLevelsToProc[0] && Level != OrderedLevelsToProc[2]) {
        OrderedLevelsToProc[1] = Level;
        break;
      }
    }
  } else {
    // If store ref contains only one iv
    // That iv at the store ref should be processed first
    OrderedLevelsToProc.push_back(0); // Dummy place holder
    for (unsigned Level : LevelsAtLoadRef1) {
      if (Level != OrderedLevelsToProc[0]) {
        OrderedLevelsToProc[1] = Level;
        break;
      }
    }
    for (unsigned Level : LevelsAtLoadRef2) {
      if (Level != OrderedLevelsToProc[0]) {
        OrderedLevelsToProc[1] = Level;
        break;
      }
    }
  }

  // Process each of the mem refs and create dope vector structure for each
  for (int i = 0; i < 3; i++) {
    RegDDRef *DopeRef =
        createDopeVectorAssignments(Loop, MatrixRefs[i], TripCountDDRefs,
                                    IsComplexType, OrderedLevelsToProc);
    // Collect the dope vector ref as argument
    CallArgs.push_back(DopeRef);
  }

  // Populate the argument vector with data type info and reset field
  CallArgs.push_back(DDRU.createConstDDRef(
      IntType,
      getEncodingForType(MatrixRefs[0]->getDestType(), IsComplexType)));
  CallArgs.push_back(DDRU.createConstDDRef(IntType, (IsZeroSet ? 0 : 1)));

  // Create the mkl call instruction like-
  // @matmul_mkl_c64_(&((%.DopeVector)[0]),  &((%.DopeVector1515)[0]),
  // &((%.DopeVector1516)[0]),  28,  1);
  auto DopeVectorPointerTy = DopeVectorType->getPointerTo();
  Type *Tys[] = {DopeVectorPointerTy, DopeVectorPointerTy, DopeVectorPointerTy,
                 IntType, IntType};
  FunctionType *pNewFuncType =
      FunctionType::get(Type::getVoidTy(Context), Tys, false);
  FunctionCallee MKLFuncCallee = Module.getOrInsertFunction(
      getFuncName(MatrixRefs[0]->getDestType(), IsComplexType).str(),
      pNewFuncType);
  Function *MKLFunc = cast<Function>(MKLFuncCallee.getCallee());
  auto CallInst = HNU.createCall(MKLFunc, CallArgs, MKLFunc->getName());
  HLNodeUtils::insertBefore(Loop, CallInst);
  LLVM_DEBUG(CallInst->dump(); dbgs() << "\n");

  // Update the loop body and parent
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
      Loop);
  if (auto Lp = Loop->getParentLoop()) {
    HLS.markLoopBodyModified(Lp);
  }
  HIRInvalidationUtils::invalidateBody(Loop);
  RegionNode->setGenCode();
  HLNodeUtils::remove(Loop);
}

// Matches instructions like these-
// %mul = (@a)[0][i1][i2]  *  (@b)[0][i2][i3]
// %mul = %50  *  (@b)[0][i2][i3]
// And collect only the memrefs through parameters
static bool matchMultiplication(const HLInst *Inst, const RegDDRef **RefA,
                                const RegDDRef **RefB) {
  if (!Inst) {
    return false;
  }
  LLVM_DEBUG(Inst->dump());
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FMul) {
    LLVM_DEBUG(dbgs() << "Opcode FMul not found.\n");
    return false;
  }

  const RegDDRef *OperandRef1 = Inst->getOperandDDRef(1);
  const RegDDRef *OperandRef2 = Inst->getOperandDDRef(2);

  if (!checkIV(OperandRef1) || !checkIV(OperandRef2)) {
    return false;
  }

  // Save only the memrefs
  if (!OperandRef1->isTerminalRef()) {
    *RefA = OperandRef1;
  }

  if (!OperandRef2->isTerminalRef()) {
    *RefB = OperandRef2;
  }

  return true;
}

// Matches instructions like these-
// %add120 = %add120  +  %mul;
// %add120 = (@c)[0][i1][i2]  +  %mul;
static bool matchAddition(const HLInst *Inst, const RegDDRef **LoadRef) {
  if (!Inst) {
    return false;
  }
  LLVM_DEBUG(Inst->dump());
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FAdd) {
    LLVM_DEBUG(dbgs() << "Opcode FAdd not found.\n");
    return false;
  }

  auto *RefA = Inst->getOperandDDRef(1);
  auto *RefB = Inst->getOperandDDRef(2);

  if (*LoadRef != nullptr && (RefA->isMemRef() || RefB->isMemRef())) {
    LLVM_DEBUG(dbgs() << "Multiple load at addition is present.\n");
    return false;
  }

  if (RefA->isMemRef()) {
    *LoadRef = RefA;
  }

  return true;
}

// Matches instructions like- (@c)[0][i1] = %add120;
// And checks that the store ref with load ref are equal
static bool checkStoreInstruction(HLInst *Inst, const RegDDRef *LoadRef,
                                  const RegDDRef **StoreRef) {
  if (!Inst || !isa<StoreInst>(Inst->getLLVMInstruction())) {
    LLVM_DEBUG(dbgs() << "Store instruction not found.\n");
    return false;
  }
  LLVM_DEBUG(Inst->dump());

  *StoreRef = Inst->getLvalDDRef();
  if (LoadRef && !DDRefUtils::areEqual(LoadRef, *StoreRef)) {
    LLVM_DEBUG(dbgs() << "Load and store location mismatch.\n");
    return false;
  }
  return true;
}

// Matches instructions like- %mul = %.unpack1317  *  %.unpack1321;
bool static matchesTempMul(HLInst *Inst, RegDDRef **LvalTmp, RegDDRef *RvalTmp1,
                           RegDDRef *RvalTmp2) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FMul) {
    LLVM_DEBUG(dbgs() << "Opcode FMul not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!FirstOperand->isSelfBlob() || !SecondOperand->isSelfBlob() ||
      FirstOperand->getSelfBlobIndex() != RvalTmp1->getSelfBlobIndex() ||
      SecondOperand->getSelfBlobIndex() != RvalTmp2->getSelfBlobIndex()) {
    return false;
  }

  return true;
}

// Matches instructions like- %sub165 = %mul164  -  %mul;
bool static matchesTempSub(HLInst *Inst, RegDDRef **LvalTmp, RegDDRef *RvalTmp1,
                           RegDDRef *RvalTmp2) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FSub) {
    LLVM_DEBUG(dbgs() << "Opcode FSub not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!FirstOperand->isSelfBlob() || !SecondOperand->isSelfBlob() ||
      FirstOperand->getSelfBlobIndex() != RvalTmp1->getSelfBlobIndex() ||
      SecondOperand->getSelfBlobIndex() != RvalTmp2->getSelfBlobIndex()) {
    return false;
  }

  return true;
}

// Matches instructions like- %add168 = %mul166  +  %mul167;
bool static matchesTempAdd(HLInst *Inst, RegDDRef **LvalTmp, RegDDRef *RvalTmp1,
                           RegDDRef *RvalTmp2) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FAdd) {
    LLVM_DEBUG(dbgs() << "Opcode FAdd not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!FirstOperand->isSelfBlob() || !SecondOperand->isSelfBlob()) {
    return false;
  }

  if (FirstOperand->getSelfBlobIndex() != RvalTmp1->getSelfBlobIndex()) {
    return false;
  }

  if (SecondOperand->getSelfBlobIndex() != RvalTmp2->getSelfBlobIndex()) {
    return false;
  }

  return true;
}

// Matches instructions like- %add173 = (@c)[0][i1 + 1][i3 + 1].0 + %sub165;
bool static matchesLoadTempAdd(HLInst *Inst, RegDDRef **LvalTmp,
                               RegDDRef **LoadRef, RegDDRef *RvalTmp) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FAdd) {
    LLVM_DEBUG(dbgs() << "Opcode FAdd not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!FirstOperand->isMemRef() || !SecondOperand->isSelfBlob()) {
    return false;
  }

  if (SecondOperand->getSelfBlobIndex() != RvalTmp->getSelfBlobIndex()) {
    return false;
  }

  *LoadRef = FirstOperand;

  return true;
}

bool static matchesComplexMatmulInnermostLoopPattern(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    RegDDRef *LvalTmp1, RegDDRef *LvalTmp2) {
  const RegDDRef *RealLoad1 = nullptr;
  const RegDDRef *RealLoad2 = nullptr;

  // Match: %.unpack1315 = (@"main_$A3")[0][i2 + 1][i3 + 1].0;
  HLInst *Inst = dyn_cast<HLInst>(Loop->getFirstChild());
  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  RealLoad1 = Inst->getRvalDDRef();
  RegDDRef *LvalTmp3 = Inst->getLvalDDRef();
  SmallVector<unsigned, 4> IVLevelsAtRealLoad1 = getIVLevels(RealLoad1);

  // Match: %.unpack1317 = (@"main_$A3")[0][i2 + 1][i3 + 1].1;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  if (RealLoad1->getSymbase() != Inst->getRvalDDRef()->getSymbase()) {
    return false;
  }
  RegDDRef *LvalTmp4 = Inst->getLvalDDRef();
  SmallVector<unsigned, 4> IVLevelsAtImagLoad1 =
      getIVLevels(Inst->getRvalDDRef());

  // Compare IV levels
  if (IVLevelsAtRealLoad1.size() != 2 || IVLevelsAtImagLoad1.size() != 2 ||
      IVLevelsAtRealLoad1[0] != IVLevelsAtImagLoad1[0] ||
      IVLevelsAtRealLoad1[1] != IVLevelsAtImagLoad1[1]) {
    return false;
  }

  // Match: %mul = %.unpack1317  *  %.unpack1321;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempMulRef1;
  if (!matchesTempMul(Inst, &TempMulRef1, LvalTmp4, LvalTmp2) &&
      !matchesTempMul(Inst, &TempMulRef1, LvalTmp2, LvalTmp4)) {
    return false;
  }

  // Match: %mul164 = %.unpack1315  *  %.unpack1319;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempMulRef2;
  if (!matchesTempMul(Inst, &TempMulRef2, LvalTmp3, LvalTmp1) &&
      !matchesTempMul(Inst, &TempMulRef2, LvalTmp1, LvalTmp3)) {
    return false;
  }

  // Match: %sub165 = %mul164  -  %mul;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempSubRef;
  if (!matchesTempSub(Inst, &TempSubRef, TempMulRef2, TempMulRef1)) {
    return false;
  }

  // Match: %mul166 = %.unpack1317  *  %.unpack1319;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempMulRef3;
  if (!matchesTempMul(Inst, &TempMulRef3, LvalTmp4, LvalTmp1) &&
      !matchesTempMul(Inst, &TempMulRef3, LvalTmp2, LvalTmp3)) {
    return false;
  }

  // Match: %mul167 = %.unpack1315  *  %.unpack1321;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempMulRef4;
  if (!matchesTempMul(Inst, &TempMulRef4, LvalTmp3, LvalTmp2) &&
      !matchesTempMul(Inst, &TempMulRef4, LvalTmp1, LvalTmp4)) {
    return false;
  }

  // Match: %add168 = %mul166  +  %mul167;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempAddRef1;
  if (!matchesTempAdd(Inst, &TempAddRef1, TempMulRef3, TempMulRef4)) {
    return false;
  }

  // Match: %add173 = (@"main_$C3")[0][i1 + 1][i3 + 1].0  +  %sub165;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempAddRef2;
  RegDDRef *LoadRef1;
  if (!matchesLoadTempAdd(Inst, &TempAddRef2, &LoadRef1, TempSubRef)) {
    return false;
  }

  // Match: %add176 = (@"main_$C3")[0][i1 + 1][i3 + 1].1  +  %add168;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  RegDDRef *TempAddRef3;
  RegDDRef *LoadRef2;
  if (!matchesLoadTempAdd(Inst, &TempAddRef3, &LoadRef2, TempAddRef1)) {
    return false;
  }

  if (LoadRef1->getSymbase() != LoadRef2->getSymbase()) {
    return false;
  }

  // Match: (@"main_$C3")[0][i1 + 1][i3 + 1].0 = %add173;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  if (!Inst || !isa<StoreInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  RealLoad2 = Inst->getLvalDDRef();
  if (RealLoad2->getSymbase() != LoadRef2->getSymbase()) {
    return false;
  }

  if (!DDRefUtils::areEqual(Inst->getRvalDDRef(), TempAddRef2)) {
    return false;
  }

  // Match: (@"main_$C3")[0][i1 + 1][i3 + 1].1 = %add176;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  if (!Inst || !isa<StoreInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  if (RealLoad2->getSymbase() != Inst->getLvalDDRef()->getSymbase()) {
    return false;
  }

  if (!DDRefUtils::areEqual(Inst->getRvalDDRef(), TempAddRef3)) {
    return false;
  }

  // Collect mem refs
  MatrixRefs.push_back(RealLoad2);
  MatrixRefs.push_back(RealLoad1);
  return true;
}

// Example of multiplication of a matrix with a vector-
// DO i1 = 0, 999, 1   <DO_LOOP>
//   %add120 = (@c)[0][i1];
//
//   + DO i2 = 0, 999, 1   <DO_LOOP>
//   |   %mul = (@b)[0][i2]  *  (@a)[0][i1][i2];
//   |   %add120 = %add120  +  %mul;
//   + END LOOP
//
//   (@c)[0][i1] = %add120;
// END LOOP
bool HIRGenerateMKLCall::isMatVecMul(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool *IsZeroSet) const {
  LLVM_DEBUG(dbgs() << "\nIn vecmul pattern matching:\n"; Loop->dump());

  // Before the innermost loop we can have the load instruction
  auto FirstChild = Loop->getFirstChild();
  *IsZeroSet = false;

  // LoadRef stores the initial load value of the output matrix
  const RegDDRef *InitialLoadRef;
  HLInst *InitialLoad = dyn_cast<HLInst>(FirstChild);
  if (!InitialLoad || !isa<LoadInst>(InitialLoad->getLLVMInstruction())) {
    return false;
  }
  InitialLoadRef = InitialLoad->getRvalDDRef();

  // Check for innermost loop
  HLLoop *InnermostLoop = dyn_cast<HLLoop>(FirstChild->getNextNode());

  if (!InnermostLoop) {
    LLVM_DEBUG(dbgs() << "Innermost loop missing.\n");
    return false;
  }
  // For the first pattern store comes as last child of second level loop
  if (!InnermostLoop->getNextNode()) {
    return false;
  }

  // Innermost loop contains multiplications and additions
  const RegDDRef *LoadRef1 = nullptr;
  const RegDDRef *LoadRef2 = nullptr;
  HLInst *FirstInst = dyn_cast<HLInst>(InnermostLoop->getFirstChild());

  // Match:  %mul = (@b)[0][i2]  *  (@a)[0][i1][i2];
  if (!matchMultiplication(FirstInst, &LoadRef1, &LoadRef2)) {
    return false;
  }

  // Memref initialization check
  // For some patterns one of the loads of input matrix  may get hoisted out
  // We need to initialize LoadRef1 or LoadRef2 from that load and reinitialize
  // InitialLoadRef to null. See similar example IR at isMatMul()
  // TODO: add example IR
  if (!LoadRef1 && InitialLoadRef) {
    std::swap(LoadRef1, InitialLoadRef);
  } else if (!LoadRef2 && InitialLoadRef) {
    std::swap(LoadRef2, InitialLoadRef);
  } else if (!LoadRef1 || !LoadRef2) {
    LLVM_DEBUG(dbgs() << "Multiplication does not involve two memrefs.\n");
    return false;
  }

  // The memory ref with two dimension should go as the last dope vector
  // So we swap them so that LoadRef1 has the higher dimension ref
  if (LoadRef1->getNumDimensions() < LoadRef2->getNumDimensions()) {
    std::swap(LoadRef1, LoadRef2);
  }

  // Second child of the innermost loop is the addition
  // InitialLoadRef can get populated from this instruction
  // Match: %add120 = %add120  +  %mul;
  HLInst *SecondInst = dyn_cast<HLInst>(FirstInst->getNextNode());
  if (!matchAddition(SecondInst, &InitialLoadRef)) {
    return false;
  }

  HLInst *ThirdInst = dyn_cast<HLInst>(InnermostLoop->getNextNode());

  // Match: (@c)[0][i1] = %add120;
  const RegDDRef *StoreRef = nullptr;
  if (!checkStoreInstruction(ThirdInst, InitialLoadRef, &StoreRef)) {
    return false;
  }

  // Check that all symbases are different
  if (LoadRef1->getSymbase() == StoreRef->getSymbase() ||
      LoadRef2->getSymbase() == StoreRef->getSymbase()) {
    return false;
  }

  // Number of dimensions check
  unsigned NumDims1 = LoadRef1->getNumDimensions();
  unsigned NumDims2 = LoadRef2->getNumDimensions();
  unsigned NumDims3 = StoreRef->getNumDimensions();

  if (!((NumDims1 == 2 && NumDims2 == 2 && NumDims3 == 2) ||
        (NumDims1 == 2 && NumDims2 == 1 && NumDims3 == 2) ||
        (NumDims1 == 2 && NumDims2 == 3 && NumDims3 == 2) ||
        (NumDims1 == 3 && NumDims2 == 2 && NumDims3 == 2))) {
    LLVM_DEBUG(dbgs() << "Number of dimensions not fit for vecmul.\n");
    return false;
  }

  // Check IV levels in 3 memrefs
  SmallVector<unsigned, 4> Levels1 = getIVLevels(LoadRef1); // 2   // 3 2
  SmallVector<unsigned, 4> Levels2 = getIVLevels(LoadRef2); // 2 1 // 3
  SmallVector<unsigned, 4> Levels3 = getIVLevels(StoreRef); // 1   // 2

  if (Levels1.size() > 1) {
    if (Levels1[0] != Levels2[0] || Levels1[1] != Levels3[0]) {
      LLVM_DEBUG(dbgs() << "IV leveles mismatch for matmul.\n");
      return false;
    }
  } else {
    if (Levels1[0] != Levels2[0] || Levels2[1] != Levels3[0]) {
      LLVM_DEBUG(dbgs() << "IV leveles mismatch for matmul.\n");
      return false;
    }
  }

  TripCountDDRefs = {Loop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef()};

  MatrixRefs.push_back(StoreRef);
  MatrixRefs.push_back(LoadRef2);
  MatrixRefs.push_back(LoadRef1);
  return true;
}

// Complex matrix multiplication example-
//+ DO i1 = 0, 49, 1   <DO_LOOP>
//|   + DO i2 = 0, 49, 1   <DO_LOOP>
//|   |   %.unpack1319 = (@"main_$B3")[0][i1 + 1][i2 + 1].0;
//|   |   %.unpack1321 = (@"main_$B3")[0][i1 + 1][i2 + 1].1;
//|   |
//|   |   + DO i3 = 0, 49, 1   <DO_LOOP>
//|   |   |   %.unpack1315 = (@"main_$A3")[0][i2 + 1][i3 + 1].0;
//|   |   |   %.unpack1317 = (@"main_$A3")[0][i2 + 1][i3 + 1].1;
//|   |   |   %mul = %.unpack1317  *  %.unpack1321;
//|   |   |   %mul164 = %.unpack1315  *  %.unpack1319;
//|   |   |   %sub165 = %mul164  -  %mul;
//|   |   |   %mul166 = %.unpack1317  *  %.unpack1319;
//|   |   |   %mul167 = %.unpack1315  *  %.unpack1321;
//|   |   |   %add168 = %mul166  +  %mul167;
//|   |   |   %add173 = (@"main_$C3")[0][i1 + 1][i3 + 1].0  +  %sub165;
//|   |   |   %add176 = (@"main_$C3")[0][i1 + 1][i3 + 1].1  +  %add168;
//|   |   |   (@"main_$C3")[0][i1 + 1][i3 + 1].0 = %add173;
//|   |   |   (@"main_$C3")[0][i1 + 1][i3 + 1].1 = %add176;
//|   |   + END LOOP
//|   + END LOOP
//+ END LOOP
bool HIRGenerateMKLCall::isComplexMatmul(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool *IsZeroSet) const {
  LLVM_DEBUG(dbgs() << "\nIn complex matmul pattern matching:\n"; Loop->dump());

  const RegDDRef *LoadRef1 = nullptr;
  *IsZeroSet = false;

  // STEP 1: Check for second level loop
  HLLoop *ChildLoop = dyn_cast<HLLoop>(Loop->getFirstChild());
  if (!ChildLoop) {
    LLVM_DEBUG(dbgs() << "Second level loop missing.\n");
    return false;
  }

  // Before the innermost loop we have two loads
  // Match: %.unpack1319 = (@"main_$B3")[0][i1 + 1][i2 + 1].0;
  HLInst *Inst = dyn_cast<HLInst>(ChildLoop->getFirstChild());
  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  LoadRef1 = Inst->getRvalDDRef();
  RegDDRef *LvalTmp1 = Inst->getLvalDDRef();
  SmallVector<unsigned, 4> Levels1 = getIVLevels(LoadRef1);

  // Match: %.unpack1321 = (@"main_$B3")[0][i1 + 1][i2 + 1].1;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  if (LoadRef1->getSymbase() != Inst->getRvalDDRef()->getSymbase()) {
    return false;
  }
  RegDDRef *LvalTmp2 = Inst->getLvalDDRef();
  SmallVector<unsigned, 4> Levels1Temp = getIVLevels(Inst->getRvalDDRef());

  // Compare IV levels
  if (Levels1.size() != 2 || Levels1Temp.size() != 2 ||
      Levels1[0] != Levels1Temp[0] || Levels1[1] != Levels1Temp[1]) {
    return false;
  }

  // STEP 2: Check for innermost loop
  HLLoop *InnermostLoop = dyn_cast<HLLoop>(Inst->getNextNode());
  if (!InnermostLoop) {
    LLVM_DEBUG(dbgs() << "Innermost loop missing.\n");
    return false;
  }

  if (!matchesComplexMatmulInnermostLoopPattern(InnermostLoop, MatrixRefs,
                                                LvalTmp1, LvalTmp2)) {
    return false;
  }
  MatrixRefs.push_back(LoadRef1);
  TripCountDDRefs = {Loop->getTripCountDDRef(), ChildLoop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef()};

  return true;
}

// Matrix multiplication example-
// DO i1 = 0, 499, 1   <DO_LOOP>
//   + DO i2 = 0, 499, 1   <DO_LOOP>
//   |   %add44 = 0.000000e+00;
//   |
//   |   + DO i3 = 0, 99, 1   <DO_LOOP>
//   |   |   %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
//   |   |   %add44 = %add44  +  %mul;
//   |   + END LOOP
//   |
//   |   (@c)[0][i1][i2] = %add44;
//   + END LOOP
// END LOOP
bool HIRGenerateMKLCall::isMatmul(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool *IsZeroSet) const {
  LLVM_DEBUG(dbgs() << "\nIn matmul pattern matching:\n"; Loop->dump());

  // STEP 1: Check for second level loop
  HLLoop *ChildLoop = dyn_cast<HLLoop>(Loop->getFirstChild());
  if (!ChildLoop) {
    LLVM_DEBUG(dbgs() << "Second level loop missing.\n");
    return false;
  }

  // Before the innermost loop we can have an optional assignment or a load
  // Match: %add44 = 0.000000e+00;
  bool TempAssignment = false;
  auto Itr = ChildLoop->child_begin();
  ConstantFP *FC = nullptr;
  HLInst *Assignment = dyn_cast<HLInst>(&*Itr);

  if (Assignment && Assignment->getLvalDDRef() &&
      Assignment->getLvalDDRef()->isTerminalRef() &&
      Assignment->getRvalDDRef() &&
      (Assignment->getRvalDDRef()->isFPConstant(&FC) ||
       isa<LoadInst>(Assignment->getLLVMInstruction()))) {
    TempAssignment = true;
    Itr++;
  }

  // Check for zero initialized or track the memref in case of load
  const RegDDRef *InitialLoadRef = nullptr;
  if (FC && !FC->isZero()) {
    LLVM_DEBUG(dbgs() << "Non-zero initialization of output array.\n");
    return false;
  } else if (Assignment && isa<LoadInst>(Assignment->getLLVMInstruction())) {
    InitialLoadRef = Assignment->getRvalDDRef();
  } else if (FC) {
    *IsZeroSet = true;
  }

  // STEP 2: Check for innermost loop
  HLLoop *InnermostLoop = dyn_cast<HLLoop>(&*Itr);
  if (!InnermostLoop) {
    LLVM_DEBUG(dbgs() << "Innermost loop missing.\n");
    return false;
  }

  // Innermost loop contains the initialization (with a load, if not at second
  // level loop), multiplications and additions
  auto InnerItr = InnermostLoop->child_begin();
  if (!TempAssignment) {
    HLInst *InitialLoad = dyn_cast<HLInst>(&*InnerItr);
    if (!InitialLoad || !isa<LoadInst>(InitialLoad->getLLVMInstruction())) {
      LLVM_DEBUG(dbgs() << "Unwanted instructions inside innermost loop.\n");
      return false;
    }
    InnerItr++;

    auto I = InitialLoad->rval_op_ddref_begin();
    if (InitialLoadRef) {
      LLVM_DEBUG(dbgs() << "Another load exists before this!");
      return false;
    }
    InitialLoadRef = *I;
  }

  // Match: %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
  const RegDDRef *LoadRef1 = nullptr;
  const RegDDRef *LoadRef2 = nullptr;
  HLInst *FirstInst = dyn_cast<HLInst>(&*InnerItr);
  if (!matchMultiplication(FirstInst, &LoadRef1, &LoadRef2)) {
    return false;
  }

  // Memref initialization check
  // For some patterns one of the loads of input matrix  may get hoisted out
  // We need to initialize LoadRef1 or LoadRef2 from that load and reinitialize
  // InitialLoadRef to null
  // + DO i1 = 0, zext.i32.i64((1 + %"matmul_$N1")) + -2, 1   <DO_LOOP>
  // |   + DO i2 = 0, zext.i32.i64((1 + %"matmul_$N1")) + -2, 1   <DO_LOOP>
  // |   |   %5 = (%"matmul_$B")[i1 + 1][i2 + 1];
  // |   |
  // |   |   + DO i3 = 0, zext.i32.i64((1 + %"matmul_$N1")) + -2, 1   <DO_LOOP>
  // |   |   |   %mul64 = (%"matmul_$A")[i2 + 1][i3 + 1]  *  %5;
  // |   |   |   %add65 = (%"matmul_$C")[i1 + 1][i3 + 1]  +  %mul64;
  // |   |   |   (%"matmul_$C")[i1 + 1][i3 + 1] = %add65;
  // |   |   + END LOOP
  // |   + END LOOP
  // + END LOOP
  if (!LoadRef1 && InitialLoadRef) {
    std::swap(LoadRef1, InitialLoadRef);
  } else if (!LoadRef2 && InitialLoadRef) {
    std::swap(LoadRef2, InitialLoadRef);
  } else if (!LoadRef1 || !LoadRef2) {
    LLVM_DEBUG(dbgs() << "Multiplication does not involve two memrefs.\n");
    return false;
  }

  // Second child of the innermost loop is the addition
  // InitialLoadRef can get populated from this instruction
  // Match: %add44 = %add44  +  %mul;
  InnerItr++;
  HLInst *SecondInst = dyn_cast<HLInst>(&*InnerItr);
  if (!matchAddition(SecondInst, &InitialLoadRef)) {
    return false;
  }

  // For the first pattern store comes as last child of the innermost loop
  // Otherwise it will in the second level loop
  InnerItr++;
  Itr++;
  HLInst *ThirdInst = nullptr;
  if (InnerItr != InnermostLoop->child_end()) {
    ThirdInst = dyn_cast<HLInst>(&*InnerItr);
  } else if (Itr != ChildLoop->child_end()) {
    ThirdInst = dyn_cast<HLInst>(&*Itr);
  }

  // Match: (@c)[0][i1][i2] = %add44;
  const RegDDRef *StoreRef = nullptr;
  if (!checkStoreInstruction(ThirdInst, InitialLoadRef, &StoreRef)) {
    return false;
  }

  // Check that all symbases are different
  if (LoadRef1->getSymbase() == StoreRef->getSymbase() ||
      LoadRef2->getSymbase() == StoreRef->getSymbase()) {
    return false;
  }

  // Number of dimensions check
  unsigned NumDims1 = LoadRef1->getNumDimensions();
  unsigned NumDims2 = LoadRef2->getNumDimensions();
  unsigned NumDims3 = StoreRef->getNumDimensions();

  if (!((NumDims1 == 2 && NumDims2 == 2 && NumDims3 == 2) ||
        (NumDims1 == 1 && NumDims2 == 1 && NumDims3 == 1) ||
        (NumDims1 == 2 && NumDims2 == 1 && NumDims3 == 2) ||
        (NumDims1 == 3 && NumDims2 == 3 && NumDims3 == 3) ||
        (NumDims1 == 1 && NumDims2 == 2 && NumDims3 == 1))) {
    LLVM_DEBUG(dbgs() << "Number of dimensions not fit for matmul.\n");
    return false;
  }

  // Check IV levels in 3 memrefs
  SmallVector<unsigned, 4> Levels1 = getIVLevels(LoadRef1); // 1 2 // 2 1 // 1 3
  SmallVector<unsigned, 4> Levels2 = getIVLevels(LoadRef2); // 2 3 // 3 2 // 2 3
  SmallVector<unsigned, 4> Levels3 = getIVLevels(StoreRef); // 1 3 // 3 1 // 1 2

  if (!((Levels1[0] == Levels2[1] && Levels1[1] == Levels3[1] &&
         Levels2[0] == Levels3[0]) ||
        (Levels1[1] == Levels2[0] && Levels1[0] == Levels3[0] &&
         Levels2[1] == Levels3[1]) ||
        (Levels1[0] == Levels3[0] && Levels1[1] == Levels2[1] &&
         Levels2[0] == Levels3[1]))) {
    LLVM_DEBUG(dbgs() << "IV leveles mismatch for matmul.\n");
    return false;
  }

  SmallSet<unsigned, 4> NestingLevels;
  NestingLevels.insert(Loop->getNestingLevel());
  NestingLevels.insert(ChildLoop->getNestingLevel());
  NestingLevels.insert(InnermostLoop->getNestingLevel());

  if (!(NestingLevels.count(Levels1[0]) && NestingLevels.count(Levels1[1]) &&
        NestingLevels.count(Levels2[0]) && NestingLevels.count(Levels2[1]) &&
        NestingLevels.count(Levels3[0]) && NestingLevels.count(Levels3[1]))) {
    return false;
  }

  TripCountDDRefs = {Loop->getTripCountDDRef(), ChildLoop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef()};

  MatrixRefs.push_back(StoreRef);
  // Check which matrix should go first based on the innermost iv matching
  // with store ref
  if (std::max(Levels3[1], Levels3[0]) == Levels1[0] ||
      std::max(Levels3[1], Levels3[0]) == Levels1[1]) {
    MatrixRefs.push_back(LoadRef1);
    MatrixRefs.push_back(LoadRef2);
  } else {
    MatrixRefs.push_back(LoadRef2);
    MatrixRefs.push_back(LoadRef1);
  }
  return true;
}
