//===--- HIRArrayContractionUtils.cpp - ---------------------*- C++ -*---===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//
// This file implements HIR Array Contraction Utility class.
//===-----------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRArrayContractionUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/STLExtras.h"

#define OPT_SWITCH "hir-array-contraction-utils"
#define OPT_DESC "HIR Array Contraction Utils"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::arraycontractionutils;

static cl::opt<StringRef>
    ContractedArrayName(OPT_SWITCH "-contracted-array-name",
                        cl::init("ContractedArray"), cl::Hidden,
                        cl::desc(OPT_DESC " Default Contracted Array Name"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void printSmallSetInt(SmallSet<unsigned, 4> &IntSet, std::string Msg) {
  formatted_raw_ostream FOS(dbgs());
  if (Msg.size()) {
    FOS << Msg << ": " << IntSet.size() << "\t<";
  }

  for (unsigned V : IntSet) {
    FOS << V << ",";
  }
  FOS << ">\n";
}
#endif //! defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// -----------------------------------------------------------------------
/*  Array Contraction Util API's implementation                          */
// -----------------------------------------------------------------------

// Check: is the given value V within the range of [LB, UB]?
template <typename T> static bool isInRange(T V, T LB, T UB) {
  assert((LB <= UB) && "Expect LB <= UB\n");
  return (V >= LB) && (V <= UB);
}

static bool isPrimitiveType(Type *Ty) {
  return isa<IntegerType>(Ty) || Ty->isFloatingPointTy();
}

bool HIRArrayContractionUtil::checkSanity(RegDDRef *Ref,
                                          SmallSet<unsigned, 4> &PreservedDims,
                                          SmallSet<unsigned, 4> &ToContractDims,
                                          SmallVectorImpl<unsigned> &DimSizeVec,
                                          Type *&RootTy) {

  // Check the Ref:
  // -is a memref with at least 5 dimensions
  //- is located inside a loop
  auto CheckRef = [&](const RegDDRef *Ref) {
    if (!Ref || !Ref->isMemRef() || (Ref->getNumDimensions() < 5)) {
      LLVM_DEBUG(
          dbgs() << "Expect a valid MemRef with at least 5 dimensions\n");
      return false;
    }

    if (!Ref->getParentLoop()) {
      LLVM_DEBUG(dbgs() << "Expect the Ref be inside a loop\n";);
      return false;
    }

    return true;
  };

  // CheckRefType: Ref is an array ref, but not having an explicit ArrayType.
  //
  // E.g.
  // Ref: (%56)[i2][i3][i4][i5][i6]
  // Its BaseCE is %56, and its type is double* (instead of an ArrayType),
  // it has 5 dimensions and there is an CE on each dimension.
  // Collectively, it represents an ArrayType suitable for contraction.
  //
  // After analysis, its RootTy is double*, and DimSizeVec may also need to
  // reflect non-constant dimension sizes.
  //
  auto CheckRefType = [&](RegDDRef *Ref, unsigned NumDimsRemain,
                          SmallVectorImpl<unsigned> &DimSizeVec,
                          Type *&RootTy) {
    const CanonExpr *CE = Ref->getBaseCE();
    LLVM_DEBUG(dbgs() << "BaseCE: "; CE->dump(1); dbgs() << "\n";
               dbgs() << "\tSrcType: "; CE->getSrcType()->dump();
               dbgs() << "\tDestType: "; CE->getDestType()->dump();
               dbgs() << "NumDimsRemain: " << NumDimsRemain << "\n";);
    assert((RootTy == nullptr) && "Expect RootTy be nullptr");
    assert(DimSizeVec.empty() && "Expect TyVec be empty");

    // CE's SrcType:
    // expect either an IntegerType or a FloatingPointType.
    const PointerType *PtrTy = dyn_cast<PointerType>(CE->getSrcType());
    Type *ElemTy = PtrTy->getElementType();
    if (isa<IntegerType>(ElemTy) || ElemTy->isFloatingPointTy()) {
      RootTy = ElemTy;
    } else {
      assert(0 && "Unknown ElemTy type\n");
    }
    LLVM_DEBUG(dbgs() << "RootTy: "; RootTy->dump(); dbgs() << "\n";);

    // Figure out the array size of each dimensions in DimsRemain:
    assert(NumDimsRemain + 1 <= Ref->getNumDimensions());
    SmallVector<unsigned, 4> DimStride(NumDimsRemain + 2);
    int64_t StrideConst = 0;

    for (unsigned I = 1; I <= NumDimsRemain + 1; ++I) {
      CanonExpr *Stride = Ref->getDimensionStride(I);

      if (Stride->isIntConstant(&StrideConst)) {
        DimStride[I] = StrideConst;
      } else {
        assert(0 && "Expect the relevant stride be an integer constant");
      }

      LLVM_DEBUG({
        CanonExpr *LB = Ref->getDimensionLower(I);
        CanonExpr *Index = Ref->getDimensionIndex(I);
        dbgs() << "Dim: " << I << "\tLB: ";
        LB->dump();
        dbgs() << "\tStride: ";
        Stride->dump();
        dbgs() << "\tIndex: ";
        Index->dump();
        dbgs() << "\n";
      });
    }

    // See the  NumDimsRemain + 1 Constant strides:
    LLVM_DEBUG({
      for (auto V : enumerate(DimStride)) {
        auto index = V.index();
        auto value = V.value();
        dbgs() << "idx: " << index << ",\tvalue: " << value << "\n";
      }
    });

    // Compute DimSizeVec for qualified Dimensions:
    for (unsigned I = 1; I <= NumDimsRemain; ++I) {
      assert(DimStride[I]);
      const unsigned NumItems = DimStride[I + 1] / DimStride[I];
      LLVM_DEBUG(dbgs() << "DimStride[" << I << "]: " << DimStride[I] << ",\t"
                        << "DimStride[" << I + 1 << "]: " << DimStride[I + 1]
                        << ",\tNumItems: " << NumItems << "\n";);
      DimSizeVec.push_back(NumItems);
    }

    // See the Results:
    LLVM_DEBUG({
      dbgs() << "DimSizeVec: " << DimSizeVec.size() << "\nl";
      for (auto V : enumerate(DimSizeVec)) {
        auto index = V.index();
        auto value = V.value();
        dbgs() << "idx: " << index << ",\tvalue: " << value << "\n";
      }
      dbgs() << "RootTy: ";
      RootTy->dump();
      dbgs() << "\n";
    });

    return true;
  };

  // CheckCE: ArrayType, M dimensions, N items on each dimension
  // [Note]
  // It is possible that different different dimensions have different number of
  // elements.
  //
  // E.g.
  // A[M][N][O][P][Q], where M, N, O, P, Q are unsigned integers and they may
  // not always be the same.
  //
  // This function will check that the given CE's type is indeed an array type,
  // and its ultimate base type is a primitive type.
  //
  // E.g.
  // int A[100][100][100][10][10] is an ArrayType, its root type is int.
  //
  auto CheckBaseCE = [&](RegDDRef *Ref, unsigned NumDimsRemain,
                         SmallVectorImpl<unsigned> &DimSizeVec, Type *&RootTy) {
    const CanonExpr *CE = Ref->getBaseCE();
    LLVM_DEBUG(dbgs() << "BaseCE: "; CE->dump(1); dbgs() << "\n";
               dbgs() << "\tSrcType: "; CE->getSrcType()->dump();
               dbgs() << "\tDestType: "; CE->getDestType()->dump();
               dbgs() << "NumDimsRemain: " << NumDimsRemain << "\n";);
    assert((RootTy == nullptr) && "Expect RootTy be nullptr");
    assert(DimSizeVec.empty() && "Expect TyVec be empty");

    // Analyze SrcType:
    // expect an array-type on each dimension, all the way to RootTy.
    const PointerType *PtrTy = dyn_cast<PointerType>(CE->getSrcType());
    const ArrayType *ArryTy = dyn_cast<ArrayType>(PtrTy->getElementType());
    if (!ArryTy) {
      Type *EleTy = PtrTy->getElementType();
      if (isPrimitiveType(EleTy)) {
        return CheckRefType(Ref, NumDimsRemain, DimSizeVec, RootTy);
      }
      LLVM_DEBUG(dbgs() << "Unknown type\n";);
      return false;
    }
    LLVM_DEBUG(dbgs() << "ArrTy: "; ArryTy->dump(););

    unsigned Dim = Ref->getNumDimensions() - 1;
    while (true) {
      unsigned N = ArryTy->getNumElements();
      if (Dim <= NumDimsRemain) {
        DimSizeVec.push_back(N);
      }

      Type *Ty = ArryTy->getElementType();
      if (isa<ArrayType>(Ty)) {
        --Dim;
        ArryTy = dyn_cast<ArrayType>(Ty);
        LLVM_DEBUG(dbgs() << "ArrTy: "; ArryTy->dump(););
      } else if (isPrimitiveType(Ty)) {
        // reach a non-ArrayType RootTy: normal exit from loop
        LLVM_DEBUG(dbgs() << "RootTy: "; Ty->dump(););
        RootTy = Ty;
        break;
      } else {
        LLVM_DEBUG(dbgs() << "Unsupported Type: "; Ty->dump(););
        return false;
      }
    }

    return true;
  };

  // Check PreservedDims and ToContractDims:
  // - ToContractDims: non-empty, be adjacent, all within range
  // - PreservedDims: non-empty, complement from ToContractDims, within range
  // - Both: both sizes sum to Ref's #Dimensions
  auto CheckPreservedAndToContractDims = [&](const RegDDRef *Ref,
                                             SmallSet<unsigned, 4>
                                                 &PreservedDims,
                                             SmallSet<unsigned, 4>
                                                 &ToContractDims) {
    LLVM_DEBUG({
      printSmallSetInt(ToContractDims, "ToContractDims");
      printSmallSetInt(PreservedDims, "PreservedDims");
    });

    // Check ToContractDims: non empty, all within range
    if (ToContractDims.size() == 0) {
      LLVM_DEBUG(dbgs() << "Expect non-empty ToContractDims\n";);
      return false;
    }

    // Expect each value in ToContractDims:
    // - unsigned integer, within range of [1..#Dimensions]
    // - be adjacent (already sorted)
    const unsigned NumDims = Ref->getNumDimensions();
    for (auto Item : ToContractDims) {
      if (!isInRange<unsigned>(Item, 1, NumDims)) {
        LLVM_DEBUG(dbgs() << "Expect items in ToContractDims be within the "
                             "range of [1.."
                          << NumDims << "]\n";);
        return false;
      }
    }

    unsigned Item0 = *ToContractDims.begin();
    for (auto I = std::next(ToContractDims.begin(), 1),
              E = ToContractDims.end();
         I != E; ++I) {
      unsigned Item1 = *I;
      if (Item1 - Item0 != 1) {
        LLVM_DEBUG(
            dbgs() << "Expect continuous dimensions in ToContractDims\n";);
        return false;
      }
      Item0 = Item1;
    }

    // Expect each value in PreservedDims:
    // - unsigned integer(s), within range of [1..#Dimensions]
    for (auto Item : PreservedDims) {
      if (!isInRange<unsigned>(Item, 1, NumDims)) {
        LLVM_DEBUG(
            dbgs()
                << "Expect items in PreservedDims be within the range of [1.."
                << NumDims << "]\n";);
        return false;
      }
    }

    // Expect ToContractDims and PreservedDims complement each other and
    // produce a full set when combined.
    const unsigned ToContractSize = ToContractDims.size();
    std::vector<unsigned> Diff;
    // Check: A - B = A
    std::set_difference(ToContractDims.begin(), ToContractDims.end(),
                        PreservedDims.begin(), PreservedDims.end(),
                        std::inserter(Diff, Diff.begin()));
    if (Diff.size() != ToContractSize) {
      LLVM_DEBUG({
        dbgs() << " ------------------------------------------------ \n";
        printSmallSetInt(ToContractDims, "ToContractDims");
        printSmallSetInt(PreservedDims, "PreservedDims");
        dbgs() << "ToContractDims and PreservedDims don't complement\n";
        dbgs() << " ------------------------------------------------ \n";
      });
      return false;
    }

    Diff.clear();
    const unsigned PreservedSize = PreservedDims.size();
    // Check: B - A = B
    std::set_difference(PreservedDims.begin(), PreservedDims.end(),
                        ToContractDims.begin(), ToContractDims.end(),
                        std::inserter(Diff, Diff.begin()));
    if (Diff.size() != PreservedSize) {
      LLVM_DEBUG({
        dbgs() << " ------------------------------------------------ \n";
        printSmallSetInt(ToContractDims, "ToContractDims");
        printSmallSetInt(PreservedDims, "PreservedDims");
        dbgs() << "ToContractDims and PreservedDims don't complement\n";
        dbgs() << " ------------------------------------------------ \n";
      });
      return false;
    }

    // Check: A + B = full set
    if (ToContractDims.size() + PreservedDims.size() !=
        Ref->getNumDimensions()) {
      LLVM_DEBUG(dbgs() << "Array dimension mismatch: missing dimension(s)\n";);
      return false;
    }

    return true;
  };

  //*** Begin of function body ***

  // Check Ref: expect a MemRef good for contraction
  if (!CheckRef(Ref)) {
    LLVM_DEBUG(dbgs() << "Failure in CheckRef(.)\n";);
    return false;
  }

  // Check ToContractDims and PreserveDims: expect they are isolated and form a
  // complete set when combined
  if (!CheckPreservedAndToContractDims(Ref, PreservedDims, ToContractDims)) {
    LLVM_DEBUG(dbgs() << "Failure in CheckPreserveAndToContract(.)\n";);
    return false;
  }

  // Check Ref's BaseCE type: expect RootTy be an primitive (int or float) type
  if (!CheckBaseCE(Ref, PreservedDims.size(), DimSizeVec, RootTy)) {
    LLVM_DEBUG(dbgs() << "Failure in CheckBaseCE(.)\n";);
    return false;
  }
  assert(RootTy && DimSizeVec.size() &&
         "Something is wrong with RootTy and/or DimSizeVec");
  LLVM_DEBUG({
    dbgs() << "DimTyVec: <" << DimSizeVec.size() << "> - \n";
    unsigned I = 0;
    for (auto Size : DimSizeVec) {
      dbgs() << I++ << ":\t" << Size << "\n";
    }
    dbgs() << "RootTy: ";
    RootTy->dump();
    dbgs() << "\n";
  });

  return true;
}

bool HIRArrayContractionUtil::allocateStorage(
    RegDDRef *Ref, HLRegion &Reg, SmallVectorImpl<unsigned> &DimSizeVec,
    Type *RootTy, RegDDRef *&AfterContractRef, unsigned &AllocaBlobIndex) {

  // Check: support stack allocation only!
  if (!Ref->accessesAlloca()) {
    LLVM_DEBUG(dbgs() << "allocateStorage(.) - Only support Stack storage\n";);
    return false;
  }

  // Check: is storage already allocated?
  if (AfterContractRef) {
    AllocaBlobIndex = AfterContractRef->getBasePtrBlobIndex();
    LLVM_DEBUG(dbgs() << "allocateStorage(.) - storage already allocated\n";);
    return true;
  }

  // Allocate new storage and record it:
  auto &HNU = Ref->getParentLoop()->getHLNodeUtils();

  // Obtain the After-Contract Ref's ArrayType
  // [Note]
  // - ArrayType and DimSizes are provided in args: DimSizeVec and RootTy
  LLVM_DEBUG(dbgs() << "RootTy: "; RootTy->dump(););

  ArrayType *AfterContractTy = ArrayType::get(RootTy, DimSizeVec[0]);
  for (unsigned I = 1, E = DimSizeVec.size(); I < E; ++I) {
    AfterContractTy = ArrayType::get(AfterContractTy, DimSizeVec[I]);
  }
  LLVM_DEBUG(dbgs() << "AfterContractType: "; AfterContractTy->dump(););

  // Create an AllocaInst
  // [Note]
  // - need to create the alloca on region(function) level.
  // - no need to create a new symbase explicitly, the newly created alloca
  //   will come with a new symbase.
  // - size of alloca is implied by its type.
  AllocaBlobIndex =
      HNU.createAlloca(AfterContractTy, &Reg, ContractedArrayName);
  LLVM_DEBUG({
    dbgs() << "AllocaBlobIndex: ";
    Ref->getBlobUtils().getBlob(AllocaBlobIndex)->dump();
    dbgs() << "\n";
  });

  return true;
}

// Add a symbase to the livein set of each loop level where the given Ref
// resides.
void HIRArrayContractionUtil::addSBToLoopnestLiveIn(HLLoop *Lp,
                                                    const unsigned SB) {
  assert(Lp && "Expect a valid HLLoop*");
  while (Lp) {
    Lp->addLiveInTemp(SB);
    Lp = Lp->getParentLoop();
  }
}

// This function maps a PreservedDim to its after-contraction dimension index.
//
// E.g.
// [Before Contraction]
// (%D)[0][i5][i4][i3][i2][i1]
// dim: 6  5   4   3   2   1
//             ~~~~~~~~~             <- to-contract dimensions
//      ^^^^^              ^         <- to-preserve dimensions
//
// [After Contraction]
// (%DD)[0][i5][i1]
//       3   2   1
//
// Thus the mapping is:
//  1 -> 1
//  5 -> 2
//  6 -> 3
//
static unsigned MapDimension(unsigned DimBefore,
                             SmallSet<unsigned, 4> &ToContractDims) {
  const unsigned LB =
      *std::min_element(ToContractDims.begin(), ToContractDims.end());
  const unsigned UB =
      *std::max_element(ToContractDims.begin(), ToContractDims.end());

  if (DimBefore < LB) {
    return DimBefore;
  } else {
    return DimBefore - (UB - LB + 1);
  }
}

void HIRArrayContractionUtil::contract(RegDDRef *Ref,
                                       SmallSet<unsigned, 4> &PreservedDims,
                                       SmallSet<unsigned, 4> &ToContractDims,
                                       unsigned AllocaBlobIndex,
                                       RegDDRef *&AfterContractRef) {
  assert(AllocaBlobIndex != InvalidBlobIndex &&
         "Expect a valid AllocaBlobIndex");

  LLVM_DEBUG({
    dbgs() << "\nAllocaSB: "
           << Ref->getBlobUtils().getTempBlobSymbase(AllocaBlobIndex) << "\n";
  });

  // ** Create the after-contraction MemRef **
  // AfterRef begins from a clone of the original ref
  RegDDRef *AfterRef = Ref->clone();

  // Remove any to-contract dimension(s):
  // [Note]
  // - This only removes the to-contract dimensions. Will need to make the
  //   DDRef's internal structures consistent with the contracted ref.
  // - Need reverse order.
  //   (SmallSet doesn't support reverse iterator, so use SmallVector to
  //   stage.)
  //
  SmallVector<unsigned, 4> Vec(ToContractDims.begin(), ToContractDims.end());
  llvm::sort(Vec.begin(), Vec.end(), std::greater<unsigned>());
  for (auto I : Vec) {
    AfterRef->removeDimension(I);
  }
  LLVM_DEBUG(dbgs() << "After removeDimension(s): \n";
             dbgs() << "#Dims: " << AfterRef->getNumDimensions();
             dbgs() << "\nNon-Detail mode: "; AfterRef->dump(0);
             dbgs() << "\nDetail mode: "; AfterRef->dump(1); dbgs() << "\n";);

  // Adjust the AfterRef:
  // - Symbase, and each available Dim : (LB, Index, Stride)

  LLVM_DEBUG({
    for (unsigned PreservedDim : PreservedDims) {
      unsigned Dim = MapDimension(PreservedDim, ToContractDims);
      LLVM_DEBUG(dbgs() << "PreservedDim: " << PreservedDim << " -> " << Dim
                        << "\nDim: " << Dim << "\t";);

      // ** Examine what we have now on Dim: (LB, Index, Stride) **
      // LB:
      CanonExpr *LB = AfterRef->getDimensionLower(Dim);
      LLVM_DEBUG(dbgs() << "LB: "; LB->dump(); dbgs() << "\t";);

      // Stride:
      CanonExpr *Stride = AfterRef->getDimensionStride(Dim);
      LLVM_DEBUG(dbgs() << "Stride: "; Stride->dump(); dbgs() << "\t";);

      // Index:
      CanonExpr *Idx = AfterRef->getDimensionIndex(Dim);
      LLVM_DEBUG(dbgs() << "Index: "; Idx->dump(); dbgs() << "\t";);

      // DimSize:
      unsigned DimSize = AfterRef->getDimensionSize(Dim);
      LLVM_DEBUG(dbgs() << "DimSize: " << DimSize << "\n";);
    }
  });

  auto &HNU = Ref->getParentLoop()->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();
  auto &CEU = DDRU.getCanonExprUtils();

  // *** Rewrite the BaseCE ***
  CanonExpr *BaseCE = AfterRef->getBaseCE();
  LLVM_DEBUG(dbgs() << "BaseCE: "; BaseCE->dump();
             dbgs() << "\tDefAtLevel: " << BaseCE->getDefinedAtLevel()
                    << "\n";);

  // Create a new CanonExpr* that contains only the NewBlob
  // [Note]
  // - Alloca is defined at region-entry level, so its level is 0.
  // - Alignment is the same as the alignment for Ref.
  CanonExpr *NewBaseCE =
      CEU.createSelfBlobCanonExpr(AllocaBlobIndex, BaseCE->getDefinedAtLevel());
  LLVM_DEBUG(dbgs() << "NewBaseCE: "; NewBaseCE->dump(1); dbgs() << "\n";);

  // Set AfterRef's BaseCE to the NewBaseCE:
  // [Note]
  // - Since both setBaseCE() and setAlignment() will call createGepInfo(),
  //   for each dimension, set LB, Stride, and Index need to happen after it.
  AfterRef->setBaseCE(NewBaseCE);
  AfterRef->setAlignment(Ref->getAlignment());
  LLVM_DEBUG({
    dbgs() << "\nAfterRef (Before updateBlobDDRefs):\t";
    AfterRef->dump(1);
    dbgs() << "\n";
  });

  // Cann't call makConsistent() on AfterRef because the ref and the entire
  // loopnest is detacched.
  SmallVector<BlobDDRef *, 8> NewBlobs;
  AfterRef->updateBlobDDRefs(NewBlobs);

  LLVM_DEBUG({
    dbgs() << "\nAfterRef (After updateBlobDDRefs()):\t";
    AfterRef->dump(1);
    dbgs() << "\n";
  });

  // Copy symbase from existing contracted ref if available, else create new
  // symbase.
  unsigned NewSB =
      AfterContractRef ? AfterContractRef->getSymbase() : DDRU.getNewSymbase();
  AfterRef->setSymbase(NewSB);

  AfterContractRef = AfterRef;
}

bool HIRArrayContractionUtil::contractMemRef(
    RegDDRef *ToContractRef, SmallSet<unsigned, 4> &PreservedDims,
    SmallSet<unsigned, 4> &ToContractDims, HLRegion &Reg,
    RegDDRef *&AfterContractRef) {

  SmallVector<unsigned, 4> DimSizeVec;
  Type *RootTy = nullptr;

  if (!checkSanity(ToContractRef, PreservedDims, ToContractDims, DimSizeVec,
                   RootTy)) {
    LLVM_DEBUG(dbgs() << "Failure in checkSanity() for contractMemRef(.)\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << " ================================================= \n"
           << "Ref: ";
    ToContractRef->dump(1);
    dbgs() << "\n";
    dbgs() << "DimTyVec: <" << DimSizeVec.size() << "> - \n";
    unsigned I = 0;
    for (auto S : DimSizeVec) {
      dbgs() << I++ << ":\t" << S << "\n";
    }
    dbgs() << "RootTy: ";
    RootTy->dump();
    dbgs() << " ================================================= \n";
  });

  unsigned AllocaBlobIndex = InvalidBlobIndex;
  if (!allocateStorage(ToContractRef, Reg, DimSizeVec, RootTy, AfterContractRef,
                       AllocaBlobIndex)) {
    LLVM_DEBUG(dbgs() << "Failure in allocateStorage(.)\n");
    return false;
  }

  contract(ToContractRef, PreservedDims, ToContractDims, AllocaBlobIndex,
           AfterContractRef);

  // Add alloca symbase as loopnest Livein:
  unsigned AllocaSB =
      ToContractRef->getBlobUtils().getTempBlobSymbase(AllocaBlobIndex);
  addSBToLoopnestLiveIn(ToContractRef->getParentLoop(), AllocaSB);

  HIRTransformUtils::replaceOperand(ToContractRef, AfterContractRef);

  return true;
}
