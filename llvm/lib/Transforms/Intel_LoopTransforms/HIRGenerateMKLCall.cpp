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
#include "llvm/Transforms/Intel_LoopTransforms/HIRGenerateMKLCallPass.h"

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
      :  DopeVectorType(nullptr), HIRF(HIRF), HLS(HLS) {}

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
                                         unsigned &DopeVectorFieldIdx,
                                         unsigned DVSymbase);

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

} // namespace

PreservedAnalyses HIRGenerateMKLCallPass::runImpl(Function &Func,
                                                  FunctionAnalysisManager &AM,
                                                  HIRFramework &HIRF) {
  ModifiedHIR =
      HIRGenerateMKLCall(HIRF, AM.getResult<HIRLoopStatisticsAnalysis>(Func))
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

// Gather all perfect Loop Nests with level 2/3
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
    LLVM_DEBUG(dbgs() << "In collect Perfect loopnest\n");

    // Perfect LoopNest only

    if (!(HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop))) {
      return;
    }

    if (GMKLCall.HLS.getSelfStatistics(InnermostLoop)
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
      if (!ChildLoop->isInnermost() &&
          isMatVecMul(ChildLoop, MatrixRefs, TripCountDDRefs, &IsZeroSet)) {
        LLVM_DEBUG(dbgs() << "\nLoop is a vecmul\n"; Loop->dump());
        computeDopeVectorFieldsAndTransform(Context, ChildLoop, MatrixRefs,
                                            TripCountDDRefs, IsZeroSet, false);
        MKLCallGenerated = true;
      }
    }

    Modified |= MKLCallGenerated;
  }

  return Modified;
}

static bool isComplexType(const RegDDRef *RealRef, const RegDDRef *ImagRef) {

  auto *StructTy = dyn_cast<StructType>(RealRef->getDimensionElementType(1));
  if (!StructTy) {
    return false;
  }

  if (StructTy->getNumElements() != 2) {
    return false;
  }
  //  B3[i1][i2].0
  //  B3[i1][i2].1
  //  Verify that first offset is 0. Second offset equals data type of the first
  //  field

  if (!CanonExprUtils::areEqual(RealRef->getBaseCE(), ImagRef->getBaseCE())) {
    return false;
  }

  if (ImagRef->getDestTypeSizeInBytes() != RealRef->getDestTypeSizeInBytes()) {
    return false;
  }

  auto RealOffsets = RealRef->getTrailingStructOffsets(1);
  auto &DL = RealRef->getCanonExprUtils().getDataLayout();

  if (DDRefUtils::getOffsetDistance(StructTy, DL, RealOffsets) != 0) {
    return false;
  }

  if (ImagRef->getDimensionElementType(1) != StructTy) {
    return false;
  }
  auto ImagOffsets = ImagRef->getTrailingStructOffsets(1);
  int64_t TypeSize = RealRef->getDestTypeSizeInBytes();

  if (DDRefUtils::getOffsetDistance(StructTy, DL, ImagOffsets) != TypeSize) {
    return false;
  }

  return true;
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
SmallVector<unsigned, 4> getIVLevelsAtDim(const RegDDRef *RRef, unsigned Dim,
                                          bool *IsValid,
                                          bool Validate = false) {

  SmallVector<unsigned, 4> Levels;
  auto *CE = RRef->getDimensionIndex(Dim);
  if (Validate)
    *IsValid = true;

  for (auto CurIVPair = CE->iv_begin(), E = CE->iv_end(); CurIVPair != E;
       ++CurIVPair) {

    if (CE->hasIV(CE->getLevel(CurIVPair))) {
      // Library supports only constant coeff = 1
      if (Validate) {
        if (CE->getIVConstCoeff(CurIVPair) != 1 ||
            CE->getIVBlobCoeff(CurIVPair)) {
          *IsValid = false;
          break;
        }
      }
      Levels.push_back(CE->getLevel(CurIVPair));
    }
  }
  return Levels;
}

// Return the iv levels present in all dimensions
SmallVector<unsigned, 4> getIVLevels(const RegDDRef *RRef, bool *IsValid,
                                     bool Validate = false) {

  SmallVector<unsigned, 4> Levels;

  if (Validate) {
    // Assumed shape or pointer array will not work with library code
    int64_t ConstStride = RRef->getDimensionConstStride(1);

    // DV.stride may be constant prop
    auto DimTy = RRef->getDimensionElementType(1);
    int64_t ElemSize = RRef->getCanonExprUtils().getTypeSizeInBytes(DimTy);
    if (ConstStride != ElemSize) {
      *IsValid = false;
      return Levels;
    }
  }

  unsigned NumDims = RRef->getNumDimensions();

  for (unsigned Dim = 1; Dim <= NumDims; Dim++) {
    SmallVector<unsigned, 4> LevelsAtDim =
        getIVLevelsAtDim(RRef, Dim, IsValid, Validate);
    if (Validate && !IsValid) {
      break;
    }

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
    unsigned &DopeVectorFieldIdx, unsigned DVSymbase) {
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

  RegDDRef *DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

  auto FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);
  auto StoreInst = HNU.createStore(ExtentRef, ".extent", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'stride' field for current dimension and create the
  // assignment instruction
  // (%.DopeVector1)[0].7 = 4;

  auto *StrideCanon = MatrixRef->getStrideAtLevel(IVLevel);

  assert(StrideCanon && "Cannot find dimension with level");

  RegDDRef *StrideCanonRef =
      DDRU.createScalarRegDDRef(GenericRvalSymbase, StrideCanon);

  StrideCanonRef->makeConsistent(MatrixRef, Level);
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);
  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);
  StoreInst = HNU.createStore(StrideCanonRef, ".stride", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'lower' field for current dimension and create the assignment
  // instruction
  // (%.DopeVector1)[0].8 = 1;
  RegDDRef *LowerRef = DDRU.createConstDDRef(IntType, 1);
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);
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

  unsigned Level = Loop->getNestingLevel() - 1;

  unsigned AllocaBlobIdx =
      HNU.createAlloca(DopeVectorType, RegionNode, ".DopeVector");
  unsigned DopeVectorFieldIdx = 0;

  unsigned DVSymbase = HNU.getDDRefUtils().getNewSymbase();

  // Compute 'addr_a0' field and create the assignment instruction
  // (%.DopeVector1)[0].0 = &((i8*)(@c)[0][0][0]);
  RegDDRef *DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

  CanonExpr *FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto *BasePtrRef = MatrixRef->clone();
  BasePtrRef->setAddressOf(true);

  BasePtrRef->setBitCastDestVecOrElemType(Type::getInt8Ty(Context));
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
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

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
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto OffsetRef = DDRU.createConstDDRef(IntType, 0);
  StoreInst = HNU.createStore(OffsetRef, ".offset", DopeVectorRef);

  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'flags' field and create the assignment instruction
  // (%.DopeVector1)[0].3 = 0;
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto FlagsRef = DDRU.createConstDDRef(IntType, 0);
  StoreInst = HNU.createStore(FlagsRef, ".flags", DopeVectorRef);

  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'rank' field and create the assignment instruction
  // (%.DopeVector1)[0].4 = 2;
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  unsigned DimNum = 0;
  for (auto CE = MatrixRef->canon_begin(), E = MatrixRef->canon_end(); CE != E;
       ++CE) {
    if ((*CE)->hasIV()) {
      DimNum++;
    }
  }

  bool IsValid = true;

  SmallVector<unsigned, 4> LevelsAtDim =
      getIVLevelsAtDim(MatrixRef, 1, &IsValid);
  auto DimNumRef = DDRU.createConstDDRef(
      IntType, std::max(DimNum, (unsigned)LevelsAtDim.size()));
  StoreInst = HNU.createStore(DimNumRef, ".rank", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  // Compute 'reserved' field and create the assignment instruction
  // (%.DopeVector1)[0].5 = 0;
  DopeVectorRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

  FirstCE = CEU.createCanonExpr(IntType);
  DopeVectorRef->addDimension(FirstCE, DopeVectorFieldIdx++);

  auto ReservedRef = DDRU.createConstDDRef(IntType, 0);
  StoreInst = HNU.createStore(ReservedRef, ".reserved", DopeVectorRef);
  HLNodeUtils::insertBefore(Loop, StoreInst);
  LLVM_DEBUG(StoreInst->dump(); dbgs() << "\n");

  DimNum = 1;
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
    LevelsAtDim = getIVLevelsAtDim(MatrixRef, DimNum, &IsValid);

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

      createDopeVectorAssignmentsForDim(
          Loop, IVLevel, MatrixRef, TripCountDDRefs[IVLevel - MinLevel],
          AllocaBlobIdx, DopeVectorFieldIdx, DVSymbase);
    }
  }

  // Create the dope vector field reference and make it live-in to loop
  RegDDRef *DopeRef =
      DDRU.createMemRef(DopeVectorType, AllocaBlobIdx, 0, DVSymbase);

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
  assert(DopeVectorType && "Dope vector type not created");

  bool IsValid = true;
  SmallVector<RegDDRef *, 8> CallArgs;
  SmallVector<unsigned, 4> LevelsAtStoreRef =
      getIVLevels(MatrixRefs[0], &IsValid);

  SmallVector<unsigned, 4> LevelsAtLoadRef1 =
      getIVLevels(MatrixRefs[1], &IsValid);
  SmallVector<unsigned, 4> LevelsAtLoadRef2 =
      getIVLevels(MatrixRefs[2], &IsValid);
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

  std::array<RegDDRef *, 3> FakeDDRef;

  // Process each of the mem refs and create dope vector structure for each
  for (int i = 0; i < 3; i++) {
    RegDDRef *DopeRef =
        createDopeVectorAssignments(Loop, MatrixRefs[i], TripCountDDRefs,
                                    IsComplexType, OrderedLevelsToProc);
    // Collect the dope vector ref as argument
    CallArgs.push_back(DopeRef);

    auto *FakeRvalRef = DopeRef->clone();
    FakeRvalRef->setAddressOf(false);
    FakeDDRef[i] = FakeRvalRef;
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

  for (int i = 0; i < 3; i++) {
    CallInst->addFakeRvalDDRef(FakeDDRef[i]);
  }

  HLNodeUtils::insertBefore(Loop, CallInst);
  LLVM_DEBUG(CallInst->dump(); dbgs() << "\n");

  OptReportBuilder &ORBuilder = HIRF.getORBuilder();

  // Loopnest replaced by matmul intrinsic
  ORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                             OptRemarkID::LoopNestReplacedByMatmul);

  ORBuilder(*Loop).preserveLostOptReport();

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

// Matches instructions like these
// %mul = %50  *  (@b)[0][i2][i3]
// %mul = (@b)[0][i2][i3] * %50
// %mul = (%"matvec_$A")[i1][i2]  *  (%"matvec_$B")[i2];

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

  // Both Temps?
  if (OperandRef1->isTerminalRef() && OperandRef2->isTerminalRef()) {
    return false;
  }

  *RefA = OperandRef1;
  *RefB = OperandRef2;

  return true;
}

// Match instruction like this
// %add.1 = %mul.4  +  (@c)[0][i1][i2];

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

  if (RefA->isMemRef() && RefB->isMemRef()) {
    LLVM_DEBUG(dbgs() << "Multiple load at addition is present.\n");
    return false;
  }

  if (RefA->isMemRef()) {
    *LoadRef = RefA;
  } else if (RefB->isMemRef()) {
    *LoadRef = RefB;
  }

  return true;
}

// Matches instructions like- (@c)[0][i1] = %add120;
// And checks that the store ref with load ref are equal
static bool checkStoreInstruction(const HLInst *Inst, const RegDDRef *LoadRef,
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

bool static matchTemp(const RegDDRef *LvalTmp, const RegDDRef *RvalTmp) {

  if (!LvalTmp->isSelfBlob() || !RvalTmp->isSelfBlob() ||
      LvalTmp->getSelfBlobIndex() != RvalTmp->getSelfBlobIndex()) {
    return false;
  }
  return true;
}

// Matches instructions like- %mul = %.unpack1317  *  %.unpack1321;
bool static matchesTempMul(const HLInst *Inst, const RegDDRef **LvalTmp,
                           const RegDDRef *RvalTmp1, const RegDDRef *RvalTmp2) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FMul) {
    LLVM_DEBUG(dbgs() << "Opcode FMul not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  const RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  const RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!FirstOperand->isSelfBlob() || !SecondOperand->isSelfBlob() ||
      FirstOperand->getSelfBlobIndex() != RvalTmp1->getSelfBlobIndex() ||
      SecondOperand->getSelfBlobIndex() != RvalTmp2->getSelfBlobIndex()) {
    return false;
  }

  return true;
}

// Matches instructions like- %sub165 = %mul164  -  %mul;
bool static matchesTempSub(const HLInst *Inst, const RegDDRef **LvalTmp,
                           const RegDDRef *RvalTmp1, const RegDDRef *RvalTmp2) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FSub) {
    LLVM_DEBUG(dbgs() << "Opcode FSub not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  const RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  const RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!FirstOperand->isSelfBlob() || !SecondOperand->isSelfBlob() ||
      FirstOperand->getSelfBlobIndex() != RvalTmp1->getSelfBlobIndex() ||
      SecondOperand->getSelfBlobIndex() != RvalTmp2->getSelfBlobIndex()) {
    return false;
  }

  return true;
}

// Matches instructions like- %add168 = %mul166  +  %mul167;
bool static matchesTempAdd(const HLInst *Inst, const RegDDRef **LvalTmp,
                           const RegDDRef *RvalTmp1, const RegDDRef *RvalTmp2) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FAdd) {
    LLVM_DEBUG(dbgs() << "Opcode FAdd not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  const RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  const RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

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

// Matches  %sub.1 = %mul.4  +  (%"cmatmul_$C3")[i1][i3].0;

bool static matchesLoadTempAdd(const HLInst *Inst, const RegDDRef **LvalTmp,
                               const RegDDRef *RvalTmp,
                               const RegDDRef **LoadRef) {
  if (!Inst) {
    return false;
  }
  unsigned Opcode = Inst->getLLVMInstruction()->getOpcode();
  if (Opcode != Instruction::FAdd) {
    LLVM_DEBUG(dbgs() << "Opcode FAdd not found.\n");
    return false;
  }

  *LvalTmp = Inst->getLvalDDRef();
  const RegDDRef *FirstOperand = Inst->getOperandDDRef(1);
  const RegDDRef *SecondOperand = Inst->getOperandDDRef(2);

  if (!SecondOperand->isMemRef() || !FirstOperand->isSelfBlob()) {
    return false;
  }

  if (FirstOperand->getSelfBlobIndex() != RvalTmp->getSelfBlobIndex()) {
    return false;
  }

  *LoadRef = SecondOperand;

  return true;
}

bool static matchesComplexMatmulInnermostLoopPattern(
    const HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs) {

  const RegDDRef *LvalTmp1;
  const RegDDRef *LvalTmp2;
  const RegDDRef *LvalTmp3;
  const RegDDRef *LvalTmp4;

  const RegDDRef *RealLoadB;  // B3
  const RegDDRef *RealLoadA;  // A3
  const RegDDRef *RealLoadC;  // C3
  const RegDDRef *RealStoreC; // C3

  const RegDDRef *ImagLoadB;
  const RegDDRef *ImagLoadA;
  const RegDDRef *ImagLoadC;
  const RegDDRef *ImagStoreC;

  unsigned InnermostLoopLevel = Loop->getNestingLevel();

  // In the innermost loop check first two loads
  // Match:  %fetch.17 = (%"cmatmul_$B3")[i1][i2].0;
  const auto *Inst = dyn_cast<HLInst>(Loop->getFirstChild());

  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  if (Loop->hasLiveOutTemps()) {
    return false;
  }

  // %fetch.17
  LvalTmp1 = Inst->getLvalDDRef();
  RealLoadB = Inst->getRvalDDRef();

  bool IsValid = true;
  SmallVector<unsigned, 4> Levels1 = getIVLevels(RealLoadB, &IsValid, true);
  if (!IsValid)
    return false;

  // Match: %fetch.18 = (%"cmatmul_$B3")[i1][i2].1;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  // %fetch.18
  LvalTmp2 = Inst->getLvalDDRef();
  ImagLoadB = Inst->getRvalDDRef();

  if (!isComplexType(RealLoadB, ImagLoadB)) {
    return false;
  }

  SmallVector<unsigned, 4> Levels1Temp =
      getIVLevels(Inst->getRvalDDRef(), &IsValid, true);
  if (!IsValid)
    return false;

  // Compare IV levels
  if (Levels1.size() != 2 || Levels1Temp.size() != 2 ||
      Levels1[0] != Levels1Temp[0] || Levels1[1] != Levels1Temp[1]) {
    return false;
  }

  if (Levels1[0] != (InnermostLoopLevel - 1) ||
      Levels1[1] != (InnermostLoopLevel - 2)) {
    return false;
  }

  // Match:  %fetch.13 = (%"cmatmul_$A3")[i2][i3].0;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  // %fetch13
  LvalTmp3 = Inst->getLvalDDRef();
  RealLoadA = Inst->getRvalDDRef();

  SmallVector<unsigned, 4> IVLevelsAtRealLoadB =
      getIVLevels(RealLoadA, &IsValid, true);
  if (!IsValid)
    return false;

  // Match:   %fetch.14 = (%"cmatmul_$A3")[i2][i3].1;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());
  if (!Inst || !isa<LoadInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  // %fetch14
  LvalTmp4 = Inst->getLvalDDRef();
  ImagLoadA = Inst->getRvalDDRef();

  if (!isComplexType(RealLoadA, ImagLoadA)) {
    return false;
  }

  SmallVector<unsigned, 4> IVLevelsAtImagLoadB =
      getIVLevels(ImagLoadA, &IsValid, true);

  if (!IsValid)
    return false;

  // Compare IV levels
  if (IVLevelsAtRealLoadB.size() != 2 || IVLevelsAtImagLoadB.size() != 2 ||
      IVLevelsAtRealLoadB[0] != IVLevelsAtImagLoadB[0] ||
      IVLevelsAtRealLoadB[1] != IVLevelsAtImagLoadB[1]) {
    return false;
  }

  if (IVLevelsAtRealLoadB[0] != (InnermostLoopLevel) ||
      IVLevelsAtRealLoadB[1] != (InnermostLoopLevel - 1)) {
    return false;
  }

  // Match: %mul.4 = %fetch.17  *  %fetch.13;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  const RegDDRef *TempMulRef1;
  if (!matchesTempMul(Inst, &TempMulRef1, LvalTmp1, LvalTmp3) &&
      !matchesTempMul(Inst, &TempMulRef1, LvalTmp3, LvalTmp1)) {
    return false;
  }

  // Match: %mul.6 = %fetch.18  *  %fetch.13;
  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  const RegDDRef *TempMulRef2;
  if (!matchesTempMul(Inst, &TempMulRef2, LvalTmp3, LvalTmp2) &&
      !matchesTempMul(Inst, &TempMulRef2, LvalTmp2, LvalTmp3)) {
    return false;
  }

  // Match:  %mul.7 = %fetch.17  *  %fetch.14;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  const RegDDRef *TempMulRef3;
  if (!matchesTempMul(Inst, &TempMulRef3, LvalTmp4, LvalTmp1) &&
      !matchesTempMul(Inst, &TempMulRef3, LvalTmp1, LvalTmp4)) {
    return false;
  }

  // Match: %sub.1 = %mul.4  +  (%"cmatmul_$C3")[i1][i3].0;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  const RegDDRef *TempAddRef1;

  if (!matchesLoadTempAdd(Inst, &TempAddRef1, TempMulRef1, &RealLoadC)) {
    return false;
  }

  SmallVector<unsigned, 4> IVLevelsAtRealLoadC =
      getIVLevels(RealLoadC, &IsValid, true);
  // Match: %6 = %fetch.18  *  %fetch.14;

  if (!IsValid)
    return false;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  const RegDDRef *TempMulRef4;
  if (!matchesTempMul(Inst, &TempMulRef4, LvalTmp2, LvalTmp4) &&
      !matchesTempMul(Inst, &TempMulRef4, LvalTmp4, LvalTmp2)) {
    return false;
  }

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  // Match: %add.2 = %sub.1  -  %6;

  const RegDDRef *TempSubRef;
  if (!matchesTempSub(Inst, &TempSubRef, TempAddRef1, TempMulRef4)) {
    return false;
  }

  // Match: %add.1 = %mul.7  +  (%"cmatmul_$C3")[i1][i3].1;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  const RegDDRef *TempAddRef3;

  if (!matchesLoadTempAdd(Inst, &TempAddRef3, TempMulRef3, &ImagLoadC)) {
    return false;
  }

  SmallVector<unsigned, 4> IVLevelsAtImagLoadC =
      getIVLevels(ImagLoadC, &IsValid, true);

  if (!IsValid)
    return false;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  // Match:  %add.3 = %add.1  +  %mul.6;

  const RegDDRef *TempAddRef4;
  if (!matchesTempAdd(Inst, &TempAddRef4, TempAddRef3, TempMulRef2)) {
    return false;
  }

  // Compare IV levels
  if (IVLevelsAtRealLoadC.size() != 2 || IVLevelsAtImagLoadC.size() != 2 ||
      IVLevelsAtRealLoadC[0] != IVLevelsAtImagLoadC[0] ||
      IVLevelsAtRealLoadC[1] != IVLevelsAtImagLoadC[1]) {
    return false;
  }

  if (IVLevelsAtRealLoadC[0] != (InnermostLoopLevel) ||
      IVLevelsAtRealLoadC[1] != (InnermostLoopLevel - 2)) {
    return false;
  }

  // Match:   (%"cmatmul_$C3")[i1][i3].0 = %add.2;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  if (!Inst || !isa<StoreInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  RealStoreC = Inst->getLvalDDRef();

  if (!CanonExprUtils::areEqual(RealStoreC->getBaseCE(),
                                RealLoadC->getBaseCE())) {
    return false;
  }

  if (!DDRefUtils::areEqual(Inst->getRvalDDRef(), TempSubRef)) {
    return false;
  }

  // Match:  (%"cmatmul_$C3")[i1][i3].1 = %add.3;

  Inst = dyn_cast<HLInst>(Inst->getNextNode());

  if (!Inst || !isa<StoreInst>(Inst->getLLVMInstruction())) {
    return false;
  }

  ImagStoreC = Inst->getLvalDDRef();

  if (!isComplexType(RealStoreC, ImagStoreC)) {
    return false;
  }

  if (!DDRefUtils::areEqual(Inst->getRvalDDRef(), TempAddRef4)) {
    return false;
  }

  if (Inst->getNextNode()) {
    return false;
  }

  // Collect MemRefs
  MatrixRefs.push_back(RealLoadC); // C3
  MatrixRefs.push_back(RealLoadA); // A3

  return true;
}

// Multiplication of a matrix with a vector
// Sinking is done prior to this pass
//
//  + DO i1 = 0, 4095, 1   <DO_LOOP>
//  |   + DO i2 = 0, 4095, 1   <DO_LOOP>
//  |   |   %add.113 = (%"matvec_$C")[i1];
//  |   |   %mul.2 = (%"matvec_$A")[i1][i2]  *  (%"matvec_$B")[i2];
//  |   |   %add.113 = %mul.2  +  %add.113;
//  |   |   (%"matvec_$C")[i1] = %add.113;
//  |   + END LOOP
//  + END LOOP

bool HIRGenerateMKLCall::isMatVecMul(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool *IsZeroSet) const {
  LLVM_DEBUG(dbgs() << "\nIn vecmul pattern matching:\n"; Loop->dump());

  bool IsValid = true;
  *IsZeroSet = false;

  const HLLoop *InnermostLoop;

  if (!(HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop))) {
    return false;
  }

  if (Loop->getFirstChild() != InnermostLoop) {
    return false;
  }

  if (InnermostLoop->hasLiveOutTemps()) {
    return false;
  }
  // LoadRef stores the initial load value of the output matrix
  const RegDDRef *InitialLoadRef;

  //  %add.113 = (%"matvec_$C")[i1]
  const HLInst *InitialLoad = dyn_cast<HLInst>(InnermostLoop->getFirstChild());

  if (!InitialLoad || !isa<LoadInst>(InitialLoad->getLLVMInstruction())) {
    return false;
  }

  InitialLoadRef = InitialLoad->getRvalDDRef();

  // Innermost loop contains multiplications and additions
  const RegDDRef *LoadRef1 = nullptr;
  const RegDDRef *LoadRef2 = nullptr;
  const HLInst *MultiplyInst = dyn_cast<HLInst>(InitialLoad->getNextNode());

  // Match:  %mul.2 = (%"matvec_$A")[i1][i2]  *  (%"matvec_$B")[i2];
  if (!matchMultiplication(MultiplyInst, &LoadRef1, &LoadRef2)) {
    return false;
  }

  // The memory ref with two dimension should go as the last dope vector
  // So we swap them so that LoadRef1 has the higher dimension ref
  if (LoadRef1->getNumDimensions() < LoadRef2->getNumDimensions()) {
    std::swap(LoadRef1, LoadRef2);
  }

  // Second child of the innermost loop is the addition
  // InitialLoadRef can get populated from this instruction
  // Match: %add.113 = %mul.2  +  %add.113;

  const HLInst *AddInst = dyn_cast<HLInst>(MultiplyInst->getNextNode());
  if (!matchAddition(AddInst, &InitialLoadRef)) {
    return false;
  }

  const HLInst *StoreInst = dyn_cast<HLInst>(AddInst->getNextNode());

  if (!StoreInst) {
    return false;
  }

  // Match:  (%"matvec_$C")[i1] = %add.113;
  const RegDDRef *StoreRef = nullptr;
  if (!checkStoreInstruction(StoreInst, InitialLoadRef, &StoreRef)) {
    return false;
  }

  // Match: Def and use of %add.113
  if (!matchTemp(InitialLoad->getLvalDDRef(), StoreInst->getRvalDDRef())) {
    return false;
  }

  if (StoreInst->getNextNode()) {
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

  if (!((NumDims1 == 2 && NumDims2 == 1 && NumDims3 == 1) ||
        (NumDims1 == 1 && NumDims2 == 2 && NumDims3 == 1) ||
        (NumDims1 == 2 && NumDims2 == 3 && NumDims3 == 2) ||
        (NumDims1 == 3 && NumDims2 == 2 && NumDims3 == 2))) {
    LLVM_DEBUG(dbgs() << "Number of dimensions not fit for vecmul\n");
    return false;
  }

  // Check IV levels in 3 memrefs
  SmallVector<unsigned, 4> Levels1 =
      getIVLevels(LoadRef1, &IsValid, true); // 2   // 3 2
  if (!IsValid)
    return false;

  SmallVector<unsigned, 4> Levels2 =
      getIVLevels(LoadRef2, &IsValid, true); // 2 1 // 3
  if (!IsValid)
    return false;

  SmallVector<unsigned, 4> Levels3 =
      getIVLevels(StoreRef, &IsValid, true); // 1   // 2
  if (!IsValid)
    return false;

  // A(i2,i1) comes before B(i2)
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

  //  Check explict index level = nesting level - 1 for store C[i1]
  if (Levels3[0] != (InnermostLoop->getNestingLevel() - 1)) {
    return false;
  }

  TripCountDDRefs = {Loop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef()};

  MatrixRefs.push_back(StoreRef);
  MatrixRefs.push_back(LoadRef2);
  MatrixRefs.push_back(LoadRef1);
  return true;
}

// Complex matrix multiplication example

//    + DO i1 = 0, 49, 1   <DO_LOOP>
//    |   + DO i2 = 0, 49, 1   <DO_LOOP>
//    |   |   + DO i3 = 0, 49, 1   <DO_LOOP>
//    |   |   |   %fetch.17 = (%"cmatmul_$B3")[i1][i2].0;
//    |   |   |   %fetch.18 = (%"cmatmul_$B3")[i1][i2].1;
//    |   |   |   %fetch.13 = (%"cmatmul_$A3")[i2][i3].0;
//    |   |   |   %fetch.14 = (%"cmatmul_$A3")[i2][i3].1;
//    |   |   |   %mul.4 = %fetch.17  *  %fetch.13;
//    |   |   |   %mul.6 = %fetch.18  *  %fetch.13;
//    |   |   |   %mul.7 = %fetch.17  *  %fetch.14;
//    |   |   |   %sub.1 = %mul.4  +  (%"cmatmul_$C3")[i1][i3].0;
//    |   |   |   %6 = %fetch.18  *  %fetch.14;
//    |   |   |   %add.2 = %sub.1  -  %6;
//    |   |   |   %add.1 = %mul.7  +  (%"cmatmul_$C3")[i1][i3].1;
//    |   |   |   %add.3 = %add.1  +  %mul.6;
//    |   |   |   (%"cmatmul_$C3")[i1][i3].0 = %add.2;
//    |   |   |   (%"cmatmul_$C3")[i1][i3].1 = %add.3;
//    |   |   + END LOOP
//    |   + END LOOP
//    + END LOOP

bool HIRGenerateMKLCall::isComplexMatmul(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool *IsZeroSet) const {
  LLVM_DEBUG(dbgs() << "\nIn complex matmul pattern matching:\n"; Loop->dump());

  const HLLoop *InnermostLoop;

  if (!(HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop))) {
    return false;
  }

  const RegDDRef *LoadRef1;
  *IsZeroSet = false;

  // STEP 1: Check for second level loop
  HLLoop *ChildLoop = dyn_cast<HLLoop>(Loop->getFirstChild());
  if (!ChildLoop) {
    LLVM_DEBUG(dbgs() << "Second level loop missing.\n");
    return false;
  }

  // STEP 2: Check for innermost loop Matrix Refs

  if (!matchesComplexMatmulInnermostLoopPattern(InnermostLoop, MatrixRefs)) {
    return false;
  }

  const auto *Inst = dyn_cast<HLInst>(InnermostLoop->getFirstChild());
  LoadRef1 = Inst->getRvalDDRef();

  MatrixRefs.push_back(LoadRef1);
  TripCountDDRefs = {Loop->getTripCountDDRef(), ChildLoop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef()};

  return true;
}

//   Matrix multiplication
//   Interchange and Sinking are done prior to this pass
//   matmul pattern occurs mostly in this form:
//
//    + DO i1 = 0, 4095, 1   <DO_LOOP>
//    |   + DO i2 = 0, 4095, 1   <DO_LOOP>
//    |   |   + DO i3 = 0, 4095, 1   <DO_LOOP>
//    |   |   |   %"cmatmul_$B3[][]_fetch.9" = (%"cmatmul_$B3")[i1][i2];
//    |   |   |   %mul.4 = %"cmatmul_$B3[][]_fetch.9"  *
//    (%"cmatmul_$A3")[i2][i3]; |   |   |   %add.1 = %mul.4  +
//    (%"cmatmul_$C3")[i1][i3]; |   |   |   (%"cmatmul_$C3")[i1][i3] = %add.1;
//    |   |   + END LOOP
//    |   + END LOOP
//    + END LOOP
//
//    But this pattern may occur  (TODO: Not important for now)
//    + DO i1 = 0, 4095, 1   <DO_LOOP>
//    |   + DO i2 = 0, 4095, 1   <DO_LOOP>
//    |   |   + DO i3 = 0, 4095, 1   <DO_LOOP>
//    |   |   |   %add  = (%"cmatmul_$C3")[i1][i3];
//    |   |   |   %mul =  (%"cmatmul_$B3")[i1][i2] * (%"cmatmul_$A3")[i2][i3];
//    |   |   |   %add =  %mul + %add;
//    |   |   |   (%"cmatmul_$C3")[i1][i3] = %add;
//    |   |   + END LOOP
//    |   + END LOOP
//    + END LOOP
//
//    For Linearized form, this pattern occurs  (TODO: Not important for now,
//    requiring new code to invoke delinerization)
//    + DO i1 = 0, zext.i32.i64(%M)
//    |   + DO i2 = 0, sext.i32.i64(%N)
//    |   |   + DO i3 = 0, sext.i32.i64(%K)
//    |   |   |   %8 = (%A)[zext.i32.i64(%N) * i1 + i2];
//    |   |   |   %mul11 = (%B)[sext.i32.i64(%K) * i2 + i3]  *  %8;
//    |   |   |   %add16 = (%C)[sext.i32.i64(%K) * i1 + i3]  +  %mul11;
//    |   |   |   (%C)[sext.i32.i64(%K) * i1 + i3] = %add16;
//    |   |   + END LOOP
//    |   + END LOOP
//    + END LOOP

bool HIRGenerateMKLCall::isMatmul(
    HLLoop *Loop, SmallVector<const RegDDRef *, 3> &MatrixRefs,
    SmallVector<const RegDDRef *, 3> &TripCountDDRefs, bool *IsZeroSet) const {
  LLVM_DEBUG(dbgs() << "\nIn matmul pattern matching:\n"; Loop->dump());

  bool IsValid = true;

  const HLLoop *InnermostLoop;

  // Check for second level loop
  HLLoop *SecondLoop = dyn_cast<HLLoop>(Loop->getFirstChild());
  if (!SecondLoop) {
    return false;
  }

  if (!(HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop))) {
    return false;
  }

  if (InnermostLoop != dyn_cast<HLLoop>(SecondLoop->getFirstChild())) {
    return false;
  }

  if (InnermostLoop->hasLiveOutTemps()) {
    return false;
  }

  const HLInst *FirstLoadInst =
      dyn_cast<HLInst>(InnermostLoop->getFirstChild());

  // First load:   %"cmatmul_$B3[][]_fetch.9" = (%"cmatmul_$B3")[i1][i2]
  if (!FirstLoadInst || !isa<LoadInst>(FirstLoadInst->getLLVMInstruction())) {
    LLVM_DEBUG(dbgs() << "Unwanted instructions inside innermost loop.\n");
    return false;
  }

  const RegDDRef *LvalTmp1 = FirstLoadInst->getLvalDDRef();
  const RegDDRef *LoadRef1 = FirstLoadInst->getRvalDDRef();

  // Match: %mul.4 = %"cmatmul_$B3[][]_fetch.9" * (%"cmatmul_$A3")[i2][i3]

  const HLInst *MultiplyInst = dyn_cast<HLInst>(FirstLoadInst->getNextNode());

  const RegDDRef *MulOperand1 = nullptr;
  const RegDDRef *MulOperand2 = nullptr;

  if (!matchMultiplication(MultiplyInst, &MulOperand1, &MulOperand2)) {
    return false;
  }

  // One of them has to be a temp
  if (MulOperand1->isMemRef() && MulOperand2->isMemRef()) {
    return false;
  }

  const RegDDRef *LoadRef2;

  if (MulOperand1->isMemRef())
    LoadRef2 = MulOperand1;
  else
    LoadRef2 = MulOperand2;

  // Match: Def and use of  %"cmatmul_$B3[][]_fetch.9"
  if (!matchTemp(MulOperand1, LvalTmp1) && !matchTemp(MulOperand2, LvalTmp1)) {
    LLVM_DEBUG(dbgs() << "Multiplication tmp does not match \n");
    return false;
  }

  // Match:  %add.1 = %mul.4  +  (%"cmatmul_$C3")[i1][i3]
  const HLInst *AddInst = dyn_cast<HLInst>(MultiplyInst->getNextNode());
  const RegDDRef *C3LoadRef;
  if (!matchAddition(AddInst, &C3LoadRef)) {
    return false;
  }

  // Match: (%"cmatmul_$C3")[i1][i3] = %add.1
  const HLInst *StoreInst = dyn_cast<HLInst>(AddInst->getNextNode());
  const RegDDRef *C3StoreRef = nullptr;
  if (!checkStoreInstruction(StoreInst, C3LoadRef, &C3StoreRef)) {
    return false;
  }

  if (StoreInst->getNextNode()) {
    return false;
  }

  // Check that all symbases are different
  if (LoadRef1->getSymbase() == C3StoreRef->getSymbase() ||
      LoadRef2->getSymbase() == C3StoreRef->getSymbase()) {
    return false;
  }

  // Number of dimensions check
  unsigned NumDims1 = LoadRef1->getNumDimensions();
  unsigned NumDims2 = LoadRef2->getNumDimensions();
  unsigned NumDims3 = C3StoreRef->getNumDimensions();

  if (!((NumDims1 == 2 && NumDims2 == 2 && NumDims3 == 2) ||
        (NumDims1 == 1 && NumDims2 == 1 && NumDims3 == 1) ||
        (NumDims1 == 2 && NumDims2 == 1 && NumDims3 == 2) ||
        (NumDims1 == 3 && NumDims2 == 3 && NumDims3 == 3) ||
        (NumDims1 == 1 && NumDims2 == 2 && NumDims3 == 1))) {
    LLVM_DEBUG(dbgs() << "Number of dimensions not fit for matmul.\n");
    return false;
  }

  // Check IV levels in 3 memrefs
  SmallVector<unsigned, 4> Levels1 =
      getIVLevels(LoadRef1, &IsValid, true); // 1 2 // 2 1 // 1 3
  if (!IsValid) {
    return false;
  }

  SmallVector<unsigned, 4> Levels2 =
      getIVLevels(LoadRef2, &IsValid, true); // 2 3 // 3 2 // 2 3
  if (!IsValid) {
    return false;
  }

  SmallVector<unsigned, 4> Levels3 =
      getIVLevels(C3StoreRef, &IsValid, true); // 1 3 // 3 1 // 1 2
  if (!IsValid) {
    return false;
  }

  if (!((Levels1[0] == Levels2[1] && Levels1[1] == Levels3[1] &&
         Levels2[0] == Levels3[0]) ||
        (Levels1[1] == Levels2[0] && Levels1[0] == Levels3[0] &&
         Levels2[1] == Levels3[1]))) {
    LLVM_DEBUG(dbgs() << "IV levels mismatch for matmul\n");
    return false;
  }

  SmallSet<unsigned, 4> NestingLevels;
  NestingLevels.insert(Loop->getNestingLevel());
  NestingLevels.insert(SecondLoop->getNestingLevel());
  NestingLevels.insert(InnermostLoop->getNestingLevel());

  if (!(NestingLevels.count(Levels1[0]) && NestingLevels.count(Levels1[1]) &&
        NestingLevels.count(Levels2[0]) && NestingLevels.count(Levels2[1]) &&
        NestingLevels.count(Levels3[0]) && NestingLevels.count(Levels3[1]))) {
    return false;
  }

  TripCountDDRefs = {Loop->getTripCountDDRef(), SecondLoop->getTripCountDDRef(),
                     InnermostLoop->getTripCountDDRef()};

  MatrixRefs.push_back(C3StoreRef);
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
